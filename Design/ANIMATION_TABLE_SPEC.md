# ANIMATION_TABLE_SPEC.md - 애니메이션 매핑 데이터 테이블 명세

> **버전:** v1 (초안 — D 검증 요청 대기)
> **작성:** 에이전트 P (Claude Opus 4.7) · 2026-05-31
> **부모 문서:** `Design/GAME_DESIGN_PILLARS.md`, `Design/SKILL_01_COMBAT_SPEC.md` v3, `Design/SKILL_01B_UNLOCK_SPEC.md` v2
> **SSOT 참조:** `Design/INPUT_MAPPING.md` (입력) · `ART_ASSET_CHECKLIST.md` (아트 자산)

본 명세는 **모든 AnimMontage 매핑의 단일 출처(SSOT)**를 신설하는 구현 요청서다. 현재 분산된 단일 슬롯(`EvadeMontage`·`HealItemMontage`·`HeavyAttackMontage`·`ParryMontage`)과 `DT_ComboAttacks.Montage` 행을 통합한다. 적 측 AnimMontage 매핑도 본 테이블에서 일원화한다.

---

## 0. 사용 규칙

- **단일 출처 원칙:** 모든 AnimMontage 자산 참조의 SSOT는 본 명세가 정의하는 `DT_AnimationMontages`다. SKILL_01·SKILL_01B의 단일 `TObjectPtr<UAnimMontage>` 슬롯은 본 테이블 행 키 참조로 점진 마이그레이션한다.
- **변경 절차:** 신규 AnimMontage(액션·적 종) 추가 시
  1. 본 명세 §3 액션 식별자 enum/태그 갱신
  2. `DT_AnimationMontages` 행 추가
  3. (필요 시) SKILL_01·SKILL_01B·Pillars 명세 참조 동기화
- **제약:** 본 명세는 **데이터 테이블 구조와 매핑 SSOT만 다룬다.** Anim Notify(`OnFootstep` 등 오디오 훅 위치)는 Skill 3(보류)에서, 적 AI 행동 결정은 Stage·적 명세에서 처리한다.

---

## 1. 목적·범위

### 1-1. 목적
- 분산된 AnimMontage 슬롯·행을 **단일 데이터 테이블**로 통합
- **PC vs 적 vs 적 종류**를 한 표에서 구분·필터 가능
- KiHoon·디자이너가 **CSV/에디터로 직접 매핑 교체** 가능 (재컴파일 불필요)
- 실제 동작(상태 머신·콤보·해금 액션)과 **1:1 매핑** 보장

### 1-2. 범위 (v1)
- **포함:** PC AnimMontage 전체 (Stage 1 + Skill 01B 해금 영역) + 적 1종(Stage 1 더미) AnimMontage 골격
- **제외:** 적 AI 행동 결정, Anim Notify 배치, 사운드 트리거 시점 (Skill 3 영역), VFX 시퀀스 매핑

### 1-3. 후속 명세 의존
- Skill 01: 콤보 1·2·3, 회피, 회복약, 피격·사망 (현재 분산 슬롯 → 본 테이블 행으로 마이그레이션)
- Skill 01B: 강공격, 패링 (동일)
- Stage 2+: 신규 적 종, 점프 공격, 궁극기 등 (본 테이블에 행 추가)

---

## 2. 데이터 테이블 설계

### 2-1. 행 구조체 `FGKAnimMontageRow`

```cpp
USTRUCT(BlueprintType)
struct FGKAnimMontageRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Owner")
    EGKAnimOwner OwnerType = EGKAnimOwner::PC;

    UPROPERTY(EditAnywhere, Category = "Owner",
              meta = (EditCondition = "OwnerType == EGKAnimOwner::Enemy"))
    EGKEnemyType EnemyType = EGKEnemyType::None;

    UPROPERTY(EditAnywhere, Category = "Action")
    EGKAnimAction ActionTag = EGKAnimAction::None;

    UPROPERTY(EditAnywhere, Category = "Action")
    int32 ActionVariant = 0;   // 콤보 0/1/2, 회피 방향 등 (없으면 0)

    UPROPERTY(EditAnywhere, Category = "Asset")
    TSoftObjectPtr<UAnimMontage> Montage;

    UPROPERTY(EditAnywhere, Category = "Playback")
    float PlayRate = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Playback")
    float BlendInTime = 0.10f;

    UPROPERTY(EditAnywhere, Category = "Playback")
    float BlendOutTime = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Meta")
    FString Notes;   // 디자이너 메모 (런타임 미사용)
};
```

