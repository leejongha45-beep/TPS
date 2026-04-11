# TPS — EnTT ECS 대량 렌더링 시스템

Unreal Engine 5.7 위에서 EnTT 기반 커스텀 ECS를 통합하여 **3,033체 동시 처리 49fps**를 달성한 TPS 프로젝트.
OOP(플레이어/무기) + DOD(대량 적 처리) 하이브리드 아키텍처.

---

## 벤치마크

`SCOPE_CYCLE_COUNTER`로 11개 Phase를 개별 측정, 병목을 특정하여 단계적으로 최적화:

| 단계 | 엔티티 | FPS | Visualization | 변경 내용 |
|------|--------|-----|---------------|-----------|
| 튜닝 전 | 467 | 24 | 6.93ms | (기준) |
| LOD 조정 | ~600 | 66 | 0.65ms | Near 300m → 100m |
| 틱 주기 조정 | 1,487 | 61 | 1.47ms | Mid 2→4, Far 4→12 프레임 |
| **최종** | **3,033** | **49** | **2.05ms** | Near 75m, Mid 200m |

> 6.5배 엔티티, 2배 프레임레이트. 최악 시나리오(전체 Mid LOD) 기준.

---

## 핵심 1 — 8-Phase 멀티스레드 ECS 파이프라인

```
[GameThread]                              [WorkerThread]
 │
 ├─ Phase 0:   PushToPrev_RenderProxy
 ├─ Phase 1:   PlayerPosition 캐싱
 ├─ Phase 1.1: NavMesh 경로 쿼리 [GT]
 ├─ Phase 1.5: LODSystem (ParallelFor)
 ├─ Phase 2:   DamageSystem
 ├─ Phase 3:   AISystem (ParallelFor)
 ├─ Phase 3.1: AttackSystem
 │
 ├─ Dispatch ─────────────────────────→  ├─ Phase 3.5: SeparationSystem
 │                                        ├─ Phase 4:   DeathSystem
 ├─ ── Barrier ──
 │
 ├─ Dispatch ─────────────────────────→  ├─ Phase 5: AnimationSystem
 │                                        ├─ Phase 6: MovementSystem
 ├─ ── Barrier ──
 │
 ├─ Phase 7:   LOD Transition [GT]
 ├─ Phase 7.5: VisualizationSystem [GT]
 ├─ Phase 8:   CleanupSystem [GT]
 └─ ++FrameCounter
```

**병렬화 전략:**
- UObject 접근(NavMesh 쿼리, ISM API)은 반드시 GameThread
- 순수 수학 연산(AI, Movement, Animation)은 Worker 스레드 ParallelFor
- Write 대상이 겹치지 않는 시스템은 TaskGraph로 동시 실행
  - `Separation(CTransform)` ∥ `Death(CEnemyState)` — Write 대상 분리
  - `Animation(CAnimation)` ∥ `Movement(CTransform)` — Write 대상 분리
- Phase 간 동기화는 TaskGraph Barrier로 보장

**Phase 순서 근거:**
- Damage → AI: 피격 즉시 같은 프레임에서 Dying 전환 (1프레임 지연 방지)
- AI → Attack → Separation: 이동 방향 결정 → 공격 판정 → 겹침 보정 순서
- LOD Transition → Visualization → Cleanup: ISM 인스턴스 이동 → 렌더링 갱신 → Dead 제거

**스케줄러 코드 (축약):**

