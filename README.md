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

## 기술 스택

| 분류 | 상세 |
|------|------|
| **엔진** | Unreal Engine 5.7 |
| **ECS** | EnTT (외부 라이브러리, 엔진 비종속) |
| **렌더링** | InstancedStaticMeshComponent (ISM) × 3 LOD |
| **길찾기** | FlowField BFS 128×128 (~242KB L2 캐시) |
| **분리** | SpatialGrid + 하드 푸시, 인덱스 해시 결정론적 방향 |
| **OOP** | SOLID 준수, 컴포넌트 6개 상호 참조 제로, MVVM UI 분리 |
| **패턴** | Observer (델리게이트), Strategy (인터페이스), Object Pooling, Spatial Partitioning |
| **풀링** | 투사체 Actor 풀링, ECS TInlineAllocator 메모리 풀링, 프레임 분산 스폰 |
| **애니메이션** | Lyra AnimInstance (멀티스레드 캐싱), ALI (Animation Layer Interface) |
| **프로파일링** | SCOPE_CYCLE_COUNTER 11 Phase, Stat ECSPhase 커스텀 그룹 |

---

## 아키텍처 — 8-Phase 멀티스레드 파이프라인

```
[GameThread]                              [WorkerThread]
 │                                         │
 ├─ Phase 0:   PushToPrev_RenderProxy      │
 ├─ Phase 1:   PlayerPosition 캐싱          │
 ├─ Phase 1.1: UpdateChaseTargets [GT]     │
 ├─ Phase 1.5: LODSystem (ParallelFor)     │
 ├─ Phase 2:   DamageSystem               │
 ├─ Phase 3:   AISystem (ParallelFor)     │
 ├─ Phase 3.1: AttackSystem               │
 │                                         │
 ├─ Dispatch ──────────────────────────→  ├─ Phase 3.5: SeparationSystem
 │                                         ├─ Phase 4:   DeathSystem
 ├─ ── Barrier ──                          │
 │                                         │
 ├─ Dispatch ──────────────────────────→  ├─ Phase 5: AnimationSystem
 │                                         ├─ Phase 6: MovementSystem
 ├─ ── Barrier ──                          │
 │                                         │
 ├─ Phase 7:   LOD Transition [GT]        │
 ├─ Phase 7.5: VisualizationSystem [GT]   │
 ├─ Phase 8:   CleanupSystem [GT]         │
 └─ ++FrameCounter                         │
```

**설계 원칙**:
- UObject 접근(라인 트레이스, NavMesh 쿼리, ISM API)은 반드시 GameThread
- 순수 수학 연산(AI, Movement, Animation)은 Worker 스레드 ParallelFor
- Phase 간 동기화는 TaskGraph Barrier로 보장

---

## 더블버퍼링 R→W→P 패턴

모든 ECS 시스템이 동일한 데이터 접근 패턴을 따릅니다:

```
① Read     — Prev 컴포넌트 → const 지역변수 캐싱 (상단에서 전부)
② 계산     — 지역변수만 사용 (View.get 접근 금지)
③ Write    — 계산 결과 → Current 컴포넌트에 쓰기
④ PushToPrev — Current → Prev 복사 (다음 Phase/프레임용)
```

**더블버퍼링 이유**: ParallelFor에서 엔티티 A가 쓰는 동안 엔티티 B가 같은 컴포넌트를 읽으면 데이터 레이스 발생.
Current(쓰기용) / Prev(읽기용)을 분리하여 **락 없이 병렬 처리를 보장**.

**컴포넌트 구조** (18개 = 9 Current + 9 Prev):