**설계 근거:**
- `OwnerType + EnemyType + ActionTag + ActionVariant` 4종 키로 모든 매핑 식별
- `TSoftObjectPtr<UAnimMontage>` → 미로드 자산이 빌드 의존 폭증을 방지 (KiHoon Q1 결정 영역 — P 권장: Soft)
- `PlayRate`/`BlendIn/Out`은 코드 매직 넘버 0 원칙 준수 (Data Table에서 조정)
- `Notes`는 CSV 편집 시 디자이너 메모 필드

### 2-2. 보조 enum

```cpp
UENUM(BlueprintType)
enum class EGKAnimOwner : uint8 { PC, Enemy };

UENUM(BlueprintType)
enum class EGKEnemyType : uint8 { None, FirstEnemy /* Stage1 더미 */ };

UENUM(BlueprintType)
enum class EGKAnimAction : uint8
{
    None,
    // 공통
    Idle, Run, Walk,
    HitStun, Death,
    // PC 전용
    Sprint, Jump, JumpAttack,
    Attack_Combo,         // ActionVariant 0/1/2 = 콤보 1/2/3타
    Evade,
    Heal,                 // 회복약 마시기
    HeavyAttack,          // Skill 01B
    Parry_Active,         // Skill 01B
    Parry_Recovery,       // Skill 01B
    // 적 전용
    Enemy_Attack,
    Enemy_Sway,           // 휘청 (Pillars §6-2)
    Enemy_Down,           // 강공격·콤보 종결타 다운
};
```

**설계 근거:**
- `EGKAnimAction` 단일 enum으로 PC·적 액션 모두 식별 (Owner 필터로 분리)
- `ActionVariant`로 콤보 인덱스 등 다중 분기 처리 → `Attack_Combo` 1개 액션 + Variant 3개
- `EGKEnemyType::None`은 PC 행 식별용

### 2-3. 행 키 명명 규칙 (RowName)

| 패턴 | 예시 |
| :--- | :--- |
| **PC:** `PC.<Action>[.<Variant>]` | `PC.Idle`, `PC.Attack_Combo.0`, `PC.Attack_Combo.1`, `PC.Attack_Combo.2`, `PC.Evade`, `PC.HeavyAttack` |
| **적:** `Enemy.<EnemyType>.<Action>` | `Enemy.FirstEnemy.Idle`, `Enemy.FirstEnemy.Attack`, `Enemy.FirstEnemy.Sway`, `Enemy.FirstEnemy.Down`, `Enemy.FirstEnemy.Death` |

> 점(`.`) 구분자로 grep·필터 용이. UE의 RowName은 FName이므로 점 사용 가능.

---

## 3. 행 정의 (v1 초안)

### 3-1. PC 행

| RowName | OwnerType | EnemyType | ActionTag | ActionVariant | 비고 (현 명세 매핑) |
| :--- | :--- | :--- | :--- | :---: | :--- |
| `PC.Idle` | PC | None | Idle | 0 | Pillars §2 |
| `PC.Walk` | PC | None | Walk | 0 | SKILL_01 v3 — Stage 1 미사용 (보유만) |
| `PC.Run` | PC | None | Run | 0 | SKILL_01 v3 §5-1 기본 이동 |
| `PC.Sprint` | PC | None | Sprint | 0 | SKILL_01 v3 §5-1 |
| `PC.Jump` | PC | None | Jump | 0 | SKILL_01 v3 §5-6 |
| `PC.JumpAttack` | PC | None | JumpAttack | 0 | SKILL_01 v3 §5-7 |
| `PC.Attack_Combo.0` | PC | None | Attack_Combo | 0 | SKILL_01 §5-2 — `DT_ComboAttacks.Combo_01.Montage` 대체 |
| `PC.Attack_Combo.1` | PC | None | Attack_Combo | 1 | 동, `Combo_02` 대체 |
| `PC.Attack_Combo.2` | PC | None | Attack_Combo | 2 | 동, `Combo_03` 대체 |
| `PC.Evade` | PC | None | Evade | 0 | SKILL_01 §5-3 — `UGKCombatConfig.EvadeMontage` 대체 |
| `PC.Heal` | PC | None | Heal | 0 | SKILL_01 §5-4 — `UGKPlayerStatsConfig.HealItemMontage` 대체 |
| `PC.HitStun` | PC | None | HitStun | 0 | SKILL_01 v3 상태 머신 |
| `PC.Death` | PC | None | Death | 0 | Pillars §5 |
| `PC.HeavyAttack` | PC | None | HeavyAttack | 0 | SKILL_01B §5-1 — `UGKCombatConfig.HeavyAttackMontage` 대체 |
| `PC.Parry_Active` | PC | None | Parry_Active | 0 | SKILL_01B §5-2 — `UGKCombatConfig.ParryMontage` 분할 |
| `PC.Parry_Recovery` | PC | None | Parry_Recovery | 0 | SKILL_01B §5-2 — 동, Active/Recovery 분리 (KiHoon Q3 결정 영역) |

