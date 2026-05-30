# INPUT_MAPPING.md — 입력 매핑 단일 출처 (SSOT)

> **문서 우선순위:** `AI_AGENTS_GUIDE.md` §0 대원칙 > `AI_AGENTS_GUIDE.md` §2·§3 > `Design/GAME_DESIGN_PILLARS.md` §8 > 본 문서
> **상태:** P 1차 산출물 — KiHoon 1차 검토 대기
> **작성:** 에이전트 P (Claude Opus 4.7) · 2026-05-30
> **부모 문서:** `Design/GAME_DESIGN_PILLARS.md` §8 (입력 매핑 원칙)
> **참조처:** `Design/SKILL_01_COMBAT_SPEC.md` §4, `Scripts/SetupSkill01CombatAssets.py`

본 문서는 본 프로젝트의 **모든 키 입력 정의의 단일 출처(Single Source of Truth)**다. 신규 Skill·UI·디버그 키가 추가될 때 본 문서를 **먼저** 갱신하고 다른 명세서·스크립트·UE 에셋을 동기화한다.

---

## 0. 사용 규칙

- **단일 출처 원칙:** 키 매핑은 본 문서가 단일 출처. SKILL_*_SPEC.md, Pillars §8, `Scripts/*.py`, `Content/Input/IMC_Default` 모두 본 문서를 참조한다.
- **변경 절차:** 키 변경 시
  1. 본 문서 §2 표 갱신
  2. §6 충돌 체크
  3. SKILL_*_SPEC.md 영향 섹션 동기화
  4. `Scripts/SetupSkill01CombatAssets.py` 등 자동화 스크립트 동기화
  5. UE 에디터에서 `IMC_Default` 재구성 (또는 스크립트 재실행)
- **에디터 우선:** 키 변경은 `IMC_Default.uasset`에서 직접 조정해도 무방하나, 본 문서 갱신을 동시에 수행하지 않으면 사일런트 드리프트 발생. 본 문서를 항상 먼저 갱신한다.

---

## 1. 입력 시스템 기반

- **엔진 시스템:** Enhanced Input (`UEnhancedInputComponent` + `UInputMappingContext` + `UInputAction`)
- **IMC(Input Mapping Context):** `/Game/Input/IMC_Default` — 단일 컨텍스트. (Stage·UI별 분리 IMC는 추후 검토)
- **Input Action 폴더:** `/Game/Input/Actions/IA_*`
- **패턴:** 다크소울 클래식 (Pillars §8) — 회피·달리기 통합 입력, 별도 락온, 별도 회복약

---

## 2. Input Action 마스터 표

| Input Action | 키보드 | 게임패드 | 값 타입 | 용도 | 정의 시점 | 정의 출처 |
| :--- | :--- | :--- | :--- | :--- | :---: | :--- |
| `IA_Move` | WASD | 좌스틱 | Axis2D | 캐릭터 이동 | UE 템플릿 기본 | UE5 ThirdPerson Template |
| `IA_Look` | 마우스 X/Y | 우스틱 | Axis2D | 카메라 회전 | UE 템플릿 기본 | UE5 ThirdPerson Template |
| `IA_EvadeSprint` | Space | B (Xbox) | Boolean | 회피(tap) + 달리기(hold) | SKILL_01 v2 | `Design/SKILL_01_COMBAT_SPEC.md` §4-2 |
| `IA_Attack` | LMB | RB | Boolean | 약공격 (콤보 1·2·3) | SKILL_01 v2 | `Design/SKILL_01_COMBAT_SPEC.md` §5-2 |
| `IA_Heal` | E | X (Xbox) | Boolean | 회복약 사용 | SKILL_01 v2 | `Design/SKILL_01_COMBAT_SPEC.md` §5-4 |
| `IA_LockOn` | MMB (마우스 휠 클릭) | R3 (우스틱 클릭) | Boolean | 락온 토글 | SKILL_01 v2 | `Design/SKILL_01_COMBAT_SPEC.md` §5-5 |

> 모든 키 매핑은 KiHoon 위임 → P 확정 (2026-05-29) + D 기술 검증 통과 (2026-05-29 Go).

---

## 3. 통합 입력 패턴 — `IA_EvadeSprint`

다크소울 클래식 패턴: tap = 회피, hold = 달리기.