| 컴포넌트 | 역할 |
|----------|------|
| CTransform / CTransformPrev | 위치/회전 |
| CMovement / CMovementPrev | 속도/최대속도 |
| CEnemyState / CEnemyStatePrev | 상태 (Idle, Moving, Dying...) |
| CHealth / CHealthPrev | 체력 |
| CLOD / CLODPrev | LOD 레벨/틱 주기 |
| CAnimation / CAnimationPrev | VAT 애니메이션 |
| CRenderProxy / CRenderProxyPrev | ISM 인스턴스 인덱스 |
| CAIMode / CAIModePrev | Rush/Chase 모드 |
| CNavTarget / CNavTargetPrev | NavMesh 웨이포인트 |

---

## ISM 대량 렌더링 + LOD

```
스폰 요청 (WaveSubsystem)
 │
 ▼
FlushSpawnQueue [GT]
 ├─ SpawnSystem::Spawn() → entt::registry에 엔티티 + 컴포넌트 생성
 ├─ ISM[LOD0].AddInstance(Position) → InstanceIndex 획득
 ├─ CRenderProxy.InstanceIndex = InstanceIndex
 └─ InstanceToEntityPerLOD[LOD][Index] = Entity  (역방향 룩업 등록)
       │
       ▼
매 프레임 ECS Phase 순회
 ├─ LODSystem (Phase 1.5)
 │   └─ 거리 계산 → LOD Level 결정 → bShouldTick 결정
 │
 ├─ AI/Movement/Animation (Phase 3~6)
 │   └─ CTransform.Position 갱신, CAnimation.AnimTime 갱신
 │
 ├─ LOD Transition (Phase 7) [GT]
 │   └─ LOD 변경된 엔티티: OldISM.RemoveInstance → NewISM.AddInstance
 │       (swap-back 패턴으로 인덱스 보정)
 │
 └─ VisualizationSystem (Phase 7.5) [GT]
     ├─ CVisCachePrev와 비교 → 변경된 인스턴스만 필터
     ├─ UpdateInstanceTransform(Index, Transform, false, false)
     ├─ SetCustomDataValue(Index, AnimIndex/AnimTime)
     └─ MarkRenderStateDirty() — 프레임당 1회
```

### LOD 틱 빈도 제어

| LOD | 거리 | 틱 주기 | 용도 |
|-----|------|---------|------|
| Near | ~75m | 매 프레임 | 전투 가시 범위 |
| Mid | ~200m | 4프레임 | 중거리 |
| Far | 200m+ | 12프레임 | 원거리 (~0.2초 @60fps) |

### VisualizationSystem 최적화

- **변경 감지**: CVisCachePrev와 비교하여 변경된 인스턴스만 `UpdateInstanceTransform` 호출
- **`MarkRenderStateDirty`** 프레임당 1회만 호출
- `bShouldTick=false` 엔티티 스킵

---

## FlowField BFS 길찾기

```
ATPSAllyBase::BeginPlay()
 └─ TPSTargetSubsystem::SetAllyBaseLocation(기지위치)
       │
       ▼
EnemyManagerSubsystem (OnWorldBeginPlay 또는 FlushSpawnQueue Lazy)
 └─ TPSTargetSubsystem::GetAllyBaseLocation() → BaseLocation 취득
       │
       ▼
FEnemyScheduler::BuildFlowField(World, BaseLocation) [1회]
 ├─ [GT] Pass 1: LineTrace × 16,384 → Heights[] + Blocked[]
 ├─ [Worker] Pass 2: 8방향 경사각 → CliffEdges[] (순수 수학)
 ├─ [Worker] Pass 3: BFS(GoalIndex) → CostField[] → Directions[]
 └─ bReady = true
       │
       ▼
런타임 AISystem (매 프레임, ParallelFor)
 ├─ Rush 모드: FlowField.LookupDirection(X, Y) → 이동 방향
 ├─ 그리드 밖: (BaseLocation - Position).Normalize() → 직선 폴백
 └─ Chase 모드: CNavTargetPrev.NextWaypoint → 플레이어 추적
```

**128×128 그리드 (16,384셀)**, SoA 레이아웃으로 총 ~242KB — L2 캐시 내 전부 적재.

### AI 2단계 전환

