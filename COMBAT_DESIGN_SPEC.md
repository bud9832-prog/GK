# COMBAT_DESIGN_SPEC.md — Skill 1 전투 기획 명세 (v0.1, 초안)

> **작성자:** 에이전트 P (기획자)
> **상위 문서:** `AI_AGENTS_GUIDE.md` §3 (아키텍처 확정), §4 (보류 항목)
> **연계 코드:** `Source/GK/GKCharacter.h` (`AGKCharacter`, `EGKCombatState`, 오디오 훅 5종)
> **상태:** 기획 검토 대기 (D 검증 → KiHoon 결재 → B 구현)

---

## 0. 개요 · 범위 · 전제

### 0-1. 본 문서 범위 (Skill 1 / Stage 1 착수분)
| 포함 ✅ | 제외 ⛔ (보류) |
| :--- | :--- |
| 약공격 3단 콤보 | 강공격 (Stage 1 클리어 후 해금 — `§4`) |
| 회피 (구르기) + i-frame | 패링 (Stage 1 클리어 후 해금 — `§4`) |
| 스테미나 (최대/회복/소모) | 궁극기 / 마법 프로젝타일 (Stage 2 클리어 후) |
| 히트스톱 (Hit Stop) | 적 AI 행동·피격 반응 (별도 명세) |
| 입력 매핑 (IMC_Default 기준) | 무기 장착/교체 시스템 |
| 몽타주 슬롯 명세 | 사운드 구현 · Wwise 이벤트 (KiHoon 영역) |

### 0-2. 전제 (Data-Driven · `§3-4`)
- 본 명세의 모든 수치는 **`UGKCombatConfig`(UPrimaryDataAsset)** 또는 **`EditDefaultsOnly`**로 외부화한다.
- C++ `.cpp`에 매직 넘버를 박지 않는다. 기획자(KiHoon)가 **에디터에서 재컴파일 없이** 조정 가능해야 한다.
- 본 문서의 값은 **참고 시작값(Reference Start Value)** 또는 **권장 범위**이며, **최종 수치는 KiHoon이 에디터에서 결정**한다.

### 0-3. 단위 표기 규약
- **시간:** `s` (초, float)
- **비율/확률:** `%` (퍼센트, 0~100 float)
- **개수/단계:** 정수 (int32)
- **속도:** `cm/s` (UE 표준)
- **필수/선택:** ✅ 필수 (구현·테스트에 반드시 필요) / ⬜ 선택 (생략 가능, 기본값 사용)

---

## 1. 산출물 1 — 전투 수치표

### 1-1. 스테미나 (Stamina)

| ID | 항목 | 단위 | 참고 시작값 | 필수 | 비고 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| ST-01 | `MaxStamina` (최대 스테미나) | float (정수 권장) | `100` | ✅ | 현재 `AGKCharacter.h`에 선언됨 |
| ST-02 | `StaminaRegenRate` (회복 속도) | `%/s` | `25` | ✅ | 1초당 최대값의 N% 회복 |
| ST-03 | `StaminaRegenDelay` (회복 지연) | `s` | `1.0` | ✅ | 행동 종료 후 회복 시작까지 대기 |
| ST-04 | `StaminaCost_Attack_Combo1` | float | `20` | ✅ | 콤보 1단 소모 |
| ST-05 | `StaminaCost_Attack_Combo2` | float | `25` | ✅ | 콤보 2단 소모 |
| ST-06 | `StaminaCost_Attack_Combo3` | float | `30` | ✅ | 콤보 3단 소모 (피니셔) |
| ST-07 | `StaminaCost_Evade` | float | `25` | ✅ | 회피 1회 소모 |
| ST-08 | `MinStaminaToAttack` (공격 최소 요구치) | float | `1` | ⬜ | 0보다 크면 가능 / 임계값 설정 가능 |
| ST-09 | `MinStaminaToEvade` (회피 최소 요구치) | float | `1` | ⬜ | 동일 |
| ST-10 | `bAllowNegativeStamina` (마이너스 허용) | bool | `false` | ⬜ | 다크소울식 (마지막 행동은 0이어도 발동) |

