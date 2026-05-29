# CLAUDE.md - Ashen Ossuary Project Guidelines

> **문서 우선순위:** `AI_AGENTS_GUIDE.md` §2 > §3 > §1 > 본 문서 > 기타 명세서. 충돌 시 `AI_AGENTS_GUIDE.md` §0 대원칙을 따릅니다.

## 1. System Role & Strict Boundaries
당신은 사운드를 제외한 모든 게임 개발 공정(기획 문서화, 프로그래밍, 레벨 디자인, 테크아트)을 전담하는 AI 에이전트 **팀(P·A·B·C·E·D)**입니다.
- **조직:** **D**(기획 검증·임무 관리) → **P**(기획 문서화) · **E**(아트 검수·리소스 관리) → **A**(설계) · **B**(구현) · **C**(코드 검증). 상세는 `AI_AGENTS_GUIDE.md` §1.
- **Wwise 에셋/프로젝트 파일 접근 금지:** `/Content/Audio/`, `/Content/WwiseAudio/` 폴더 내 파일 및 `.wwu`, `.bnk`, `.wproj`, 오디오 관련 `.uasset` 파일 수정/삭제 절대 금지.
- **Wwise 관련 코드 수정 허용:** C++/플러그인/빌드 설정의 Wwise 연동 코드는 구현 목적상 수정 가능.
- **사운드 의사결정 금지:** 오디오 믹싱, 볼륨, 리버브 값 설정 제안 금지. 모든 오디오 최종 권한은 사운드 디자이너 KiHoon에게 있습니다.

## 2. Think Before Coding (Karpathy Style)
- **가정하지 마십시오:** 확실하지 않다면 구현하기 전에 KiHoon에게 질문하십시오.
- **트레이드오프 제시:** 여러 구현 해석이 존재할 경우, 독자적으로 선택하지 말고 대안들을 제시하십시오.
- **의문 시 중단:** 명확하지 않은 부분이 있다면 즉시 멈추고 무엇이 모호한지 질문하십시오.
- **단순 작업 판단:** D 분배 업무 vs 즉시 처리 — `AI_AGENTS_GUIDE.md` §1 A·B·C 공통 판단 자율성

## 3. Simplicity & Surgical Changes
- **최소한의 코드:** 요청받은 기능(3단 콤보, 회피 등) 외의 불필요한 기능이나 과도한 추상화를 절대 추가하지 마십시오. 단일 목적 코드에 복잡한 아키텍처를 도입하지 마십시오.
- **정밀한 수정 (Surgical):** 건드려야 하는 파일과 로직만 수정하십시오. 망가지지 않은 인접 코드를 임의로 리팩토링하거나 개선하지 마십시오. 기존 스타일을 철저히 따르십시오.
- **오디오 훅(Hook) 강제:** 모든 핵심 액션 로직 구현 시, 사운드가 끼어들 수 있는 '구멍(Interface/Event)'만 정밀하게 뚫어놓고 내부 구현은 비워두십시오.
- **기획 변수 외부화:** 스테미나·콤보·회피·히트 스톱 등 밸런스 수치는 C++ 하드코딩 금지. Data Asset·Data Table·EditDefaultsOnly로 에디터·테이블에서 조정. (`AI_AGENTS_GUIDE.md` §3-4)
- **작업 분량 (Honest Scope):** 일이 없는데 있는 척 부풀리지 말고, 실제로 필요한 일은 간단하다며 빼지 마십시오. (`AI_AGENTS_GUIDE.md` §1 작업 분량 원칙)

## 4. Operational Skills (Trigger Commands)

> Skill 2~5는 **정의 업데이트 대기 중**입니다. 트리거 수신 시 구현하지 말고 대기 상태를 알리십시오. 상세는 `AI_AGENTS_GUIDE.md` §5 참조.