```cpp
void FEnemyScheduler::Tick(float DeltaTime)
{
    // Phase 1.5 — LOD 거리 판정 + 틱 빈도 결정
    LODSystem::Tick(Registry, PlayerPosition, DeltaTime, FrameCounter);          // W: CLOD(.Level .bShouldTick .AccumDT)  R: CTransformPrev.Position

    // Phase 2 — 데미지 큐 일괄 소비
    FramePlayerKillCount = DamageSystem::Tick(Registry, DamageQueue,              // W: CHealth.Current -= Damage
                                              InstanceToEntityPerLOD, HitEffects); // R: InstanceToEntityPerLOD[LOD][Index]

    // Phase 3 — AI 상태 결정 + 이동 방향 (ParallelFor)
    AISystem::Tick(Registry, PlayerPosition, AttackRange,                         // W: CEnemyState, CMovement, CAIMode, CWaypoint
                   CachedWaypoints, WaypointAcceptRadius, NPCPositions);          // R: CHealthPrev, CTransformPrev, CLODPrev

    // Phase 3.1 — 공격 쿨다운 틱 + 데미지 집계
    AttackSystem::Tick(Registry, DeltaTime, pCharacterDamageable);                // W: CAttack, CEnemyState  R: CAttackPrev, CAnimationPrev

    // Phase 3.5+4 — TaskGraph 병렬 (Write 대상 분리)
    FGraphEventRef SepTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&Registry = Registry, &PlayerPosition]() {
            SeparationSystem::Tick(Registry, PlayerPosition);                     // W: CTransform.Position  R: CTransformPrev (SpatialGrid)
        }, TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask);

    FGraphEventRef DeathTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&Registry = Registry]() {
            DeathSystem::Tick(Registry);                                          // W: CEnemyState = Dead  R: CEnemyStatePrev, CAnimationPrev
        }, TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask);

    FTaskGraphInterface::Get().WaitUntilTasksComplete({SepTask, DeathTask});

    // Phase 5+6 — TaskGraph 병렬 (Write 대상 분리)
    FGraphEventRef AnimTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&Registry = Registry, DeltaTime]() {
            AnimationSystem::Tick(Registry, DeltaTime);                           // W: CAnimation(.AnimIndex .AnimTime)  R: CEnemyStatePrev, CLODPrev
        }, TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask);

    FGraphEventRef MoveTask = FFunctionGraphTask::CreateAndDispatchWhenReady(
        [&Registry = Registry, DeltaTime]() {
            MovementSystem::Tick(Registry, DeltaTime);                            // W: CTransform.Position  R: CMovementPrev, CNavTargetPrev
        }, TStatId{}, nullptr, ENamedThreads::AnyHiPriThreadHiPriTask);

    FTaskGraphInterface::Get().WaitUntilTasksComplete({AnimTask, MoveTask});

    // Phase 7~8 — GameThread 직렬
    LODSystem::TransitionInstances(Registry, HISMRefs, InstanceToEntityPerLOD);   // W: CRenderProxy (swap-back 보정)
    VisualizationSystem::Tick(Registry, ...);                                     // R: CTransformPrev, CAnimationPrev → ISM 갱신 (변경 감지)
    CleanupSystem::Tick(Registry, ...);                                           // 내림차순 삭제 → swap-back → destroy
}
```

---

## 핵심 2 — Swarm Fold/Unfold 시스템

플레이어 거리에 따라 **추상 군집(Folded) ↔ 개별 엔티티(Unfolded)** 를 동적 전환하는 LOD 최적화.
먼 군집은 정수 연산으로 추상 전투, 가까워지면 개별 ECS 엔티티/NPC로 펼쳐서 전투.

```
[Folded — 추상 군집]                              [Unfolded — 개별 엔티티]

FSwarm { TroopCount, Position, HP }               ECS Entity × N (CSwarmID로 소속 추적)
 │  정수 연산으로 추상 전투                          │  개별 AI/Movement/Animation 처리
 │  웨이포인트 따라 이동                             │  ISM 렌더링 + 풀 파이프라인
 │                                                 │
 └──── 플레이어 접근 (< 150m) ──── Unfold ────→    │
       플레이어 이탈 (> 200m) ──── Fold ──────←    │
```

**히스테리시스:** Unfold 150m / Fold 200m 분리 → 경계에서 반복 전환 방지