**결정 필요 (KiHoon):**
- **회복 정책 2안 선택**
  - **안 A (소울라이크형):** 행동 종료 후 `StaminaRegenDelay`(1초) 대기 → 지속 회복. 회피·공격 중에도 1초 지나면 다음 행동과 무관하게 회복 시작.
  - **안 B (세키로형):** `EGKCombatState::Idle` 진입 시에만 회복. Attacking·Evading·HitStun 중에는 회복 정지. 더 긴장감 있는 페이스.
  - 트레이드오프: A는 너그러움/조작 자유도 높음. B는 더 신중한 자원 관리 강제.

### 1-2. 콤보 윈도우 (Combo Window)

| ID | 항목 | 단위 | 참고 시작값 | 필수 | 비고 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| CB-01 | `MaxComboIndex` (최대 콤보 단계) | int32 | `3` | ✅ | 현재 `ComboIndex` 변수 존재 |
| CB-02 | `ComboInputWindow_OpenRatio` (몽타주 내 입력 가능 시작 비율) | `%` | `40` | ✅ | 몽타주 진행률 40%부터 다음 콤보 입력 받음 |
| CB-03 | `ComboInputWindow_CloseRatio` (몽타주 내 입력 가능 종료 비율) | `%` | `95` | ✅ | 몽타주 진행률 95%까지 |
| CB-04 | `ComboResetTime` (콤보 끊김 판정) | `s` | `0.6` | ✅ | 마지막 공격 종료 후 N초 내 미입력 시 콤보 1단으로 리셋 |
| CB-05 | `bUseInputBuffer` (입력 버퍼 사용) | bool | `true` | ✅ | true면 윈도우 전 입력도 큐잉 |
| CB-06 | `InputBufferDuration` (입력 버퍼 유지) | `s` | `0.3` | ⬜ | 버퍼 사용 시만 의미 있음 |
| CB-07 | `ComboCancelByEvade` (회피로 콤보 캔슬 가능) | bool | `true` | ✅ | 콤보 도중 회피 입력 시 즉시 회피로 전환 |
| CB-08 | `ComboCancelWindow_OpenRatio` (회피 캔슬 가능 시작) | `%` | `20` | ⬜ | 너무 이르면 페인트 남발 / 너무 늦으면 답답함 |

**결정 필요 (KiHoon):**
- **콤보 입력 모델 2안 선택**
  - **안 A (윈도우만):** `CB-05 = false`. 정확히 `OpenRatio~CloseRatio` 구간 입력만 유효. 정밀하지만 답답함.
  - **안 B (윈도우 + 버퍼):** `CB-05 = true`. 윈도우 전 입력도 큐잉되어 `OpenRatio` 도달 시 자동 발사. 조작감 부드러움 (현재 권장).
  - 트레이드오프: A는 입력 정확성 요구. B는 캐주얼하지만 의도치 않은 콤보 발동 가능.

### 1-3. i-frame (회피 무적 프레임)

| ID | 항목 | 단위 | 참고 시작값 | 필수 | 비고 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| IF-01 | `EvadeTotalDuration` (회피 전체 시간) | `s` | `0.6` | ✅ | 회피 모션 전체 길이 |
| IF-02 | `IFrameStartTime` (i-frame 시작 시점) | `s` | `0.05` | ✅ | 회피 시작 후 N초부터 무적 |
| IF-03 | `IFrameDuration` (i-frame 지속) | `s` | `0.35` | ✅ | 무적 유지 시간 |
| IF-04 | `EvadeRecoveryTime` (후딜 = 회피 종료 후 행동 불가) | `s` | `0.1` | ⬜ | 회피 종료 직후 다른 행동 제한 |
| IF-05 | `EvadeDistance` (회피 거리) | `cm` | `500` | ✅ | 회피 한 번에 이동하는 거리 |
| IF-06 | `EvadeSpeed` (회피 속도) | `cm/s` | `1000` | ⬜ | `EvadeDistance / EvadeTotalDuration` 자동 계산 또는 별도 지정 |
| IF-07 | `bDirectional` (방향 회피) | bool | `true` | ✅ | true: 입력 방향 / false: 무조건 전방 |
| IF-08 | `bDefaultBackstepIfNoInput` (입력 없을 시 백스텝) | bool | `true` | ⬜ | 방향 입력 없으면 뒤로 |

