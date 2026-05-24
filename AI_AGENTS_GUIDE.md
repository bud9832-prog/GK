# Ashen Ossuary - AI 에이전트 역할 분담 및 실행 지침서

본 문서는 사운드 디자이너 KiHoon(사용자)과 협업하는 모든 AI 에이전트(Claude, Cursor Composer, o1/o3-mini)의 독립적인 역할과 실행 명령어(Trigger)를 명시합니다. **모든 에이전트는 본 지침과 `CLAUDE.md` 규칙을 반드시 상시 준수해야 하며, 문서 간 충돌 시 본 문서의 [§3 아키텍처 확정 사항]을 최우선으로 따릅니다.**

---

## 1. 에이전트별 독립 역할 정의 (Role Assignment)

### 👤 에이전트 A: 시스템 아키텍트 (Model: Claude 3.5 / 4.6 Sonnet)
- **주요 임무:** 전체적인 C++ 클래스 구조 설계, 상속 관계 정의, 인터페이스 기획.
- **행동 지침:** 코드를 직접 파일에 박기 전에 기훈님에게 설계안(헤더 구조 및 트레이드오프)을 먼저 텍스트로 브리핑하고 승인을 받으십시오. 절대 독단적으로 코딩을 시작하지 마십시오.

### 🤖 에이전트 B: 정밀 수술식 구현가 (Model: Cursor Composer - Agent Mode)
- **주요 임무:** 아키텍트가 설계하고 **기훈님이 승인한** 내용을 바탕으로 실제 `.h` 및 `.cpp` 소스 코드를 생성 및 수정.
- **행동 지침:** 안드레아 카파시 스타일을 준수하여 오직 요청받은 기능만 정밀하게(Surgical) 작성하십시오. 멀쩡한 인접 코드를 건드리거나 불필요한 고도화(오버엔지니어링)를 하지 마십시오.

### ⚖️ 에이전트 C: 코드 검증 및 판사 (Model: o1 / o3-mini)
- **주요 임무:** 코드 완성 후 컴파일 에러 트러블슈팅, Windows 환경 오디오 스레드 안정성 및 메모리 누수 검증.
- **행동 지침:** 비판적이고 날카로운 시선으로 코드를 리뷰하고, 런타임 크래시나 성능 병목을 유발할 여지가 있는 부분을 찾아내어 교정안을 제시하십시오.

### 공통 워크플로 (모든 에이전트·모든 작업 규모)
**단순한 작업이라도 예외 없이** 아래 순서를 따릅니다.
1. **브리핑:** 구현 대상, 접근 방식, 트레이드오프를 텍스트로 제시
2. **승인:** 기훈님 확인 후에만 코딩 시작
3. **구현:** 승인된 범위만 Surgical하게 작성
4. **검증:** 컴파일 에러 및 Warning 확인
5. **커밋:** 기훈님 요청 시 Git 커밋·푸시 수행 (메시지는 **한국어**로 작성)

### Git 커밋 메시지 규칙
- **언어:** 제목·본문 모두 **한국어**로 작성합니다. 영어 커밋 메시지는 사용하지 않습니다.
- **필수 명시 (2항목):** 모든 커밋 메시지에 아래를 반드시 포함합니다.
  1. **작업 내용:** 무엇을 했는지 (변경 파일·기능·목적)
  2. **담당 에이전트:** 누가 수행했는지 (아래 표기 중 하나)
- **에이전트 표기:**
  | 코드 | 역할 | 모델 |
  | :--- | :--- | :--- |
  | **A** | 시스템 아키텍트 | Claude 3.5 / 4.6 Sonnet |
  | **B** | 정밀 수술식 구현가 | Cursor Composer (Agent Mode) |
  | **C** | 코드 검증 및 판사 | o1 / o3-mini |