| 항목 | 위치 | 비고 |
| :--- | :--- | :--- |
| HoldThreshold (분기 임계 시간) | `UGKCombatConfig.SprintHoldThreshold = 0.18s` | Data Asset 노출 |
| Release 기반 tap 판정 | SKILL_01 §4-2-b | press·release 시간차로 판정 |
| 상호배타 게이트 | SKILL_01 §4-2-c | 한 press 사이클당 1액션 |
| 의사 코드 | SKILL_01 §4-2-d | 구현 참조 |

**상세 구현 명세는 `Design/SKILL_01_COMBAT_SPEC.md` §4-2 참조.** 본 문서는 키 매핑만 단일 출처로 한다.

---

## 4. 입력 우선순위 (전투 컨텍스트)

전투 상태에서 동시 입력 처리 우선순위 — 상세는 `Design/SKILL_01_COMBAT_SPEC.md` §4-3 참조.

| 순위 | 입력·상태 |
| :---: | :--- |
| 1 | HitStun / Death (모든 입력 무시) |
| 2 | Heal 모션 진행 중 (회피만 캔슬) |
| 3 | `IA_EvadeSprint` (tap → 회피) |
| 4 | `IA_Attack` (콤보 진행 또는 신규) |
| 5 | `IA_EvadeSprint` (hold → Sprint) |
| 6 | `IA_Move` (Run) |
| - | `IA_LockOn` (독립 토글) |

---

## 5. UE 에셋 위치 (현재 구현 상태)

| 자산 | 경로 | 생성·갱신 주체 |
| :--- | :--- | :--- |
| Input Mapping Context | `/Game/Input/IMC_Default` | UE 템플릿 + `Scripts/SetupSkill01CombatAssets.py`로 갱신 |
| Input Action (`IA_Move`, `IA_Look`) | `/Game/Input/Actions/` | UE 템플릿 |
| Input Action (`IA_EvadeSprint`, `IA_Attack`, `IA_Heal`, `IA_LockOn`) | `/Game/Input/Actions/` | B 에이전트 자동화 스크립트 (`Scripts/SetupSkill01CombatAssets.py`) 생성 |

스크립트 실행 명령: `UnrealEditor-Cmd.exe GK.uproject -ExecutePythonScript=Scripts/SetupSkill01CombatAssets.py`

---

## 6. 충돌 점검 체크리스트

신규 키 추가 시 반드시 확인:

### 6-1. 키 중복 금지
같은 키에 두 IA 매핑 금지. 모디파이어(`Shift`, `Ctrl`)로 분기하더라도 단일 키 단독 매핑은 1IA만 사용.

### 6-2. 마우스 버튼 분배 (3슬롯)
| 슬롯 | 현재 할당 | 예약/여유 |
| :--- | :--- | :--- |
| LMB | `IA_Attack` (약공격) | 확정 |
| RMB | (여유) | **예약:** Skill 01B 강공격(`IA_HeavyAttack`) |
| MMB | `IA_LockOn` | 확정 |

### 6-3. 키보드 핸드 포지션
- **왼손 (WASD 주변):** WASD, Space, E (회복약) — 핸드 위치 자연스러움
- **오른손:** 마우스 + 휠/버튼
- **추후 추가 검토 영역:** Q, R, F, Shift, Ctrl, 1~4 — 비어 있음
- **금지 영역:** WASD 자체에 추가 매핑 금지 (이동 우선)

### 6-4. UE 표준 예약 키 (충돌 주의)
- `ESC` — UE 기본 일시정지/메뉴 (추후 UI에서 처리)
- `Tab` — 추후 UI(인벤토리·메뉴) 예약
- `F1`~`F12` — 디버그/개발자 도구 예약

---

## 7. 후속 산출물 예고

본 문서는 신규 Skill 추가 시점에 갱신된다.

| 예정 IA | 트리거 시점 | 권장 매핑 (P 잠정안 — 확정 시점에 본 표 갱신) |
| :--- | :--- | :--- |
| `IA_HeavyAttack` | Skill 01B (Stage 1 클리어 후) | RMB / RT |
| `IA_Parry` | Skill 01B | (RMB 점유 충돌 가능 — 별도 검토) |
| `IA_Ultimate` | Stage 2 클리어 후 | (미정 — Q 후보) |
| `IA_Menu` | UI 명세 | ESC |
| `IA_DebugToggle` | 디버그 도구 | ` (그레이브) |

---

## 8. 변경 이력

| 일자 | 변경 | 작성자 |
| :--- | :--- | :--- |
| 2026-05-30 | 신설 — SKILL_01 v2 §4 / Pillars §8 / B 자동화 스크립트의 키 매핑을 단일 출처로 통합 | P |