### 3-2. 적 행 (Stage 1 더미 1종)

| RowName | OwnerType | EnemyType | ActionTag | ActionVariant | 비고 |
| :--- | :--- | :--- | :--- | :---: | :--- |
| `Enemy.FirstEnemy.Idle` | Enemy | FirstEnemy | Idle | 0 | Pillars §6 |
| `Enemy.FirstEnemy.Walk` | Enemy | FirstEnemy | Walk | 0 | Pillars §6 |
| `Enemy.FirstEnemy.Attack` | Enemy | FirstEnemy | Enemy_Attack | 0 | Pillars §6 |
| `Enemy.FirstEnemy.Sway` | Enemy | FirstEnemy | Enemy_Sway | 0 | Pillars §6-2 휘청 (피격) |
| `Enemy.FirstEnemy.Down` | Enemy | FirstEnemy | Enemy_Down | 0 | Pillars §6-2 (강공격·콤보 종결타) |
| `Enemy.FirstEnemy.Death` | Enemy | FirstEnemy | Death | 0 | Pillars §6-2 |

---

## 4. 기존 분산 슬롯과의 관계 (마이그레이션)

| 기존 슬롯 | 대체 (DT_AnimationMontages 행) | 마이그레이션 단계 |
| :--- | :--- | :--- |
| `UGKCombatConfig.EvadeMontage` | `PC.Evade` | B 단계 — Config에서 `RowName` 참조로 변경 (또는 슬롯 deprecate) |
| `UGKPlayerStatsConfig.HealItemMontage` | `PC.Heal` | 동 |
| `UGKCombatConfig.HeavyAttackMontage` | `PC.HeavyAttack` | 동 |
| `UGKCombatConfig.ParryMontage` | `PC.Parry_Active` + `PC.Parry_Recovery` | KiHoon Q3 후 단일 vs 분할 결정 |
| `DT_ComboAttacks.Montage` 컬럼 | `PC.Attack_Combo.0/1/2` 행 (DT_AnimationMontages) | DT_ComboAttacks는 **수치만** 보유 + Variant 인덱스로 DT_AnimationMontages 참조 |

**마이그레이션 권장 (KiHoon Q2 결정 영역):**
- **옵션 A (P 권장):** SKILL_01·SKILL_01B의 Config 단일 슬롯들을 **`FName RowName` 필드로 교체**. AGKCharacter는 `DT_AnimationMontages->FindRow(RowName)`으로 조회. 기존 단일 슬롯은 즉시 제거.
- **옵션 B:** 단일 슬롯과 DT 병행 (DT가 정의되어 있으면 DT 우선, 미정의 시 슬롯 폴백). 마이그레이션 안전성↑·복잡도↑.

---

## 5. AGKCharacter / AGKEnemyCharacter 연동

### 5-1. 노출 슬롯 (P 권장 — 옵션 A 기준)

```cpp
// AGKCharacter
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Table")
TObjectPtr<UDataTable> AnimMontageTable;   // DT_AnimationMontages

// AGKEnemyCharacter — 동일 테이블 공유 (Owner 필터로 분기)
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim|Table")
TObjectPtr<UDataTable> AnimMontageTable;   // 동일 테이블, EnemyType 행만 사용
```

### 5-2. 조회 헬퍼 (B 구현 권장 시그니처)