**결정 필요 (KiHoon):**
- **i-frame 발동 타이밍 2안 선택**
  - **안 A (즉발):** `IF-02 = 0.0`. 입력 즉시 무적. 관대함.
  - **안 B (소울라이크 지연):** `IF-02 = 0.05~0.1`. 첫 프레임은 위험. 정밀 조작 강제 (참고값 기준).
  - 트레이드오프: A는 입문자 친화. B는 도전적, 소울라이크 정체성.

### 1-4. 히트스톱 (Hit Stop)

| ID | 항목 | 단위 | 참고 시작값 | 필수 | 비고 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| HS-01 | `HitStopDuration_Default` | `s` | `0.08` | ✅ | 통상 타격 히트스톱 |
| HS-02 | `HitStopDuration_ComboFinisher` | `s` | `0.15` | ⬜ | 콤보 3단(피니셔)에 강조 |
| HS-03 | `HitStopTimeDilation` (시간 배율) | float (0~1) | `0.05` | ✅ | 0: 완전 정지 / 0.05: 거의 정지 / 1.0: 정지 없음 |
| HS-04 | `HitStopTarget` (적용 대상) | enum | `Both` | ✅ | `Attacker` / `Victim` / `Both` |
| HS-05 | `bHitStopAffectsCamera` (카메라 셰이크 동반) | bool | `false` | ⬜ | true 시 작은 카메라 셰이크 추가 (셰이크 자체는 별도 자산) |
| HS-06 | `bSkipHitStopOnKill` (처치 시 생략) | bool | `false` | ⬜ | true 시 처치 타격은 즉시 진행 (사망 모션과 충돌 방지) |

**결정 필요 (KiHoon):**
- **히트스톱 적용 대상 3안 선택**
  - **안 A (`Attacker`만):** 공격자만 정지. 적은 자연스럽게 반응. 가벼운 묵직함.
  - **안 B (`Victim`만):** 피격자만 정지. 격투게임식 강한 임팩트.
  - **안 C (`Both`):** 양쪽 모두 정지. 가장 강한 임팩트 (참고값 기준). 다수 적 상황에서 부자연스러울 수 있음.
  - 트레이드오프: Stage 2 난전 대비 시 C는 부자연스러움 ↑. Stage 1 1:1에는 C가 가장 강력함.

---

## 2. 산출물 2 — 입력 / 트리거 표

### 2-1. Input Action 목록 (IMC_Default 기준)

| 상태 | Input Action 이름 | 자산 경로 (계획) | 트리거 (Enhanced Input) | 용도 | 필수 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| ✅ 기존 | `IA_Move` | `/Content/Input/Actions/IA_Move` | `Triggered` | 이동 (Vector2D) | ✅ |
| ✅ 기존 | `IA_Look` | `/Content/Input/Actions/IA_Look` | `Triggered` | 카메라 회전 (Vector2D) | ✅ |
| ✅ 기존 | `IA_Jump` | `/Content/Input/Actions/IA_Jump` | `Started`/`Completed` | 점프 | ⚠️ 결정 필요 (아래) |
| 🆕 신규 | `IA_Attack` | `/Content/Input/Actions/IA_Attack` (계획) | `Started` | 약공격 (Stage 1 범위) | ✅ |
| 🆕 신규 | `IA_Evade` | `/Content/Input/Actions/IA_Evade` (계획) | `Started` | 회피 (구르기) | ✅ |
| ⏸ 보류 | `IA_HeavyAttack` | — | — | 강공격 (Stage 1 클리어 후 해금) | ⛔ |
| ⏸ 보류 | `IA_Parry` | — | — | 패링 (Stage 1 클리어 후 해금) | ⛔ |