```cpp
void UTPSSwarmSubsystem::CheckPlayerProximity()
{
    for (FSwarm& Swarm : Swarms)
    {
        const float DistSq = FVector::DistSquared(Swarm.Position, PlayerPos);

        if (!Swarm.bUnfolded && DistSq < UnfoldRadiusSq)        // 150m 이내 → 펼침
        {
            UnfoldSwarm(Swarm);                                  // ECS Entity 스폰 or NPC 풀에서 획득
        }
        else if (Swarm.bUnfolded && DistSq > FoldRadiusSq)       // 200m 이탈 → 접음
        {
            FoldSwarm(Swarm);                                    // Entity 파괴 or NPC 풀 반환, TroopCount 보존
        }
    }
}

void UTPSSwarmSubsystem::UnfoldSwarm(FSwarm& Swarm)
{
    for (int32 i = 0; i < Swarm.TroopCount; ++i)
    {
        FEnemySpawnParams Params;
        Params.Position = Swarm.Position + FVector(FMath::RandRange(-500.f, 500.f), ...);
        Params.SwarmID  = Swarm.ID;                              // CSwarmID로 소속 추적 → Fold 시 회수
        EnemyManager->QueueSpawn(Params);
    }
    Swarm.bUnfolded = true;
}

void UTPSSwarmSubsystem::FoldSwarm(FSwarm& Swarm)
{
    // CSwarmID == Swarm.ID인 생존 Entity 수 집계 → TroopCount 갱신 (전사자 반영)
    Swarm.TroopCount = CountLivingEntities(Swarm.ID);
    MarkEntitiesDead(Swarm.ID);                                  // ECS Entity → Dead 상태 → CleanupSystem에서 파괴
    Swarm.bUnfolded = false;
}
```

**미니맵 군집 추적 (MinimapViewModel):**

Folded 군집은 개별 엔티티가 없어서 월드에 시각적으로 보이지 않음.
미니맵에서 추상 군집의 위치와 규모를 실시간 추적하여 플레이어가 전장 상황을 파악할 수 있게 함.
Unfold되면 개별 엔티티가 월드에 존재하므로 마커를 자동으로 숨김.

```cpp
// Folded 군집만 미니맵에 마커 표시 — Unfolded되면 개별 엔티티가 보이므로 마커 숨김
for (int32 i = 0; i < Swarms.Num(); ++i)
{
    const FSwarm& Swarm = Swarms[i];
    FMinimapMarkerData& Marker = SwarmMarkers[i];

    if (Swarm.TroopCount <= 0 || Swarm.bUnfolded)               // 전멸 or 펼쳐진 상태 → 숨김
    {
        Marker.bVisible = false;
        continue;
    }

    Marker.Position = WorldToNormalized(Swarm.Position);         // 월드 → 정규화 좌표 변환
    Marker.Color = (Swarm.Team == ESwarmTeam::Enemy)             // 적: 빨강, 아군: 파랑
        ? FLinearColor(1.f, 0.3f, 0.3f, 0.9f)
        : FLinearColor(0.3f, 0.5f, 1.f, 0.9f);
    Marker.Size = FMath::Clamp(6.f + Swarm.TroopCount * 0.03f, 6.f, 20.f);  // 병력 수 비례 크기
}
```

**핵심 이점:**
- 먼 군집은 정수 연산만 → CPU 부하 최소화
- Fold 시 생존자 수 보존 → 추상 전투 결과와 실제 전투 결과 일관성 유지
- Folded 군집도 미니맵에서 위치/규모 확인 가능 → Unfold 시 마커 자동 숨김
- NPC(아군)도 동일 패턴 → `TPSNPCPoolSubsystem`에서 풀 획득/반환

---

## 핵심 3 — 더블버퍼링 Lock-Free 패턴

ParallelFor에서 수천 Entity가 동시에 같은 컴포넌트를 읽고 쓰면 레이스 컨디션 발생.
읽기(Prev)와 쓰기(Current)를 구조적으로 분리하여 **lock 없이 병렬 처리를 보장**.

```
❌ 단일 버퍼 — 레이스 컨디션:
  Thread A: Entity[0].Position = 새값        ← 쓰기 중
  Thread B: Entity[0].Position 읽기          ← half-written 값 읽음 → 정의되지 않은 동작

✅ 더블버퍼링 — Lock-Free:
  Thread A: Entity[0].Position(Current) = 새값     ← Current에 쓰기
  Thread B: Entity[0].PositionPrev 읽기             ← Prev는 이전 프레임 확정값 → 안전
```

lock 대신 더블버퍼링을 선택한 이유: 수천 Entity에 lock을 걸면 contention이 병렬화 이점을 상쇄함.

모든 시스템이 동일한 R→W→P 패턴을 따름:

```
① Read       — Prev → const 지역변수 캐싱
② 계산       — 지역변수만 사용 (View.get 접근 금지)
③ Write      — 결과 → Current에 쓰기
④ PushToPrev — Current → Prev 복사 (다음 Phase/프레임의 Read용)
```

**병렬 실행 — Animation ∥ Movement (Write 대상 상호 배타):**

