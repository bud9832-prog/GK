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
- **패턴:** **분리 입력 (SKILL_01 v3 / Pillars §8 v2)** — 회피·달리기·점프 각각 단독 키. tap/hold 분기 없음. 단일 키 → 단일 액션. (v1 통합 입력 `IA_EvadeSprint` 패턴은 §3 이력 보존용)

---

## 2. Input Action 마스터 표 (통합 SSOT — 활성 + 예약)

본 표는 **모든 IA의 단일 출처**다. 상태 라벨로 활성·예약·미정을 구분한다.

| Input Action | 키보드 | 게임패드 | 값 타입 | 용도 | 상태 | 정의 출처 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `IA_Move` | WASD | 좌스틱 | Axis2D | 캐릭터 이동 | **[확정]** | UE5 ThirdPerson Template |
| `IA_Look` | 마우스 X/Y | 우스틱 | Axis2D | 카메라 회전 | **[확정]** | UE5 ThirdPerson Template |
| `IA_Sprint` | **Shift** | LB (Xbox) | Boolean | 달리기 hold (스테미나 소모) | **[확정]** (v3) | `Design/SKILL_01_COMBAT_SPEC.md` v3 §5-1 |
| `IA_Jump` | **Space** | A (Xbox) | Boolean | 점프 (스테미나 자유). 공중 `IA_Attack` → 점프 공격 1타 | **[확정]** (v3) | `Design/SKILL_01_COMBAT_SPEC.md` v3 §5-6·§5-7 |
| `IA_Evade` | **Ctrl** | B (Xbox) | Boolean | 회피 (구르기) | **[확정]** (v3) | `Design/SKILL_01_COMBAT_SPEC.md` v3 §5-3 |
| `IA_Attack` | LMB | RB | Boolean | 약공격 (지상 콤보 1·2·3 / 공중 점프 공격 1타) | **[확정]** | `Design/SKILL_01_COMBAT_SPEC.md` §5-2 / v3 §5-7 |
| `IA_Heal` | E | X (Xbox) | Boolean | 회복약 사용 | **[확정]** | `Design/SKILL_01_COMBAT_SPEC.md` §5-4 |
| `IA_LockOn` | MMB (마우스 휠 클릭) | R3 (우스틱 클릭) | Boolean | 락온 토글 | **[확정]** | `Design/SKILL_01_COMBAT_SPEC.md` §5-5 |
| `IA_HeavyAttack` | **RMB** | **RT** | Boolean | 강공격 단발 1타 (적 Down 트리거) | **[확정 예약]** Skill 01B 활성화 시 점유 시작 | `Design/SKILL_01B_UNLOCK_SPEC.md` v2 §5-1 |
| `IA_Parry` | **R** | **LT** | Boolean | 패링 단발 시도 (성공 시 적 Down + Riposte 1.5s) | **[확정 예약]** Skill 01B 활성화 시 점유 시작 (KiHoon §11-Q7 결정 2026-05-30) | `Design/SKILL_01B_UNLOCK_SPEC.md` v2 §4-2·§5-2 |
| `IA_Ultimate` | **G** | 미정 | Boolean | 궁극기(불 마법 프로젝타일) | **[확정 예약]** Stage 2 활성화 시 점유 시작 | Stage 2 후속 명세 (예정) |

**상태 라벨 정의 (§6-2와 일관):**
- **[확정]** — 이미 활성 + 매핑 결정. UE 자산 존재.
- **[확정 예약]** — 매핑 결정, 활성화 시점만 후속 명세에서 토글. 자산은 미생성·게이트 대기 상태.
- **[미정]** — 매핑 자체가 미결 (현재 없음. 모든 예약 IA는 매핑 확정).

> 모든 키 매핑 결정 이력:
> - UE 템플릿 기본 (`IA_Move`/`IA_Look`)
> - SKILL_01 v2 (`IA_Attack`/`IA_Heal`/`IA_LockOn`, KiHoon 위임 → P 확정 2026-05-29 + D 통과)
> - SKILL_01 v3 (`IA_Sprint`/`IA_Jump`/`IA_Evade`, KiHoon 재정의 2026-05-30 — 분리 입력)
> - SKILL_01B v2 (`IA_HeavyAttack`/`IA_Parry`, KiHoon §11-Q7 답변 2026-05-30)
> - Stage 2 (`IA_Ultimate`, KiHoon 재정의 2026-05-30 시점 예약)

---

## 3. ~~통합 입력 패턴 — `IA_EvadeSprint`~~ ⚠ v3에서 폐기 (이력 보존용)