| 모드 | 조건 | 경로 탐색 | 비용 |
|------|------|-----------|------|
| **Rush** (기본) | 스폰 시 | FlowField 방향 배열 룩업 | O(1) — 런타임 BFS 0 |
| **Chase** (피격/탐지) | HP 감소 or 거리 내 | NavMesh 개별 경로 쿼리 | 소수 엔티티만 |

### Lazy 초기화

WorldSubsystem과 Actor BeginPlay 순서가 보장되지 않으므로,
FlushSpawnQueue에서 TPSTargetSubsystem을 폴링하여 기지 위치 등록 시 자동 FlowField 빌드.

---

## 분리 시스템 — SpatialGrid + 하드 푸시

```
SeparationSystem::Tick (Phase 3.5, Worker)
 │
 ├─ Step A: FSpatialGrid::Build [단일 스레드]
 │   ├─ CTransformPrev.Position 읽기 (Dying/Dead 제외)
 │   ├─ 플레이어 기준 CullingRadius 밖 컬링
 │   └─ 해시 그리드 셀 등록: CellKey → [Index, ...]
 │
 └─ Step B: Hard Push [ParallelFor]
     ├─ 9셀(3×3) 탐색 → 이웃 검출
     ├─ Penetration = SeparationRadius - Distance
     ├─ Push += Direction × (Penetration × 0.5f)
     ├─ Push.Z = 0 (XY 평면 고정)
     ├─ Push = Clamp(MaxSeparationForce)
     │
     ├─ CTransform.Position += Push      ← Write
     └─ CTransformPrev.Position = 결과   ← PushToPrev
```

- **해시 기반 공간 분할** (CellSize = SeparationRadius)
- **위치 직접 보정** (Penetration × 0.5f — 양쪽 50%씩 밀어냄)
- **Z축 고정** (Push.Z = 0) — 분리력이 엔티티를 공중으로 띄우는 현상 방지
- **인덱스 해시 결정론적 방향** — `FMath::FRandRange` thread-safety 이슈 해결

---

## 데미지 파이프라인 — 비동기 큐 패턴

### 왜 비동기 큐인가?

액터는 개별 Tick이 있어서 `OnHit`에서 즉시 데미지 처리가 가능하지만,
ECS 엔티티는 개별 Tick이 없고 Scheduler가 Phase 순서대로 일괄 처리합니다.
투사체의 `OnHit`은 임의 타이밍에 발생하지만, ECS는 고정 Phase 순서를 지켜야 하므로
**큐에 모아서 Phase 2에서 동기적으로 일괄 소비**하는 비동기 큐 패턴을 적용했습니다.

```
플레이어 사격
 │
 ▼
TPSFireComponent::FireOnce() [GT]
 ├─ ProjectilePool.Get() → 투사체 활성화 (오브젝트 풀링)
 └─ Projectile 발사
       │
       ▼
TPSProjectileBase::OnHit() [GT]
 ├─ Cast<UInstancedStaticMeshComponent>(OtherComp) → ISM 히트 감지
 ├─ Hit.Item → InstanceIndex (ISM per-instance 콜리전)
 ├─ FindLODIndexByHISM(OtherComp) → LODLevel 역조회
 └─ EnemyManagerSubsystem::ApplyDamage(InstanceIndex, LODLevel, Damage)
       │
       ▼
FEnemyScheduler::DamageQueue.Add({InstanceIndex, LODLevel, Damage}) [GT]
       │
       ▼
DamageSystem::Tick (Phase 2) [GT]
 ├─ DamageQueue 순회
 ├─ InstanceToEntityPerLOD[LOD][Index] → Entity 역조회
 ├─ CHealth.Current -= Damage          ← Write
 └─ CHealthPrev.Current = 결과         ← PushToPrev
       │
       ▼
AISystem (Phase 3) → CHealthPrev.Current <= 0 → EEnemyState::Dying
       │
       ▼
DeathSystem (Phase 4) → CleanupSystem (Phase 8)
 └─ ISM.RemoveInstance + Registry.destroy(Entity)
```