```cpp
// ── Phase 5+6: 두 시스템이 TaskGraph로 동시 실행 ──
// Animation → CAnimation에만 쓰기  |  Movement → CTransform에만 쓰기
// Write 대상이 겹치지 않으므로 lock 없이 병렬 안전

// [Thread A] AnimationSystem              [Thread B] MovementSystem
// W: CAnimation                           W: CTransform
// R: CEnemyStatePrev, CLODPrev            R: CMovementPrev, CNavTargetPrev

ParallelFor(Count, [&](int32 Index)        ParallelFor(Count, [&](int32 Index)
{                                          {
    // ① Read                                  // ① Read
    State = CEnemyStatePrev.State;              Vel = CMovementPrev.Velocity;
    AccumDT = CLODPrev.AccumDT;                 GroundZ = CNavTargetPrev.GroundZ;

    // ③ Write                                  // ③ Write
    CAnimation.AnimIndex = NewIdx;              CTransform.Position += Vel * DT;
    CAnimation.AnimTime  = NewTime;             CTransform.Rotation = NewRot;

    // ④ PushToPrev                             // ④ PushToPrev
    CAnimationPrev = CAnimation;                CTransformPrev = CTransform;
});                                        });
```

| 컴포넌트 | 역할 |
|----------|------|
| CTransform / Prev | 위치/회전 |
| CMovement / Prev | 속도/최대속도 |
| CEnemyState / Prev | 상태 (Idle, Moving, Dying...) |
| CHealth / Prev | 체력 |
| CLOD / Prev | LOD 레벨/틱 주기/AccumulatedDeltaTime |
| CAnimation / Prev | VAT 애니메이션 |
| CRenderProxy / Prev | ISM 인스턴스 인덱스 |
| CAIMode / Prev | Rush/Chase 모드 |
| CNavTarget / Prev | NavMesh 웨이포인트 |

---

## 핵심 4 — 오브젝트 풀링 (런타임 힙 할당 제로)

런타임에 `SpawnActor`/`new`를 호출하면 힙 할당 + GC 부하로 프레임 히치 발생.
초기화 시점에 모든 오브젝트를 미리 생성하고, 런타임에는 Get/Return으로 재활용.

**투사체 Actor 풀링:**

```cpp
// ── 초기화: 2단계 스폰 (프레임 히치 방지) ──

void UTPSProjectilePoolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    SpawnProjectileBatch(ConfigAsset->InitialSpawnCount);        // ① 초기 배치 500개 즉시 스폰

    if (TotalSpawnedCount < PoolSize)                            // ② 나머지 → 16ms 주기로 10개씩 분산
    {
        InWorld.GetTimerManager().SetTimer(
            DeferredSpawnTimerHandle, this,
            &UTPSProjectilePoolSubsystem::DeferredSpawn, 0.016f, true);
    }
}

// ── 런타임: Get/Return ──

ATPSProjectileBase* UTPSProjectilePoolSubsystem::GetProjectile()
{
    if (Pool.Num() > 0) { return Pool.Pop(); }                   // 풀에서 Pop → 활성화

    // 풀 고갈 시 긴급 스폰 — 게임 중단 없이 Warning 로그
    return GetWorld()->SpawnActor<ATPSProjectileBase>(LoadedProjectileClass, ...);
}

void UTPSProjectilePoolSubsystem::ReturnProjectile(ATPSProjectileBase* InProjectile)
{
    Pool.Push(InProjectile);                                     // 비활성화 후 풀에 Push → 재활용
}
```

**ECS 메모리 풀링:**

| 기법 | 적용 위치 | 효과 |
|------|-----------|------|
| `TInlineAllocator<3000>` | 시스템 Entity 수집 배열 5개 | 3000개까지 스택, 힙 할당 제로 |
| `TArray::Reset()` | SpawnQueue, DamageQueue | 메모리 해제 없이 카운트만 초기화 |
| 스택 배열 `FDeadEntry[3000]` | CleanupSystem Dead 수집 | 힙 할당 완전 회피 |
| `FrameOffset` 분산 | LOD 틱 스케줄링 | 같은 LOD Entity를 프레임별 균등 분배 |

**공통 철학**: 런타임 핫패스에서 `new`/`SpawnActor`를 호출하지 않는다.