> **상태:** **v3 분리 입력 패턴으로 폐기** (KiHoon 재정의 2026-05-30, SKILL_01 v3 §4). 본 섹션은 v1 명세 이력 보존용으로 남기며, 현재 활성 패턴은 §2 마스터 표(`IA_Sprint`+`IA_Jump`+`IA_Evade` 분리)다.

~~다크소울 클래식 패턴: tap = 회피, hold = 달리기.~~

| ~~항목~~ | ~~위치~~ | ~~비고~~ |
| :--- | :--- | :--- |
| ~~HoldThreshold (분기 임계 시간)~~ | ~~`UGKCombatConfig.SprintHoldThreshold = 0.18s`~~ | ~~Data Asset 노출~~ — v3에서 필드 제거 |
| ~~Release 기반 tap 판정~~ | ~~SKILL_01 §4-2-b~~ | ~~press·release 시간차로 판정~~ — v3에서 폐기 |
| ~~상호배타 게이트~~ | ~~SKILL_01 §4-2-c~~ | ~~한 press 사이클당 1액션~~ — v3에서 불필요 |
| ~~의사 코드~~ | ~~SKILL_01 §4-2-d~~ | ~~구현 참조~~ — v3 명세에서 삭제 |

**현행 패턴:** §2 마스터 표 + SKILL_01 v3 §4 (분리 입력). 본 섹션은 변경 이력 추적 목적으로만 보존.

---

## 4. 입력 우선순위 (전투 컨텍스트, v3 분리 입력 기준)

전투 상태에서 동시 입력 처리 우선순위 — 본 표는 요약본. **상세 13순위 표는 `Design/SKILL_01_COMBAT_SPEC.md` v3 §4-3** (Skill 01B 활성화 후엔 `Design/SKILL_01B_UNLOCK_SPEC.md` v2 §4-3 13순위) 참조.

| 순위 | 입력·상태 | 비고 |
| :---: | :--- | :--- |
| 1 | HitStun / Death | 모든 입력 무시 |
| 2 | Heal 모션 진행 중 | 회피만 캔슬 |
| 3 | JumpAttack 모션 진행 중 | 모든 입력 무시 (v3) |
| 4 | `IA_Evade` (Ctrl) | 회피 진입 |
| 5 | `IA_Attack` (LMB) | 지상 콤보 / 공중 점프 공격 분기 |
| 6 | `IA_Jump` (Space) | 점프 진입 (§4-2-a 게이트 만족 시) |
| 7 | `IA_Sprint` (Shift hold) | Sprint 유지 |
| 8 | `IA_Move` | Run |
| - | `IA_LockOn` (MMB) | 독립 토글 |

> Skill 01B 활성화 후 `IA_HeavyAttack`(RMB)·`IA_Parry`(R)가 우선순위 추가됨 — 상세는 SKILL_01B v2 §4-3 참조.

---

## 5. UE 에셋 위치 (현재 구현 상태)

| 자산 | 경로 | 생성·갱신 주체 |
| :--- | :--- | :--- |
| Input Mapping Context | `/Game/Input/IMC_Default` | UE 템플릿 + B 자동화 스크립트 (v3 입력 동기화 커밋 `1cc6b48`) |
| Input Action (`IA_Move`, `IA_Look`) | `/Game/Input/Actions/` | UE 템플릿 |
| Input Action (`IA_Sprint`, `IA_Jump`, `IA_Evade`, `IA_Attack`, `IA_Heal`, `IA_LockOn`) | `/Game/Input/Actions/` | B 에이전트 자동화 스크립트 (v3 입력 분리 동기화 — git `1cc6b48` / `fe151f3` / `70f4fe7` 참조) |
| Input Action (`IA_HeavyAttack`, `IA_Parry`, `IA_Ultimate`) | (미생성) | **[확정 예약]** — 후속 활성화 사이클 (Skill 01B B 자산 신설 / Stage 2 명세) |
| ~~Input Action (`IA_EvadeSprint`)~~ | ~~`/Game/Input/Actions/`~~ | ~~v1 자동화 스크립트~~ — **v3에서 폐기. 자산 유지 시 `Scripts/SetupSkill01CombatAssets.py`의 deprecation 처리 검토 (B 영역)** |

스크립트 실행 명령: `UnrealEditor-Cmd.exe GK.uproject -ExecutePythonScript=Scripts/SetupSkill01CombatAssets.py`

---

## 6. 충돌 점검 체크리스트

신규 키 추가 시 반드시 확인:

### 6-1. 키 중복 금지
같은 키에 두 IA 매핑 금지. 모디파이어(`Shift`, `Ctrl`)로 분기하더라도 단일 키 단독 매핑은 1IA만 사용.

