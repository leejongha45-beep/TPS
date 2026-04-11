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

**핵심 이점:**
- 먼 군집은 정수 연산만 → CPU 부하 최소화
- Fold 시 생존자 수 보존 → 추상 전투 결과와 실제 전투 결과 일관성 유지
- NPC(아군)도 동일 패턴 → `TPSNPCPoolSubsystem`에서 풀 획득/반환

---

## 핵심 3 — 더블버퍼링 Lock-Free 패턴

모든 ECS 컴포넌트는 Current/Prev 쌍으로 운용:

```
① Read     — Prev → const 지역변수 캐싱
② 계산     — 지역변수만 사용
③ Write    — 결과 → Current에 쓰기
④ PushToPrev — Current → Prev 복사
```

**왜 필요한가:** ParallelFor에서 Entity A가 Current를 쓰는 동안 Entity B가 같은 Current를 읽으면 레이스 컨디션 발생. Prev(읽기 전용) / Current(쓰기 전용)를 분리하여 **lock 없이 수천 Entity 병렬 처리**.

lock 대신 더블버퍼링을 선택한 이유: 수천 Entity에 lock을 걸면 contention이 병렬화 이점을 상쇄함.

**병렬 실행 시스템 — AISystem (ParallelFor):**

```cpp
// 수천 Entity를 워커 스레드에서 병렬 처리 — Prev/Current 분리로 lock 불필요
ParallelFor(Count, [&](int32 Index)
{
    const entt::entity Entity = Entities[Index];

    // ① Read — Prev에서 지역변수로 캐싱 (이번 프레임 Write와 격리)
    const FVector Pos   = View.get<CTransformPrev>(Entity).Position;
    const float HP      = View.get<CHealthPrev>(Entity).Current;
    const float Speed   = View.get<CMovementPrev>(Entity).MaxSpeed;

    // ② 계산 — 지역변수만 사용
    if (HP <= 0.f) { NewState = Dying;  NewVelocity = FVector::ZeroVector; }
    else           { NewVelocity = Dir * Speed; NewState = Moving; }

    // ③ Write — Current에 쓰기
    View.get<CEnemyState>(Entity).State  = NewState;
    View.get<CMovement>(Entity).Velocity = NewVelocity;

    // ④ PushToPrev — 다음 Phase/프레임의 Read용
    View.get<CEnemyStatePrev>(Entity).State  = NewState;
    View.get<CMovementPrev>(Entity).Velocity = NewVelocity;
});
```

**직렬 실행 시스템 — DamageSystem (GameThread 단일):**

```cpp
// DamageQueue는 투사체 OnHit에서 임의 타이밍에 적재 → Phase 2에서 동기적 일괄 소비
// 단일 스레드: 큐 소비 + Entity 상태 변경이 순서 보장 필요
for (const FDamageEvent& Event : DamageQueue)
{
    // ① Read — InstanceToEntity 룩업으로 O(1) Entity 특정
    const entt::entity Entity = InstanceToEntityPerLOD[Event.LODLevel][Event.InstanceIndex];

    // ② Write — 체력 감산
    CHealth& Health = Registry.get<CHealth>(Entity);
    Health.Current = FMath::Max(Health.Current - Event.Damage, 0.f);

    // ③ PushToPrev — 다음 Phase(AISystem)에서 HP 0 감지 → Dying 전환
    Registry.get<CHealthPrev>(Entity).Current = Health.Current;
}
DamageQueue.Reset();  // 메모리 해제 없이 카운트만 초기화
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

### 투사체 Actor 풀링

```
[초기화] DataAsset에서 PoolSize/BatchSize 로드
 ├─ ① 초기 배치: SpawnBatch(500) — 즉시 사용 가능 보장
 └─ ② 지연 스폰: Timer(16ms 주기) × 10개씩 — 프레임 히치 방지

[런타임] Get/Return
 ├─ Pool.Get() → 비활성 투사체 Pop → 활성화 + 발사
 ├─ Pool.Return() → 비활성화 + Push
 └─ 풀 고갈 시: 긴급 SpawnActor (Warning 로그) — 게임 중단 없음
```

### ECS 메모리 풀링

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