---

## 기타 시스템

### ISM 대량 렌더링 + LOD
- LOD별 ISM 3개 (Near ~75m 매프레임, Mid ~200m 4프레임, Far 200m+ 12프레임)
- 히스테리시스로 LOD 경계 진동 방지 (올라갈 때/내려올 때 임계값 분리)
- LOD Transition 시 swap-back 패턴으로 InstanceToEntity 역방향 테이블 보정
- VisualizationSystem: CVisCache 변경 감지 → 변경된 인스턴스만 ISM 갱신

### 데미지 파이프라인
- 투사체 OnHit → DamageQueue 적재 → Phase 2에서 일괄 소비
- ISM per-instance 콜리전 → InstanceIndex → InstanceToEntity[LOD][Index]로 O(1) Entity 특정
- 히트 이펙트: DamageSystem이 FHitEffectRequest 수집 → 게임 스레드에서 Niagara 스폰

### 분리 시스템 (SpatialGrid)
- 공간 해시 그리드 (CellSize = SeparationRadius) → 9셀 탐색으로 O(N²) → O(N)
- 하드 푸시 (위치 직접 보정) + Push.Z = 0 (공중 띄움 방지)
- 인덱스 해시 결정론적 방향 — FMath::FRandRange thread-safety 이슈 회피

### OOP 설계 (플레이어/무기)
- Controller → TScriptInterface<인터페이스> → Character 계층 분리
- 6개 컴포넌트 상호 참조 제로, 델리게이트로만 통신
- 상속 계층: CharacterBase → SoldierBase → Player (LSP 준수)

### MVVM UI
- ViewModel이 Model과 View 사이 데이터 변환 담당
- AmmoViewModel (탄약 표시), CrosshairViewModel (조준선 스프레드)

### Lyra AnimInstance
- GameThread: UObject 접근 → 지역변수 캐싱
- WorkerThread: 캐싱된 값으로 순수 수학 연산만 수행
- Animation Layer Interface (ALI): 무기별 애니메이션 교체

---

## 트러블슈팅

### ISM 인스턴스 렌더링 불가
- **증상**: `AddInstance` 성공, `InstanceCount` 증가, 화면에 미표시
- **원인**: `WorldSubsystem::Initialize()`에서 ISM 생성 — 렌더 씬 미준비
- **해결**: `OnWorldBeginPlay()`로 이동 (Mass Entity 패턴)

### EnTT 레지스트리 Race Condition
- **증상**: 빈 레지스트리에서 두 Worker가 동시에 `view()` 호출 → 풀 동시 생성 크래시
- **해결**: `bHasEntities` 외부 플래그로 빈 레지스트리 접근 차단

### LOD Transition InstanceIndex 크래시
- **증상**: `InstanceIndex < GetMaxInstanceIndex` assertion 실패
- **원인**: R→W→P 리팩토링 후 `CRenderProxyPrev`(stale)에서 인덱스를 읽음. swap-back이 Current를 갱신하므로 Prev는 stale
- **해결**: `CRenderProxy`(Current)에서 InstanceIndex 읽기로 변경

### 분리 시스템 적 겹침
- **증상**: 적들이 한 점에 수렴하여 겹침
- **원인**: `FMath::FRandRange`가 ParallelFor에서 thread-safe하지 않아 동일 방향으로 밀어냄 + 소프트 보정이 구조적으로 불충분
- **해결**: 인덱스 해시 결정론적 방향 + 하드 푸시(위치 직접 보정) + Push.Z=0

---

## 기술 스택

| 분류 | 상세 |
|------|------|
| 엔진 | Unreal Engine 5.7 |
| ECS | EnTT (헤더 온리 라이브러리, sparse set 기반) |
| 렌더링 | ISM × 3 LOD + CustomDataFloat (VAT) |
| 길찾기 | NavMesh 웨이포인트 체인 (Rush) + NavMesh 개별 경로 (Chase) |
| OOP | SOLID, 인터페이스 10개, 컴포넌트 6개, 델리게이트 통신 |
| 풀링 | Actor 풀링 (투사체), TInlineAllocator (ECS), 프레임 분산 스폰 |
| 프로파일링 | SCOPE_CYCLE_COUNTER 11 Phase, Stat ECSPhase 커스텀 그룹 |