```cpp
UFUNCTION(BlueprintCallable, Category = "Anim|Table")
UAnimMontage* GetMontageForAction(EGKAnimAction Action, int32 Variant = 0) const;

UFUNCTION(BlueprintCallable, Category = "Anim|Table")
const FGKAnimMontageRow* GetRowForAction(EGKAnimAction Action, int32 Variant = 0) const;
```

- AGKCharacter는 `OwnerType = PC` 고정으로 조회
- AGKEnemyCharacter는 자신의 `EnemyType` 필드를 동반 필터
- Soft Reference는 `LoadSynchronous` 또는 비동기 로드 (KiHoon Q1 결정 후 확정)

### 5-3. 상태 머신 정합

| 상태 (SKILL_01 v3 / SKILL_01B v2) | DT_AnimationMontages 조회 키 |
| :--- | :--- |
| `Idle` | `(PC, Idle, 0)` |
| `Run` / `Sprint` | `(PC, Run/Sprint, 0)` |
| `Attack_Combo[i]` | `(PC, Attack_Combo, i)` |
| `Evade_Active` / `Evade_Recovery` | `(PC, Evade, 0)` (단일 몽타주 분할 재생 또는 분할 행 — Q3) |
| `Heal` | `(PC, Heal, 0)` |
| `HitStun` / `Death` | `(PC, HitStun/Death, 0)` |
| `Jump` / `JumpAttack` | `(PC, Jump/JumpAttack, 0)` |
| `HeavyAttack` | `(PC, HeavyAttack, 0)` |
| `Parry_Active` / `Parry_Recovery` | `(PC, Parry_Active/Parry_Recovery, 0)` |

> 상태 머신 자체는 변경 없음. **조회 경로만** Config 단일 슬롯 → DT 참조로 전환.

---

## 6. 오디오 훅 (변경 없음 확인)

- 본 명세는 **데이터 매핑 SSOT**만 다룬다. AnimMontage 안의 `Anim Notify`(`OnFootstep`/`OnWeaponSwing` 등 오디오 훅) 배치는 **Skill 3 (Animation_Notify_Injector — 보류)**의 영역.
- SKILL_01 v3 §6·SKILL_01B v2 §6의 훅 시그니처(`OnWeaponSwing(0~4)` sentinel, `OnHealItem*`, `OnParry*`, `OnEvadeStart/End`)는 **본 작업에서 변경 없음**.
- KiHoon 사운드 의사결정 영역 침범 금지 원칙 준수.

---

## 7. 아트 의존성

### 7-1. ART_ASSET_CHECKLIST 정합

본 테이블은 ART_ASSET_CHECKLIST.md Tier 0/1/2의 모든 애니메이션 항목과 1:1 대응한다.

| ART Tier | 항목 | DT_AnimationMontages 행 |
| :--- | :--- | :--- |
| Tier 0 | Idle/Walk/Run/Sprint 루프 | `PC.Idle`, `PC.Walk`, `PC.Run`, `PC.Sprint` |
| Tier 0 | 회복약 마시기 몽타주 | `PC.Heal` |
| Tier 0 | 적 Idle/Walk/Attack/Sway/Down/Death | `Enemy.FirstEnemy.*` 6행 |
| Tier 1 | 콤보 1·2·3 몽타주 | `PC.Attack_Combo.0/1/2` |
| Tier 1 | 회피 몽타주 | `PC.Evade` |
| Tier 1 | 피격 리액션 몽타주 | `PC.HitStun` |
| Tier 1 | 점프 Start/Airborne/Land Set | `PC.Jump` (Set 통합 또는 분할 — Q4) |
| Tier 1 | 점프 공격 몽타주 | `PC.JumpAttack` |
| Tier 2 | 강공격 몽타주 | `PC.HeavyAttack` |
| Tier 2 | 패링 자세·성공 몽타주 | `PC.Parry_Active` + `PC.Parry_Recovery` |
| Tier 2 | 사망 몽타주 (PC) | `PC.Death` |

### 7-2. 플레이스홀더 전략 (실 자산 수령 전)
- 모든 행의 `Montage` 슬롯에 UE5 ThirdPerson Template 기본 몽타주 또는 마네킹 펀치 임시 할당
- 실 자산 수령 시 행별 `Montage` 슬롯만 교체 (재컴파일 불필요)
- CSV Export → 자산 경로 일괄 교체 → Import 워크플로 보장