**신규 IA 생성은 B(구현)의 작업.** P는 명세만 제공.

### 2-2. 키 바인딩 권장안 (IMC_Default 매핑)

| Input Action | 키보드 (안 A) | 키보드 (안 B) | 게임패드 (참고) | 결정 필요 |
| :--- | :--- | :--- | :--- | :--- |
| `IA_Move` | WASD | WASD | Left Stick | — (확정) |
| `IA_Look` | Mouse XY | Mouse XY | Right Stick | — (확정) |
| `IA_Jump` | Space | **삭제 또는 V** | Face Button Bottom (A/X) | ⚠️ Stage 1 점프 필요 여부 |
| `IA_Attack` | Mouse Left | Mouse Left | Face Button Right (B/○) | ⚠️ 확인 필요 |
| `IA_Evade` | **Space** (안 A) / Left Shift (안 B) | Space | Face Button Bottom (A/X) | ⚠️ 점프 키와 충돌 |

**결정 필요 (KiHoon):**
- **점프 vs 회피 키 충돌**
  - **안 A (소울라이크 표준):** Space = 회피, 점프는 V 또는 삭제. Stage 1 레벨에 점프 불필요한 평지 위주면 가장 자연스러움.
  - **안 B (점프 유지):** Space = 점프, 회피는 Left Shift. 두 키 모두 점프와 회피 동시 사용 가능. 손가락 거리 부담.
  - 현재 코드: `IA_Jump`가 이미 바인딩되어 있음 (`AGKCharacter.cpp` L77). 점프 폐지 결정 시 코드에서 제거 필요(B 작업).
- **공격 키 (마우스 좌 vs 키보드)**
  - 안 A: Mouse Left (소울라이크 표준)
  - 안 B: 키보드 J/K (격투게임 스타일)

### 2-3. 트리거 규칙 (탭 / 홀드 / 버퍼)

| Action | 입력 트리거 (Enhanced Input) | 탭/홀드 | 버퍼링 정책 | 비고 |
| :--- | :--- | :--- | :--- | :--- |
| `IA_Attack` | `Started` | **탭 전용** (Stage 1) | `bUseInputBuffer=true` 시 윈도우 전 입력 큐잉 (`CB-06` 0.3s) | 홀드형 강공격은 Stage 2 이후 |
| `IA_Evade` | `Started` | 탭 전용 | 콤보 중 입력 시 캔슬 회피 (`CB-07`) | 홀드 시 무시 |
| `IA_Move` | `Triggered` | (연속) | — | 회피 방향 입력 판정용 (회피 직전 0.1s 이내 Move Axis 사용) |
| `IA_Jump` (유지 시) | `Started`/`Completed` | 탭 점프, 홀드 시 점프 유지 | — | 현행 유지 시 변경 없음 |

**결정 필요 (KiHoon):**
- **회피 방향 판정 시점**
  - 안 A: 회피 입력 직전 0.1초의 Move Axis 값 사용 (조작감 자연스러움).
  - 안 B: 회피 입력과 동시에 Move 키가 눌려 있는지 즉시 확인 (정밀).

### 2-4. 입력 실패 / 예외 처리