---

## OOP 설계 — SOLID 원칙 기반 디커플링

ECS와 별개로 플레이어/무기 시스템은 SOLID 원칙을 철저히 준수하여 설계.
핵심은 **델리게이트 + 인터페이스로 모든 의존성을 끊어 재활용 가능성을 극대화**한 것입니다.

### 의존성 흐름도 — 순환 참조 제로

```
TPSPlayerController (Enhanced Input)
 │
 ├─ TScriptInterface<IMoveable>     ──→ TPSPlayer
 ├─ TScriptInterface<ISprintable>   ──→ TPSPlayer
 ├─ TScriptInterface<IAimable>      ──→ TPSSoldierBase
 ├─ TScriptInterface<IJumpable>     ──→ TPSPlayer
 ├─ TScriptInterface<IEquippable>   ──→ TPSSoldierBase
 ├─ TScriptInterface<IFireable>     ──→ TPSSoldierBase
 └─ TScriptInterface<IInteractable> ──→ TPSPlayer
       │
       │  Controller는 구체 Character 클래스를 모름
       │  OnPossess()에서 1회 캐스팅 후 인터페이스만 사용
       ▼
TPSSoldierBase / TPSPlayer (Character)
 │
 │  컴포넌트를 소유하지만, 컴포넌트끼리는 서로 모름
 │  Character가 중앙에서 델리게이트 배선
 │
 ├─ TPSFireComponent ──── OnFireOnceDelegate ──────────→ AnimInstance
 │                   ──── OnFireStateChangedDelegate ──→ AnimInstance, CrosshairViewModel
 │
 ├─ TPSEquipComponent ─── OnEquipMontagePlayDelegate ──→ AnimInstance
 │                    ─── OnEquipStateChangedDelegate ─→ AnimInstance, Character
 │
 ├─ TPSCameraControlComponent (독립, 델리게이트 불필요)
 └─ TPSPlayerInteractionComponent (독립)
       │
       │  AnimInstance는 델리게이트 구독만 — 역방향 호출 없음
       ▼
TPSPlayerCoreAnimInstance
 ├─ WeakPtr<TPSEquipComponent>  ← 구독만, 호출 안 함
 └─ WeakPtr<TPSFireComponent>   ← 구독만, 호출 안 함
```

**검증 결과**: FireComponent ↔ EquipComponent 상호 참조 **제로**. include 체인 순환 **제로**.

### 6개 컴포넌트 — 단일 책임, 상호 참조 제로

| 컴포넌트 | 책임 | 다른 컴포넌트 직접 참조 |
|----------|------|------------------------|
| TPSFireComponent | 사격 + 탄약 관리 | 없음 |
| TPSEquipComponent | 장착/해제 몽타주 | 없음 |
| TPSCameraControlComponent | 카메라 회전 + 줌 | 없음 |
| TPSPlayerInteractionComponent | 상호작용 감지 | 없음 |
| TPSPlayerStateComponent | 비트플래그 상태 관리 | 없음 |
| TPSPlayerStatusComponent | HP/속도 스탯 데이터 | 없음 |

### 상속 계층 — LSP 준수

```
ATPSCharacterBase (Abstract)
 ├─ IMoveable, ISprintable, IJumpable, IInteractable, ITargetable, IDamageable
 ├─ 공통 컴포넌트: State, Status, Footstep, CMC
 │
 └─> ATPSSoldierBase (Abstract) — 전투 캐릭터
      ├─ + IAimable, IEquippable, IFireable
      ├─ + 전투 컴포넌트: Equip, Fire, AnimLayer
      │
      └─> ATPSPlayer — 플레이어
           ├─ + IInterpolable (ADS 보간)
           └─ + Camera, Interaction, AmmoViewModel
```

각 계층은 부모의 기능을 축소하지 않고 확장만 합니다.

### 10개 인터페이스 — ISP