- **제목 형식:** `[에이전트 X] {작업 요약}` — 50자 내외
- **본문 형식:** 제목 아래 빈 줄 후 `작업:` · `에이전트:` 항목을 각각 1줄 이상 서술
- **예시:**
  ```
  [에이전트 B] GK C++ 모듈 스켈레톤 추가

  작업: Cleanup 준비를 위해 AGKCharacter·AGKGameMode 베이스 클래스, 오디오 훅 선언, GK.uproject 모듈 등록
  에이전트: B (정밀 수술식 구현가 / Cursor Composer)
  ```

### 작업 분량 원칙 (Honest Scope)
- **부풀리지 마십시오:** 실제 변경·리스크에 비해 브리핑, 파일 수정, 문서, 커밋, 후속 제안을 과장하지 않습니다. 할 일이 없으면 **「할 일 없음」**이라고 말합니다.
- **줄이지 마십시오:** 컴파일 깨짐, 참조 끊김, 실제 버그·누락이 있으면 「간단하다」는 이유로 생략·미루지 않습니다.
- **기준:** diff와 리스크가 말하는 분량 = 보고·구현·검증하는 분량.

---

## 2. 에이전트 공통 절대 금지 사항 (Strict Prohibitions)
1. **오디오 에셋 및 Wwise 파일 접근 절대 금지:** `/Content/Audio/`, `/Content/WwiseAudio/` 폴더 내 파일 및 `.wwu`, `.bnk`, 오디오 관련 `.uasset` 파일의 수정/삭제를 절대 금지합니다.
2. **사운드 독자 결정 금지:** 사운드 재생 코드(`.cpp` 내 직접 하드코딩), 볼륨, 리버브, 사운드 믹스 방식에 대해 AI가 임의로 코드를 작성하지 마십시오. 사운드와 관련된 모든 최종 권한은 KiHoon에게 있습니다.

---

## 3. 프로젝트 아키텍처 확정 사항 (Canonical Decisions)

문서 간 불일치가 있었던 항목을 아래 기준으로 통일합니다.

### 3-1. 클래스 명명 및 상속 구조
| 구분 | 확정 명칭 | 비고 |
| :--- | :--- | :--- |
| C++ 플레이어 베이스 | `AGKCharacter` | KiHoon이 C++ 모듈 생성 후 확정·안내 예정. 생성 전까지 임시 명칭. |
| 블루프린트 플레이어 | `BP_GKCharacter` | C++ 베이스 `AGKCharacter`의 Blueprint Child |
| C++ 게임모드 베이스 | `AGKGameMode` | Stage 해금 등 GameMode 로직의 C++ 베이스 (모듈 생성 후) |
| **현재 임시 에셋** | `BP_ThirdPersonCharacter` | UE Third Person BP 템플릿. C++ 모듈 도입 전까지 유지 |

- 게임플레이 코어 로직은 **C++ 베이스 클래스**에 두고, KiHoon의 Wwise 매핑은 **Blueprint Child**에서 `BlueprintImplementableEvent` 오디오 훅으로 연결합니다.
- ~~`AMyCharacter`~~, ~~`BP_PlayerCharacter`~~ 등 구 명칭은 **사용하지 않습니다**.

### 3-2. 전투 방식
- **근접 무기(Melee) 전투**입니다. 캐릭터는 무기를 장착하며, 원거리(Rifle/Pistol) 전투는 본 프로젝트 범위가 아닙니다.
- 무기·애니메이션·VFX 아트 에셋은 KiHoon이 준비 중입니다. 에셋 경로 및 몽타주 명세는 **기획 명세 수령 후** 확정합니다.

### 3-3. 필수 오디오 훅 시그니처 (표준)
모든 문서·코드에서 아래 시그니처를 **단일 표준**으로 사용합니다.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Movement")
void OnFootstep(EPhysicalSurface SurfaceType);

UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
void OnWeaponSwing(int32 ComboIndex);

UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
void OnEvadeStart();

UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
void OnEvadeEnd();

UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
void OnHitDamage(FVector HitLocation, AActor* Attacker);
```

- `OnHitDamage`의 `Attacker` 인자는 **필수**입니다. (구버전 `FVector` 단독 시그니처 폐기)
- `OnFootstep`의 Surface 감지는 C++에서 Line Trace 처리, Wwise Switch 매핑은 Blueprint(KiHoon)에서 처리합니다.

### 3-4. 현재 프로젝트 상태 (2025-05)
- **엔진:** UE 5.7
- **C++ `Source/` 모듈:** `Source/GK/` — `AGKCharacter`, `AGKGameMode` 스켈레톤. 에디터에서 `.sln` 생성 후 빌드 필요.
- **레벨:** `Lvl_ThirdPerson` 1개 (Zone 1~3 분리 레벨은 추후 지시)
- **입력:** Enhanced Input (`Content/Input/IMC_Default`)

---

## 4. 보류 항목 — KiHoon 지시 대기 (Do Not Implement Yet)

아래 항목은 AI 에이전트가 **독자적으로 구현·수치 결정하지 않습니다.** 기획 명세 또는 명시적 지시가 올 때까지 대기합니다.

| 항목 | 상태 |
| :--- | :--- |
| C++ 게임 모듈(`Source/GK/`) 생성 | **완료** — `AGKCharacter`, `AGKGameMode` 스켈레톤 |
| Skill 2~5 상세 정의 및 트리거 명세 | 문서 추후 업데이트 예정 |
| 콤보·회피·스테미나·히트스톱 수치 및 입력 바인딩 | 기획 명세 수령 후 |
| 무기/회피/공격 애니메이션 몽타주 경로 | 아트 에셋 준비 중 |
| 피지컬 머티리얼 6종 생성 및 Surface Type 매핑 | Skill 4 또는 별도 지시 후 |
| Zone 1~3 레벨 구성 및 Spatial Audio 볼륨 배치 | 세부 구현 지시 후 |
| 1단계 레거시 Cleanup 범위 (삭제 대상 목록) | 아트·기획 확정 후 별도 지시 |

---

## 5. 명령어 기반 실행 스킬셋 (Operational Commands)

> **주의:** Skill 2~5는 정의가 미완성입니다. 기훈님이 문서를 업데이트하기 전까지 해당 트리거를 받아도 **구현에 착수하지 말고**, "스킬 정의 업데이트 대기 중"임을 알리십시오.

기훈님이 채팅창에 아래 [트리거 문장]을 입력하면, **승인된 설계**를 바탕으로 최소한의 C++ 코드로 기능을 구현하고 **필수 오디오 훅(UFUNCTION)**을 반드시 배치하십시오.

### ⚔️ [Skill 1: Combat_System_Builder] — 정의 확정, 구현은 기획 명세 후
- **트리거 문장:** `전투 시스템 구현해줘`
- **구현 대상:** `AGKCharacter` (C++ 베이스) / `BP_GKCharacter` (Blueprint Child)
- **구현 내용:** 소울라이크 스타일 3단 공격 콤보, 스테미나 소모/회복, 히트 스톱(Hit Stop), 회피(구르기)
- **필수 오디오 훅:** §3-3 표준 시그니처 전부 (`OnFootstep` 포함)
- **선행 조건:** C++ 모듈 생성 완료 + KiHoon 기획 명세(수치·입력·애니 경로) 수령

### 🔒 [Skill 2~5] — 정의 보류
| Skill | 트리거 (예정) | 상태 |
| :--- | :--- | :--- |
| Stage_Progression_Manager | `단계별 해금 시스템 만들어줘` | 정의 업데이트 대기 |
| Animation_Notify_Injector | `애니메이션에 오디오 노티파이 박아줘` | 정의 업데이트 대기 |
| Physical_Material_Setup | `재질 시스템 세팅해줘` | 정의 업데이트 대기 |
| Level_Zone_Builder | `Zone [번호] 레벨 구성해줘` | 정의 업데이트 대기 |

Skill 2~5의 상세 Action·오디오 훅 목록은 `CLAUDE.md` §4에 임시 참조용으로 남겨두되, **본 문서 업데이트 전까지 구현 착수 금지**입니다.