---

## 8. KiHoon 결정 항목 — **[KiHoon 2026-05-31 일괄 승인 — Q1~Q7 P 권장 모두 채택]**

> 본 표의 모든 항목(Q1~Q7)은 KiHoon 2026-05-31 일괄 승인되어 **P 권장값으로 확정**됨. D 검증(R1~R5)은 별도 진행.

| Q | 항목 | 확정값 (P 권장 채택) | 트레이드오프 (참고용) |
| :--- | :--- | :--- | :--- |
| **Q1** | AnimMontage 참조 방식 | **`TSoftObjectPtr<UAnimMontage>`** | Soft: 빌드 의존 폭증 방지·비동기 로드 가능 / Hard: 즉시 사용·코드 단순. **권장: Soft** (테이블이 점점 커질수록 빌드 의존 폭증 위험) |
| **Q2** | 기존 Config 단일 슬롯 마이그레이션 | **옵션 A — 즉시 교체** | A: SSOT 일관 / B: 안전성↑·복잡도↑. **권장: A** (현재 미구현 상태라 비용 낮음) |
| **Q3** | 패링 몽타주 단일 vs 분할 | **분할 (Parry_Active + Parry_Recovery)** | 분할: 상태 머신 직접 매핑·캔슬 윈도우 명확 / 단일: 자산 1개로 충분. **권장: 분할** (SKILL_01B §5-2의 캔슬 정책과 정합) |
| **Q4** | Jump Set 단일 vs 분할 (Start/Airborne/Land) | **단일 (`PC.Jump`)** | 단일: 자산 1개·재생 시점 코드 단순 / 분할: Start→Airborne→Land 3-phase 정밀. **권장: 단일** (UE Jump 기본 패턴, ART Tier 1 표기와 정합 — "Set" = 1개 자산 묶음) |
| **Q5** | `EGKEnemyType` 초기 값 범위 | **`None` + `FirstEnemy` 2개** | Stage 1 더미 1종. Stage 2+에서 추가 (`SecondEnemy`, `Boss` 등). **권장: 2개 시작 + 후속 확장** |
| **Q6** | DT_ComboAttacks 행 키 명명 규칙 변경 여부 | **유지 (`Combo_01`/`02`/`03`)** | 변경: `PC.Attack_Combo.0/1/2`로 통일 / 유지: 기존 명세·CSV 영향 없음. **권장: 유지** (DT_ComboAttacks는 수치 전용, `ActionVariant` 인덱스로 DT_AnimationMontages 참조) |
| **Q7** | `Notes` 필드 노출 여부 | **유지 (디자이너 메모용)** | 유지: 디자이너 편집 편의 / 제거: 행 슬림화. **권장: 유지** (CSV 편집 시 가독성↑) |

---

## 9. D 검증 요청 포인트

### R1. 데이터 테이블 구조 적정성
- `OwnerType + EnemyType + ActionTag + ActionVariant` 4종 키 조합으로 충돌 없이 모든 매핑 식별 가능한지
- `FGKAnimMontageRow` 필드 9개가 실제 동작에 필요·충분한지
- 향후 적 추가(Stage 2~) 시 enum 확장만으로 충분한지 (구조체 변경 없이)

### R2. `EGKAnimAction` enum 단일 vs 분리
- PC·적 액션을 한 enum에 두는 설계 vs 분리(`EGKPCAction` + `EGKEnemyAction`)의 트레이드오프
- P 권장: 단일 enum + Owner 필터 — 조회 인터페이스 일원화 / 분리 시 컴파일타임 타입 안전성↑
- D 판정 요청: 단일 vs 분리

### R3. Config 단일 슬롯 마이그레이션 영향
- `UGKCombatConfig`의 4개 슬롯(`EvadeMontage`/`HealItemMontage`/`HeavyAttackMontage`/`ParryMontage`)을 `FName RowName`으로 교체 시 SKILL_01 v3·SKILL_01B v2 §7-1·§7-2 명세와 충돌 없는지
- `DT_ComboAttacks.Montage` 컬럼 제거 + `ActionVariant` 인덱스 매핑이 SKILL_01 v3 §7-3 `FGKComboAttackRow`와 충돌 없는지