```
Action: IMoveable, ISprintable, IAimable, IJumpable, IEquippable, IFireable, IInteractable
Data:   ITargetable, IDamageable, IInterpolable
```

### DIP — Controller는 구체 타입을 모름

```
Controller → TScriptInterface<IMoveable> → StartMove()
(구체 Character 클래스에 직접 의존하지 않음)
```

---

## MVVM — UI 완전 분리

```
[Model — 게임 로직]           [ViewModel — 데이터 변환]        [View — 렌더링]

TPSWeaponBase                 AmmoViewModel                   AmmoWidget
 ├─ CurrentAmmo ──Push──→     ├─ SetAmmo(cur, max)            ├─ Text: "30/120"
 └─ MaxAmmo                   ├─ UpdateAmmoColor()            └─ Color: Blue/Red
                              │  (20발 이하 → Red)
                              └─ GetCurrentAmmo() ──Pull──→

TPSPlayer                    CrosshairViewModel               HUD Canvas
 └─ StateComponent           ├─ Update(DeltaTime)             ├─ DrawLine × 4
    ├─ bMoving ──Read──→     │  ├─ Moving? → +Spread          └─ DrawDot × 1
    ├─ bSprinting            │  ├─ Sprinting? → +Spread
    ├─ bAiming               │  ├─ Aiming? → ×0.5
    └─ bFiring               │  └─ FInterpTo(Smooth)
                             └─ GetSpreadOffset() ──Pull──→
```

### ItemBox — UI가 인스턴스를 소유하지 않는 설계

```
[View — UI]                    [Model — ItemBox]

ItemBoxWidget                   ATPSItemBox
 ├─ 라이플 버튼 클릭             │  ├─ 무기 인스턴스 소유권
 │   └─ ItemBox->SpawnWeapon()──→│  ├─ SpawnWeapon(EWeaponType)
 │      (UI는 요청만, 생성 안 함) │  │   ├─ SpawnActor<ATPSWeaponBase>()
 │                               │  │   └─ Player.EquipComponent.SetWeapon()
 ├─ 닫기 버튼 클릭              │  └─ 인스턴스 생명주기 관리
 └─ UI는 무기 참조를 갖지 않음   │
```

무기 인스턴스 소유권은 캐릭터가 아닌 ItemBox에 있습니다.
전투가 없는 레벨에서는 불필요한 인스턴스 사전 생성을 방지합니다.

---

## Lyra AnimInstance — 스레드 안전 캐싱

```
[GameThread] NativeUpdateAnimation
 ├─ UObject 접근 (매 프레임, GT 전용)
 │   ├─ Owner→GetVelocity() → CachedVelocity
 │   ├─ Owner→GetActorRotation() → CachedActorRotation
 │   ├─ StateComponent→HasState(Aiming) → bIsAiming
 │   └─ CharacterMovement→IsFalling() → bIsFalling
 │
 │  ──── 캐싱 완료, UObject 접근 끝 ────
 │
 ▼
[WorkerThread] NativeThreadSafeUpdateAnimation
 ├─ 순수 수학 연산만 (UObject 접근 금지)
 │   ├─ GroundSpeed = CachedVelocity.Size2D()
 │   ├─ Direction = CalculateDirection(CachedVelocity, CachedRotation)
 │   ├─ AimPitch/AimYaw = RotationDelta 계산
 │   └─ RootYawOffset = FInterpTo(Smooth)
 │
 ▼
AnimGraph (블루프린트)
 └─ GroundSpeed, Direction, AimPitch 등 → BlendSpace/StateMachine 구동
```

- **Animation Layer Interface (ALI)**: C++ `LinkAnimLayer`/`UnlinkAnimLayer`로 무기별 애니메이션 교체

---

## 인터랙션 파이프라인