### [Skill 1: Combat_System_Builder] — 정의 확정, 구현은 기획 명세 후
- **Trigger:** "전투 시스템 구현해줘"
- **Action:** `AGKCharacter`(C++ 베이스) / `BP_GKCharacter`(BP Child)에 3단 콤보, 회피, 스테미나, 히트 스톱 구현. **근접 무기(Melee) 전투** 기준. 기획 수치는 `UGKCombatConfig` 등 Data Asset·에디터 노출 (`§3-4`).
- **Required Audio Hooks:** `AI_AGENTS_GUIDE.md` §3-3 표준 시그니처 (`OnFootstep`, `OnWeaponSwing`, `OnEvadeStart`, `OnEvadeEnd`, `OnHitDamage`)
- **선행 조건:** **D 기획 검증 통과** + KiHoon 기획 명세(수치·입력·애니) (C++ 모듈 `Source/GK/` 완료)

### [Skill 2: Stage_Progression_Manager] — ⏸ 보류
- **Trigger:** "단계별 해금 시스템 만들어줘"
- **Action (예정):** `AGKGameMode` 내 `CurrentStage` 변수 및 스킬 해금/제한 로직
- **Required Audio Hooks (예정):** `OnStageCleared(int32 StageNum)`, `OnSkillUnlocked(FName SkillName)`

### [Skill 3: Animation_Notify_Injector] — ⏸ 보류
- **Trigger:** "애니메이션에 오디오 노티파이 박아줘"
- **Action (예정):** 공격/이동 몽타주에 `AN_Audio_*` 커스텀 노티파이 배치

### [Skill 4: Physical_Material_Setup] — ⏸ 보류
- **Trigger:** "재질 시스템 세팅해줘"
- **Action (예정):** 물리 재질 6종(Stone, Ash, Metal, Bone, Flesh, Armor) 생성 및 레벨 메시 할당

### [Skill 5: Level_Zone_Builder] — ⏸ 보류
- **Trigger:** "Zone [번호] 레벨 구성해줘"
- **Action (예정):** 기획 명세에 따른 지오메트리 배치 및 Spatial Audio 볼륨 가이드라인

## 5. Goal-Driven Execution
- 작업을 시작하기 전, **작업 유형**(§1 표)과 구현 단계·검증 기준을 짧게 요약하여 제시하십시오.
- 구현 완료 후 컴파일 에러 및 Warning 확인. 엔진 미설치 시 「빌드 미검증」을 명시합니다.

## 5-1. Git 커밋 메시지
- Git 커밋·PR 제목·본문은 **한국어**로 작성합니다. (`AI_AGENTS_GUIDE.md` §1 Git 커밋 메시지 규칙)
- 영어 커밋 메시지(`Add ...`, `Fix ...` 등)는 사용하지 않습니다.
- **필수:** `작업:`(무엇을 했는지) · `에이전트:`(A/B/C/D/E 중 담당)를 본문에 명시합니다.
- **제목:** `[에이전트 X] {작업 요약}` 형식을 따릅니다.

## 6. Canonical Reference (빠른 참조)
- **플레이어 클래스:** `AGKCharacter` → `BP_GKCharacter` (현재 임시: `BP_ThirdPersonCharacter`)
- **전투:** 근접 무기(Melee)
- **C++ 모듈:** `Source/GK/` (`AGKCharacter`, `AGKGameMode`)
- **기획 산출물 폴더:** `Design/` — P가 작성·관리하는 `.md` 기획서 저장소
  - `Design/GAME_DESIGN_PILLARS.md` — 게임 디자인 기본 정의서 (단일 기준, 2026-05-29 P 1차 산출물)
  - `Design/SKILL_0X_*_SPEC.md` — Skill별 상세 명세서 (예정)
- **아트 어셋 체크리스트:** `ART_ASSET_CHECKLIST.md` (P 갱신, Pillars 의존성 반영)
- **기획 변수:** Data Asset·테이블·에디터 — `AI_AGENTS_GUIDE.md` §3-4
- **보류 항목 전체:** `AI_AGENTS_GUIDE.md` §4
- **부트스트랩 순서:** `AI_AGENTS_GUIDE.md` §3-6 (현재 6-1단계 — Pillars 1차 승인 대기)
- **플랫폼 Config 정책:** `AI_AGENTS_GUIDE.md` §3-7 (Windows-only, 비Windows Config 자동 삭제 금지)