| 케이스 | 조건 | 처리 | 입력 무시? | 오디오 훅 호출? |
| :--- | :--- | :--- | :--- | :--- |
| 스태미나 부족 (공격) | `Stamina < MinStaminaToAttack` | 입력 무시 또는 짧은 휘청임 모션 | 결정 필요 | ⛔ 호출 안 함 |
| 스태미나 부족 (회피) | `Stamina < MinStaminaToEvade` | 입력 무시 | ✅ 무시 | ⛔ 호출 안 함 |
| 상태 잠금 (`HitStun`) | `CombatState == HitStun` | 모든 액션 입력 무시. 단 회피는 결정 필요 (HitStun 캔슬 가능 여부) | 부분 무시 | ⛔ 호출 안 함 |
| 상태 잠금 (`Evading`) | `CombatState == Evading` | 회피 중 추가 회피 입력은 버퍼링하지 않음. 공격은 `EvadeRecoveryTime` 경과 후 가능 | ✅ 무시 | ⛔ 호출 안 함 |
| 상태 잠금 (`Attacking`) | `CombatState == Attacking` | 콤보 윈도우 외 공격 입력은 버퍼링(`CB-05`). 회피는 `ComboCancelByEvade`에 따름 | 부분 무시 | ⛔ 호출 안 함 |
| 점프 중 (`IsFalling`) | `GetCharacterMovement()->IsFalling()` | 공격·회피 입력 무시 (Stage 1 범위) | ✅ 무시 | ⛔ 호출 안 함 |
| 사망 (향후) | (HP=0, Stage 1 범위 밖) | 모든 입력 무시 | ✅ 무시 | — |

**결정 필요 (KiHoon):**
- **스태미나 부족 시 공격 입력 처리**
  - 안 A: 완전 무시. 아무 반응 없음.
  - 안 B: 짧은 휘청임/지친 모션 재생 (별도 몽타주 필요 — 보류).
- **HitStun 중 회피 캔슬 허용 여부**
  - 안 A: 허용 (회복 기회 부여, 너그러움).
  - 안 B: 불허 (피격 페널티 유지, 소울라이크 정체성).

---

## 3. 산출물 3 — 애니메이션 / 몽타주 맵핑표

### 3-1. 콤보 1~3단 몽타주

| ID | 단계 | 임시 슬롯명 | 몽타주 경로 (TBD) | 상태 전이 | 오디오 훅 호출 타이밍 | 확인 필요 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| AM-01 | 콤보 1단 | `AM_Combo_01_Slot` | `/Content/Characters/.../AM_Combo_01` (TBD) | Idle → Attacking → Idle | `OnWeaponSwing(1)` 호출 (몽타주 내 Notify 권장 — Skill 3 보류 항목) | ⚠️ 몽타주 자산 미존재 |
| AM-02 | 콤보 2단 | `AM_Combo_02_Slot` | `/Content/Characters/.../AM_Combo_02` (TBD) | Attacking → Attacking | `OnWeaponSwing(2)` | ⚠️ 동일 |
| AM-03 | 콤보 3단 (피니셔) | `AM_Combo_03_Slot` | `/Content/Characters/.../AM_Combo_03` (TBD) | Attacking → Idle | `OnWeaponSwing(3)` | ⚠️ 동일 |

### 3-2. 회피 몽타주

| ID | 단계 | 임시 슬롯명 | 몽타주 경로 (TBD) | 상태 전이 | 오디오 훅 호출 타이밍 | 확인 필요 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| AM-04 | 회피 시작 | `AM_Evade_Start_Slot` | `/Content/Characters/.../AM_Evade_Roll` (TBD) | Idle/Attacking → Evading | 몽타주 시작 시 `OnEvadeStart()` | ⚠️ 자산 미존재 |
| AM-05 | 회피 종료 | (AM-04 후반부) | (AM-04에 포함) | Evading → Idle | i-frame 종료 시점(`IF-02+IF-03`)에 `OnEvadeEnd()` | ⚠️ 회피 시작/종료 별도 몽타주? 통합? 결정 필요 |

**결정 필요 (KiHoon):**
- **회피 몽타주 구성 2안**
  - **안 A (단일 몽타주):** 시작 + 종료가 하나의 몽타주. `OnEvadeStart`는 몽타주 시작, `OnEvadeEnd`는 Anim Notify로 호출 (Skill 3 보류).
  - **안 B (분할 몽타주):** Start/End 분리. 캔슬 처리 유연. 자산 2개 필요.