```
[감지] Player → ItemBox 트리거 충돌
        │
        ▼
[등록] ItemBox::OnBeginOverlap()
        ├─ TScriptInterface<IInteractable> = Player
        └─ Player.InteractionComponent.SetCurrentTarget(this)
        │
        ▼
[입력] F키 → Controller → IInteractable::Interact() → Player → InteractionComponent
        │
        ▼
[실행] InteractionComponent::OpenInteraction()
        ├─ ItemBox::Interact() → ItemBoxUI 표시
        ├─ EActionState::Interacting 비트플래그 ON
        └─ InputMode → GameAndUI
        │
        ▼
[아이템] 무기 선택 → ItemBox::SpawnWeapon()
        └─ Player.EquipComponent.SetWeapon()
        │
        ▼
[해제] 닫기 or 트리거 이탈
        ├─ EActionState::Interacting OFF
        └─ InputMode → GameOnly
```

감지/등록은 `IInteractable` 인터페이스로 열어두되,
아이템 지급은 `ATPSPlayer`에 직접 접근 — 불필요한 추상화를 피하고 명확한 데이터 흐름 유지.

---

## 오브젝트 풀링 — 프레임 분산 스폰 + 런타임 힙 할당 제로

### 투사체 Actor 풀링

```
[Initialize] DataAsset 로드
 ├─ PoolSize = 1000
 ├─ InitialSpawnCount = 500
 └─ DeferredSpawnBatchSize = 10
       │
       ▼
[OnWorldBeginPlay] 2단계 스폰
 ├─ ① 초기 배치: SpawnBatch(500) — 즉시 사용 가능 보장
 └─ ② 지연 스폰: Timer(16ms 주기) × 10개씩 — 프레임 히치 방지
       │
       ▼
[런타임] Get / Return 패턴
 ├─ Pool.Get() → 비활성 투사체 Pop → 활성화 + 발사
 ├─ Pool.Return() → 비활성화 + Pool.Push
 └─ 풀 고갈 시: 긴급 SpawnActor (Warning 로그) — 게임 중단 없음
```

### ECS 메모리 풀링

```
TArray<entity, TInlineAllocator<3000>>  — 스택에 3000개 사전 할당, 힙 할당 제로
SpawnQueue.Reset()                      — 메모리 해제 없이 카운트만 초기화
DamageQueue.Reset()                     — 동일 패턴, 매 프레임 큐 재활용
FlowField::Initialize()                — 16,384셀 1회 할당, 런타임 재할당 없음
```

**공통 철학**: 런타임 핫패스에서 `new`/`SpawnActor`를 호출하지 않는다.

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
- **원인**: R→W→P 리팩토링 후 `CRenderProxyPrev`(stale)에서 인덱스를 읽음. Swap-back이 Current를 갱신하는데, Prev는 이전 프레임 값
- **해결**: `CRenderProxy`(Current)에서 InstanceIndex 읽기로 변경

### FlowField 빌드 실패

- **증상**: BaseLocation이 ZeroVector, FlowField 미빌드
- **원인**: `OnWorldBeginPlay()`가 `ATPSAllyBase::BeginPlay()` 이전에 호출 — 기지 위치 미등록
- **해결**: FlushSpawnQueue에서 TPSTargetSubsystem Lazy 폴링 추가

### 분리 시스템 적 겹침

- **증상**: 적들이 한 점에 수렴하여 겹침
- **원인 1**: `FMath::FRandRange`가 ParallelFor에서 thread-safe하지 않아 모든 스레드가 동일 방향으로 밀어냄
- **원인 2**: 소프트 보정(속도 블렌딩)이 구조적으로 불충분 — 겹침 해소 속도가 겹침 생성 속도보다 느림
- **해결**: 인덱스 해시 결정론적 방향 + 하드 푸시(위치 직접 보정) + Push.Z=0

---

## 빌드

Unreal Engine 5.7 필요. EnTT는 ThirdParty로 포함되어 별도 설치 불필요.

```
1. UE 5.7 설치
2. TPS.uproject 더블클릭 → 솔루션 생성
3. Development Editor 빌드
```

---

## 라이선스

MIT