### 6-2. 마우스 버튼 분배 (3슬롯)
| 슬롯 | 현재 할당 | 상태 |
| :--- | :--- | :--- |
| LMB | `IA_Attack` (약공격) | **[확정]** |
| RMB | `IA_HeavyAttack` (강공격, Skill 01B에서 활성화) | **[확정 예약]** — RMB / RT. Skill 01B 게이트 통과 후 활성 |
| MMB | `IA_LockOn` | **[확정]** |

> 라벨 정의 (`Design/SKILL_01B_UNLOCK_SPEC.md` §4와 일관):
> - **[확정]** — 이미 활성 + 매핑 결정
> - **[확정 예약]** — 매핑 결정, 활성화 시점만 후속 명세에서 토글
> - **[미정]** — 매핑 자체가 미결 (Q 후보 답변 대기)

### 6-3. 키보드 핸드 포지션 (v3 분리 입력 반영)
- **왼손 (WASD 주변):** WASD, Space(`IA_Jump`), Shift(`IA_Sprint`), Ctrl(`IA_Evade`), E(`IA_Heal`), R(`IA_Parry` [확정 예약]) — 분리 입력 패턴으로 키 점유 확대
- **오른손:** 마우스 + 휠/버튼 (LMB·RMB·MMB 3슬롯 점유 — §6-2)
- **추후 추가 검토 영역:** Q, F, T, 1~4 — 비어 있음 (Shift·Ctrl·R은 v3·Skill 01B에서 점유 시작)
- **금지 영역:** WASD 자체에 추가 매핑 금지 (이동 우선)

### 6-4. UE 표준 예약 키 (충돌 주의)
- `ESC` — UE 기본 일시정지/메뉴 (추후 UI에서 처리)
- `Tab` — 추후 UI(인벤토리·메뉴) 예약
- `F1`~`F12` — 디버그/개발자 도구 예약

---

## 7. 후속 산출물 예고

> **SSOT 일원화 (2026-05-30):** `IA_HeavyAttack`·`IA_Parry`·`IA_Ultimate` 3건은 §2 통합 마스터 표로 이전됨. 본 표는 §2에 아직 포함되지 않은 후속 IA만 추적한다.

| 예정 IA | 트리거 시점 | 매핑 상태 (§6-2 라벨 적용) |
| :--- | :--- | :--- |
| `IA_Menu` | UI 명세 | **[확정 예약]** ESC |
| `IA_DebugToggle` | 디버그 도구 | **[확정 예약]** ` (그레이브) |
| (후속 추가 시) | (해당 명세 도래 시) | 신규 IA는 §2 마스터 표에 바로 등록 |

---

## 8. 변경 이력

| 일자 | 변경 | 작성자 |
| :--- | :--- | :--- |
| 2026-05-30 | 신설 — SKILL_01 v2 §4 / Pillars §8 / B 자동화 스크립트의 키 매핑을 단일 출처로 통합 | P |
| 2026-05-30 | surgical 보정 (D 조건부 Go 해소) — §6-2 RMB 슬롯 + §7 후속 IA 표를 **[확정] / [확정 예약] / [미정]** 3종 라벨로 일원화. `IA_HeavyAttack` [확정 예약], `IA_Parry` [미정] (SKILL_01B §11-Q7 답변 대기), `IA_Ultimate` [확정 예약]. 기능·매핑 키 변경 없음 (RMB·G는 기존 예약 그대로) | P |
| 2026-05-30 | KiHoon SKILL_01B §11-Q7 답변 반영 — `IA_Parry` **[미정] → [확정 예약] R / LT**. §6-3 추후 검토 영역에서 R 제거 + `IA_Parry` 점유 표시, §7 후속 IA 표 IA_Parry 행 갱신. 마스터 표(§2) 등록은 Skill 01B B 자산 신설 사이클에서 진행 (P 작업 영역) | P |
| 2026-05-30 | **D 조건부 Go 해소 — SSOT 통합 갱신.** §1 패턴 텍스트 "다크소울 클래식" → "분리 입력" 갱신. **§2 마스터 표 통합 SSOT로 전환** — v3 분리 입력(`IA_Sprint`·`IA_Jump`·`IA_Evade`) 반영 + `IA_HeavyAttack`/`IA_Parry`/`IA_Ultimate` [확정 예약] 통합. §3 v1 통합 입력 패턴 폐기 노티 + strikethrough. §4 우선순위 표 v3 기준 갱신. §5 UE 에셋 위치 v3 IA + Skill 01B 예약 IA 반영. §6-3 키보드 핸드 포지션 v3 점유 갱신. §7 후속 IA 표에서 §2 이전 3건 제거. 키 매핑 자체는 변경 없음(이력 정리·SSOT 통합만) | P |