### 3-3. 피격 / 히트스톱 연계

| ID | 단계 | 임시 슬롯명 | 몽타주 경로 (TBD) | 상태 전이 | 오디오 훅 호출 타이밍 | 확인 필요 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| AM-06 | 피격 (Hit React) | `AM_HitReact_Slot` | `/Content/Characters/.../AM_HitReact` (TBD) | * → HitStun → Idle | 피격 처리 시점에 `OnHitDamage(HitLocation, Attacker)` 1회 호출 | ⚠️ 자산 미존재 / 방향별 분기 여부 결정 필요 |
| AM-07 | 히트스톱 (모션 정지) | (몽타주 아님) | — | (현 상태 유지, `CustomTimeDilation` 변경) | 히트 적중 시점에 `OnHitDamage` + Time Dilation 조정. 별도 오디오 훅 없음 | — |

**결정 필요 (KiHoon):**
- **피격 몽타주 방향 분기 여부**
  - 안 A: 단일 몽타주 (간단, Stage 1 적합).
  - 안 B: 4방향 (Front/Back/Left/Right) 몽타주. 자산 4개. Stage 2 난전에 유리.

### 3-4. 누락 / 확인 필요 항목 (총 정리)

| 항목 | 상태 | 비고 |
| :--- | :--- | :--- |
| 콤보 1~3단 몽타주 자산 (`AM_Combo_01~03`) | ⚠️ 미존재 | KiHoon 아트 에셋 준비 중 (`§4` 보류) |
| 회피 몽타주 자산 (`AM_Evade_Roll`) | ⚠️ 미존재 | 동일 |
| 피격 몽타주 자산 (`AM_HitReact`) | ⚠️ 미존재 | 동일 |
| Animation Notify (오디오 노티파이) | ⛔ 보류 | Skill 3 미정의 (`§5`) |
| 무기 액터 / 무기 충돌 컴포넌트 | ⛔ 별도 명세 | 본 문서 범위 밖 |
| 적 캐릭터 / AI / 피격 처리 | ⛔ 별도 명세 | 본 문서 범위 밖 (적이 없으면 `OnHitDamage` 테스트 불가) |
| 피지컬 머티리얼 6종 | ⛔ Skill 4 보류 | `OnFootstep` Switch 매핑 |
| 강공격 / 패링 / 궁극기 | ⛔ §4 보류 | Stage 클리어 시 해금 — 본 명세 범위 밖 |

---

## 4. 데이터 주도 적용 — `UGKCombatConfig` 권장 구조 (B 구현 참고)

> 본 절은 **B(구현)의 참고용**. P는 명세만 제공. 실제 구조 결정은 A(설계)·B 협의.

```
UGKCombatConfig : UPrimaryDataAsset
├─ Stamina (Struct FGKStaminaConfig)
│   ├─ MaxStamina, RegenRate, RegenDelay
│   ├─ Cost_Combo1, Cost_Combo2, Cost_Combo3
│   ├─ Cost_Evade
│   ├─ MinToAttack, MinToEvade
│   └─ bAllowNegative
├─ Combo (Struct FGKComboConfig)
│   ├─ MaxComboIndex
│   ├─ InputWindow_OpenRatio, CloseRatio
│   ├─ ResetTime
│   ├─ bUseInputBuffer, InputBufferDuration
│   └─ bCancelByEvade, CancelWindow_OpenRatio
├─ Evade (Struct FGKEvadeConfig)
│   ├─ TotalDuration, IFrameStart, IFrameDuration
│   ├─ RecoveryTime
│   ├─ Distance, Speed
│   └─ bDirectional, bDefaultBackstepIfNoInput
└─ HitStop (Struct FGKHitStopConfig)
    ├─ Duration_Default, Duration_Finisher
    ├─ TimeDilation
    ├─ Target (enum: Attacker/Victim/Both)
    ├─ bAffectsCamera, bSkipOnKill
    (몽타주 참조는 캐릭터 BP의 EditDefaultsOnly가 적절 — Config 분리 가능)
```