### R4. Soft vs Hard Reference 선택의 런타임 영향
- `TSoftObjectPtr<UAnimMontage>` 사용 시 첫 재생 지연(로드 비용) 허용 가능 여부
- `PreloadCriticalMontages()` 같은 부팅 시 강제 로드 단계 필요 여부 (회피·콤보 1타 등 즉시 반응이 필요한 행)

### R5. 적 측 AnimMontage 매핑 위치
- 적 측 매핑을 본 테이블에 통합 vs 별도 `DT_EnemyAnimMontages` 분리
- P 권장: 통합 (Owner 필터로 분리 가능) / 분리 시 적별 행 관리 용이

---

## 10. 완료 기준 (Definition of Done)

### 10-1. P 산출물 (본 명세서)
- [x] `Design/ANIMATION_TABLE_SPEC.md` 신설 (본 문서)
- [x] **KiHoon Q1~Q7 일괄 승인 — P 권장 모두 채택 (2026-05-31)**
- [ ] D 검증 R1~R5 통과 → A 설계 브리핑 게이트 진입

### 10-2. A 산출물 (설계 브리핑, D 통과 후)
- [ ] `FGKAnimMontageRow`/`EGKAnimOwner`/`EGKEnemyType`/`EGKAnimAction` 헤더 설계
- [ ] AGKCharacter/AGKEnemyCharacter `AnimMontageTable` 슬롯 + `GetMontageForAction()` 헬퍼 인터페이스
- [ ] DT_ComboAttacks·Config 슬롯 마이그레이션 패치 계획

### 10-3. B 산출물 (구현)
- [ ] C++ 구조체·enum 추가 (`Source/GK/`)
- [ ] `Content/Data/DT_AnimationMontages.uasset` 신설 (또는 자동화 스크립트)
- [ ] PC 행 16개 + 적 행 6개 채움 (Montage 슬롯은 플레이스홀더 허용)
- [ ] AGKCharacter 조회 헬퍼 구현 + 상태 머신 호출부 마이그레이션
- [ ] (Q1=Soft 시) `PreloadCriticalMontages()` 부팅 단계

### 10-4. C 산출물 (검증)
- [ ] Win64 빌드 + Error/Warning 0
- [ ] PIE 런타임에서 행별 Montage 재생 확인 (콤보·회피·회복약·점프 최소)
- [ ] DT CSV Export→Import 왕복 무결성
- [ ] LogWwise Error 0 (오디오 훅 영향 없음 확인)

---

## 11. 후속 산출물 예고

- **`Design/SKILL_01_COMBAT_SPEC.md`**: §7-2/§7-3에서 단일 `Montage` 슬롯/컬럼 제거 + DT_AnimationMontages 참조 표기 (D 통과·B 마이그레이션 완료 시 동기화)
- **`Design/SKILL_01B_UNLOCK_SPEC.md`**: §7-1/§7-2 동
- **`ART_ASSET_CHECKLIST.md`**: 본 테이블 행 키를 비고에 명시 (자산 수령 시 어느 행에 할당할지 직관화)
- **`AI_AGENTS_GUIDE.md` §3-4**: Data Asset·Data Table 정책에 본 테이블을 표준 사례로 추가

---

## 12. 변경 이력

| 일자 | 변경 내용 | 작성자 |
| :--- | :--- | :--- |
| 2026-05-31 | **v1 초안 작성** — KiHoon "애니메이션 매핑 데이터 테이블 + PC/적/적 종 구분 + 실제 동작과 일치" 요청 반영. `FGKAnimMontageRow` 구조체 설계, `EGKAnimOwner`/`EGKEnemyType`/`EGKAnimAction` enum 설계, 행 22개(PC 16 + 적 6) 초안, 기획 변수 외부화·DT_ComboAttacks와의 마이그레이션 전략 제시, KiHoon Q1~Q7 결정 항목 + D R1~R5 검증 포인트 분리 | P |
| 2026-05-31 | **KiHoon Q1~Q7 일괄 승인 — P 권장 모두 채택** (Q1 Soft Reference / Q2 즉시 교체 / Q3 패링 분할 / Q4 Jump 단일 / Q5 None+FirstEnemy / Q6 DT_ComboAttacks 행 키 유지 / Q7 Notes 유지). §8 헤더·§10-1 DoD 노티 추가. 본문 설계·행 정의·구조체 변경 없음. D 검증(R1~R5)은 후속 사이클로 분리 | P |