- **현재 `AGKCharacter.h`에 선언된 `MaxStamina`**는 Config 도입 시 **Config로 이전** 권장. 또는 EditDefaultsOnly 유지 후 Config가 Override.

---

## 5. 오디오 훅 호출 타이밍 (구현 제안 아님, 호출 지점만)

> KiHoon 영역 침범 금지(`§2`). 본 절은 **C++에서 어느 함수가 어느 상태 전이 시점에 훅을 호출하는지**만 명시. 훅 내부 구현은 BP(KiHoon)에서.

| 오디오 훅 (이미 선언됨) | 호출 시점 | 호출 위치 (권장) | 비고 |
| :--- | :--- | :--- | :--- |
| `OnFootstep(SurfaceType)` | 발 접지 프레임 | Anim Notify 또는 `OnLanded` 오버라이드 | Skill 4(피지컬 머티리얼) 미구현 시 `Default` Surface 전달 |
| `OnWeaponSwing(ComboIndex)` | 콤보 휘두름 시작 | 몽타주 재생 직후 (B 구현) | `ComboIndex`는 1~3 |
| `OnEvadeStart()` | 회피 입력 수락 직후 | 회피 진입 함수 첫 줄 | `i-frame` 시작 전 |
| `OnEvadeEnd()` | i-frame 종료 시점 | 타이머 완료 또는 Anim Notify | `IF-02 + IF-03` 경과 시 |
| `OnHitDamage(Loc, Attacker)` | 피격 처리 시점 | TakeDamage 또는 Hit 처리 함수 | Stage 1 범위 밖(적 미존재) — 인터페이스만 준비 |

**원칙:** C++에서는 훅 **호출만**. 사운드 종류·볼륨·믹스는 BP(KiHoon)가 결정. C++ 매개변수만 정확히 전달.

---

## 6. KiHoon 결정 필요 항목 (Top 5)

| 순위 | 항목 | 위치 | 영향도 |
| :---: | :--- | :--- | :--- |
| **1** | **스테미나 회복 정책** (소울라이크 A vs 세키로 B) | §1-1 | 전반적 페이스·난이도. B 구현 시 회복 조건 분기. |
| **2** | **콤보 입력 모델** (윈도우만 A vs 윈도우+버퍼 B) | §1-2 | 조작감 핵심. 버퍼 사용 시 추가 큐잉 로직 필요. |
| **3** | **i-frame 발동 타이밍** (즉발 A vs 지연 B) | §1-3 | 소울라이크 정체성·난이도. |
| **4** | **히트스톱 적용 대상** (Attacker / Victim / Both) | §1-4 | 임팩트 강도. Stage 2 난전 대응에 영향. |
| **5** | **점프 vs 회피 키 충돌** (Space 회피 A vs Space 점프 유지 B) | §2-2 | Stage 1 레벨 디자인에 점프 필요 여부와 연동. 코드 변경(B) 필요. |

**그 외 결정 대기 (Top 5 외):**
- 스태미나 부족 시 공격 처리 (무시 vs 휘청임 모션)
- HitStun 중 회피 캔슬 허용 여부
- 회피 몽타주 구성 (단일 vs 분할)
- 피격 몽타주 방향 분기 (1방향 vs 4방향)
- 공격 키 (Mouse Left vs 키보드)
- 회피 방향 판정 시점 (직전 0.1s vs 동시)

---

## 7. 변경 이력
| 버전 | 날짜 | 작성 | 변경 사항 |
| :--- | :--- | :--- | :--- |
| v0.1 | 2026-05-25 | 에이전트 P | 초안 작성. Stage 1 범위 (콤보·회피·스테미나·히트스톱). Top 5 결정 항목 식별. |
