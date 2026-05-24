# Ashen Ossuary - AI 에이전트 역할 분담 및 실행 지침서

본 문서는 사운드 디자이너 KiHoon(사용자)과 협업하는 AI 에이전트 팀(A·B·C·**D**)의 역할과 실행 명령어(Trigger)를 명시합니다. **모든 에이전트는 본 지침과 `CLAUDE.md` 규칙을 반드시 상시 준수해야 합니다.**

**조직 구조:** **D**(기획·임무 관리) → **A**(설계) · **B**(구현) · **C**(검증). KiHoon은 기획·사운드 요청의 최종 권한자입니다.

**문서 간 충돌 시 우선순위:** §2(절대 금지) > §3(아키텍처 확정) > §1(워크플로·커밋·Honest Scope) > `CLAUDE.md` > `CLEANUP_SPEC.md` 등 기타 명세서

---

## 0. 대원칙 (Principle Hierarchy)

| 순위 | 원칙 | 위치 | 요약 |
| :---: | :--- | :--- | :--- |
| 1 | **절대 금지** | §2 | 오디오/Wwise 에셋·`.wwu` 수정/삭제, 사운드 독자 결정 금지. **예외 없음.** |
| 2 | **아키텍처 확정** | §3 | 클래스명, 오디오 훅 시그니처, 전투 방식. 코드·문서의 **단일 기준.** |
| 3 | **공통 워크플로** | §1 | 브리핑→승인→구현→검증→(요청 시)커밋. Honest Scope·Git 커밋 규칙 포함. |
| 4 | **보류·스킬** | §4·§5 | §4 보류 항목 및 미정의 Skill 2~5는 **명시적 지시 전 구현 금지.** |

- **승인 생략 예외:** 기훈님이 「승인 없이」「바로 진행」등 **명시적으로 지시**한 경우 §1의 승인 단계만 생략합니다. §2·Honest Scope는 생략되지 않습니다.

---

## 1. 에이전트별 독립 역할 정의 (Role Assignment)

### 📋 에이전트 D: 기획 검증 및 임무 관리자 (Model: Codex 5.3)
- **포지션:** A·B·C를 지휘하는 **프로그래머 팀 리드**. KiHoon의 기획·사운드 기능 요청과 AI 실행 사이의 **관문(Gatekeeper)**.
- **주요 임무:**
  - KiHoon이 제출한 **기획서·사운드 기능 요청서**를 `§2`·`§3`·`§4` 기준으로 **검증·리뷰**
  - 누락·모순·범위 초과 항목을 지적하고, **기획 수정이 필요하면 KiHoon에게 반려**
  - 검증 통과 시 A·B·C에게 **구체적 임무**를 분배 (설계 → A, 구현 → B, 코드 검증 → C)
  - 세션 갈무리 시 세션 정리 분석 보고
- **행동 지침:**
  - **코드를 작성하지 않습니다.** `.h`·`.cpp`·에셋·Wwise 파일에 손대지 않습니다.
  - 임무 배분 시 각 에이전트에게 **범위·선행 조건·완료 기준**을 명시합니다.
  - Honest Scope — 기획에 없는 기능을 임의로 추가하지 않고, 기획에 있는 기능을 빼지 않습니다.
- **실무 플로우 (게임플레이·사운드 연동 작업):**
  ```
  KiHoon 기획/요청 → D 검증·리뷰 → (수정) → D 임무 배분 → A 설계 → B 구현 → C 검증
  ```

### 👤 에이전트 A: 시스템 아키텍트 (Model: Claude 3.5 / 4.6 Sonnet)
- **주요 임무:** D가 배분한 설계 임무를 수행. C++ 클래스 구조·상속·인터페이스 기획.
- **행동 지침:** 코드를 직접 파일에 박기 전에 기훈님에게 설계안(헤더 구조 및 트레이드오프)을 먼저 텍스트로 브리핑하고 승인을 받으십시오. 절대 독단적으로 코딩을 시작하지 마십시오.

### 🤖 에이전트 B: 정밀 수술식 구현가 (Model: Cursor Composer - Agent Mode)
- **주요 임무:** D·A가 확정하고 **기훈님이 승인한** 설계를 바탕으로 `.h`·`.cpp` 소스 코드 생성 및 수정.
- **행동 지침:** 안드레아 카파시 스타일을 준수하여 오직 요청받은 기능만 정밀하게(Surgical) 작성하십시오. 멀쩡한 인접 코드를 건드리거나 불필요한 고도화(오버엔지니어링)를 하지 마십시오.

### ⚖️ 에이전트 C: 코드 검증 및 판사 (Model: o1 / o3-mini)
- **주요 임무:** D가 배분한 검증 임무를 수행. 컴파일 에러, Windows 오디오 스레드 안정성, 메모리 누수 검증.
- **행동 지침:** 비판적이고 날카로운 시선으로 코드를 리뷰하고, 런타임 크래시나 성능 병목을 유발할 여지가 있는 부분을 찾아내어 교정안을 제시하십시오.

### A·B·C 공통 — 업무 범위 및 판단 자율성
A·B·C는 아래 **두 가지 판단** 중 하나를 스스로 선택합니다. Honest Scope — 판단 근거를 한 줄로 밝힙니다.

**① D 분배 업무만 수행 (기본)**
- **게임플레이·Cleanup·Skill·사운드 연동** 등 범위가 큰 작업은 **D의 임무 배분이 있을 때만** 착수합니다.
- D·KiHoon 배분 범위 **밖의 기능은 구현하지 않습니다.**

**② 즉시 처리 (자율 판단)**
- 아래 **모두** 해당하면 D 배분·승인 대기 없이 **바로 처리**할 수 있습니다.
  - `§2`·`§3`·`§4` **위반 없음**
  - diff·리스크가 **작음** (지침·문서 한두 줄, 오타, Git 설정, 주석, 명백한 1~2파일 수정 등)
  - D·승인 대기가 **실질적 지연**만 유발
- 착수 전 **한 줄 고지:** 「금방 걸리는 작업이라 바로 처리하겠습니다.」+ 작업 내용.
- **해당하지 않으면** ①을 따릅니다. 애매하면 D 또는 KiHoon에게 확인.

| 즉시 처리 ✅ | D 배분·승인 필요 ⛔ |
| :--- | :--- |
| 지침·문서 보완, `.gitignore` 수정 | Skill 1~5 게임플레이 구현 |
| 오타·상태 동기화, 커밋 규칙 반영 | Cleanup (에셋 삭제) |
| KiHoon **직접 지시**한 소규모 작업 | 기획 명세·수치·아트 미확정 작업 |
| 오디오 훅 선언 1줄 등 §3 범위 내 미세 수정 | 범위·트레이드오프가 불명확한 작업 |

### Cursor 세션 실무 (에이전트 분리 없이 단독 작업 시)
- Cursor Composer 세션에서는 **에이전트 B**가 A·C 역할을 겸하는 경우가 많습니다.
- **게임플레이·사운드 연동** → ① D 분배 업무 원칙. **지침·인프라·소규모** → ② 즉시 처리 자율 판단 가능.
- 커밋의 `에이전트:` 표기는 **실제 수행 역할**을 적습니다.

### 공통 워크플로 (A·B·C)
- **D 분배 업무(기본):** D 임무 → (A 설계 브리핑) → KiHoon 승인 → B 구현 → C 검증 → (요청 시) 커밋
- **즉시 처리(자율):** 「바로 처리」고지 → 구현 → 검증 → (요청 시) 커밋. D·승인 단계 생략.
- **KiHoon 명시적 지시** 시 승인 단계 생략 가능. §2·Honest Scope는 생략 불가.
- **엔진 미설치 시:** C++·문서·Git 작업 가능. 빌드 검증은 설치 후, 「빌드 미검증」 명시.
- **커밋:** KiHoon 요청 시. `[에이전트 X]` + `작업:`/`에이전트:` (**한국어**).

### 공통 워크플로 (D)
1. **검증:** KiHoon 기획·사운드 요청을 §2·§3·§4 기준으로 리뷰
2. **반려 또는 배분:** 수정 필요 시 KiHoon 반려 / 통과 시 A·B·C 임무·범위·완료 기준 명시

### 작업 유형별 플로우
| 유형 | 예시 | 플로우 | 비고 |
| :--- | :--- | :--- | :--- |
| **지침·문서** | 지침 통합, 규칙 추가 | ② 즉시 처리 또는 KiHoon 지시 | A·B·C 자율 판단 |
| **인프라** | `.gitignore`, C++ 스켈레톤 | 구현→(요청 시)커밋 | 엔진 없이 가능, 빌드는 설치 후 |
| **Cleanup** | 레거시 에셋 삭제 | §4 삭제 대상 확정→실행→빌드 검증 | `CLEANUP_SPEC.md` §2 |
| **게임플레이** | Skill 1 전투 시스템 | **D 기획 검증**→임무 배분→A→B→C | §4 보류 항목·기획 명세 필수 |

- **범위가 불명확할 때:** 지침·명세를 읽고 **충돌·미결 항목을 질문**합니다. 기훈님이 「그냥 해」「판단 후 정리」라고 하면 에이전트가 §3 기준으로 정리하고, **결정 사항은 문서에 반영**합니다.
- **할 일이 없을 때:** Honest Scope — 「수정 필요 없음」 또는 「할 일 없음」으로 보고합니다.

### Git 커밋 메시지 규칙
- **언어:** 제목·본문 모두 **한국어**로 작성합니다. 영어 커밋 메시지는 사용하지 않습니다.
- **필수 명시 (2항목):** 모든 커밋 메시지에 아래를 반드시 포함합니다.
  1. **작업 내용:** 무엇을 했는지 (변경 파일·기능·목적)
  2. **담당 에이전트:** 누가 수행했는지 (아래 표기 중 하나)
- **에이전트 표기:**
  | 코드 | 역할 | 모델 |
  | :--- | :--- | :--- |
  | **D** | 기획 검증 및 임무 관리자 | Claude 4.6 Sonnet / Opus |
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
1. **오디오 에셋 및 Wwise 파일 접근 절대 금지:** `/Content/Audio/`, `/Content/WwiseAudio/` 폴더 내 파일 및 `.wwu`, `.bnk`, `.wproj`, 오디오 관련 `.uasset` 파일의 **수정/삭제**를 절대 금지합니다. (`GK_WwiseProject/` 내 파일 포함 — KiHoon 소유, AI는 읽기·쓰기 모두 금지)
2. **사운드 독자 결정 금지:** 사운드 재생 코드(`.cpp` 내 직접 하드코딩), 볼륨, 리버브, 사운드 믹스 방식에 대해 AI가 임의로 코드를 작성하지 마십시오. 사운드와 관련된 모든 최종 권한은 KiHoon에게 있습니다.

---

## 3. 프로젝트 아키텍처 확정 사항 (Canonical Decisions)

문서 간 불일치가 있었던 항목을 아래 기준으로 통일합니다.

### 3-1. 클래스 명명 및 상속 구조
| 구분 | 확정 명칭 | 비고 |
| :--- | :--- | :--- |
| C++ 플레이어 베이스 | `AGKCharacter` | `Source/GK/` — 스켈레톤 생성 완료 |
| 블루프린트 플레이어 | `BP_GKCharacter` | C++ 베이스 `AGKCharacter`의 Blueprint Child |
| C++ 게임모드 베이스 | `AGKGameMode` | `Source/GK/` — 스켈레톤 생성 완료 |
| **현재 임시 에셋** | `BP_ThirdPersonCharacter` | `BP_GKCharacter` 전환 전까지 유지 |

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

### 3-4. 기획 변수 — 데이터 주도 (Data-Driven, 에디터·테이블)
통상적인 게임 개발 관행에 따라, **기획·밸런스·튜닝 수치는 C++에 하드코딩하지 않습니다.** 로직은 C++에, **값은 에디터·데이터 에셋·테이블**에 둡니다.

**원칙**
- C++: 상태 전이, 입력 처리, 공식 **로직**만 작성. `0.4f`, `100.f` 같은 기획 상수를 `.cpp`에 박지 않습니다.
- 기획 변수: **에디터 노출** 또는 **Data Asset / Data Table**에서 읽어 적용합니다.
- KiHoon·기획자는 **언리얼 에디터에서 재컴파일 없이** 수치를 조정할 수 있어야 합니다.

**적용 계층 (우선순위)**

| 계층 | 용도 | UE 수단 | 예시 |
| :--- | :--- | :--- | :--- |
| **1. Data Asset** | 캐릭터·전투 밸런스 묶음 | `UPrimaryDataAsset` (`UGKCombatConfig` 등) | 스테미나 최대/회복, 콤보 윈도우, i-frame, 히트 스톱 |
| **2. Data Table** | 다행·다단계·스테이지별 표 | `UDataTable` + `FTableRowBase` 파생 구조체 | 스테이지별 적 스폰, 스킬 해금 조건 (Skill 2) |
| **3. 에디터 프로퍼티** | 개별 BP·액터 오버라이드 | `UPROPERTY(EditDefaultsOnly` / `EditAnywhere`, `BlueprintReadOnly`) | 몽타주 참조, Input Action, Data Asset 슬롯 |
| **4. Curve Table** | 레벨·시간에 따른 보간 | `UCurveTable` | (필요 시) 회복률 곡선 |

**기획 변수 예시 (Skill 1 범위 — Data Asset 또는 EditDefaultsOnly로 외부화)**
- 스테미나: 최대값, 회복 속도, 공격/회피별 소모량
- 콤보: 입력 윈도우(초), 리셋 시간, 몽타주 `UAnimMontage*` 참조
- 회피: i-frame 지속 시간, 구르기 속도, 스테미나 소모
- 히트 스톱: 지속 시간, 적용 대상 플래그
- 이동: `MaxWalkSpeed` 등 — `CharacterMovement` 또는 Config에서 주입

**구현 패턴 (Skill 1·B 구현 시 준수)**
```cpp
// C++ — Config 참조만. 수치는 에셋·에디터에서.
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
TObjectPtr<UGKCombatConfig> CombatConfig;
```
- `BP_GKCharacter`에서 Config Data Asset 할당·오버라이드.
- Audio Hook(`BlueprintImplementableEvent`)과 **별개** — 훅은 이벤트 구멍, 수치는 Config.

**금지**
- `.cpp` 내부 매직 넘버로 기획 밸런스 결정
- 기획 수치 변경을 위해 C++ 재컴파일을 요구하는 구조

### 3-5. 현재 프로젝트 상태 (2026-05)
- **엔진:** UE 5.7 — 설치 완료, `GKEditor Win64 Development` 빌드 검증 완료 (2026-05-25)
- **C++ `Source/` 모듈:** `Source/GK/` — `AGKCharacter`, `AGKGameMode` 스켈레톤 + 오디오 훅 선언 완료, 캐릭터 튜닝값 `EditDefaultsOnly` 외부화 완료
- **레벨:** `Lvl_ThirdPerson` 1개 (Zone 1~3 분리 레벨은 추후 지시)
- **입력:** Enhanced Input (`Content/Input/IMC_Default`)

### 3-6. 프로젝트 부트스트랩 순서 (확정된 진행 순서)
1. ~~지침·문서 통합~~ ✅
2. ~~Git 설정 (`.gitignore`, `.gitattributes`)~~ ✅
3. ~~C++ 모듈 스켈레톤 (`Source/GK/`)~~ ✅
4. ~~UE 5.7 엔진 설치 → `.sln` 생성 → 빌드 검증~~ ✅
5. **Cleanup** — 1차 에셋 삭제 및 코드 Cleanup 완료 (`CLEANUP_SPEC.md` §2, #1·#5 보류)
6. **게임플레이** — D 기획 검증 + 기획 명세 + Skill 트리거 (`§4` 보류 항목 해소 후) ← 다음

---

## 4. 보류 항목 — KiHoon 지시 대기 (Do Not Implement Yet)

아래 항목은 AI 에이전트가 **독자적으로 구현·수치 결정하지 않습니다.** 기획 명세 또는 명시적 지시가 올 때까지 대기합니다.

### 완료 (더 이상 보류 아님)
| 항목 | 결과 |
| :--- | :--- |
| C++ 게임 모듈(`Source/GK/`) 스켈레톤 | `AGKCharacter`, `AGKGameMode`, 오디오 훅 선언 |
| 지침·Git 설정 | `AI_AGENTS_GUIDE.md`, `.gitignore`, `.gitattributes` 등 |
| UE 5.7 엔진 설치 및 C++ 빌드 검증 | `.sln` 생성, `GKEditor Win64 Development` 빌드 성공 |
| 1차 Cleanup | Rifle/Pistol/Unarmed Attack 에셋 삭제, 코드 단 템플릿 잔재 및 튜닝값 외부화 |

### 보류 중
| 항목 | 상태 |
| :--- | :--- |
| Skill 2~5 상세 정의 및 트리거 명세 | 문서 추후 업데이트 예정 |
| 콤보·회피·스테미나·히트스톱 수치 및 입력 바인딩 | 기획 명세 수령 후 |
| 무기/회피/공격 애니메이션 몽타주 경로 | 아트 에셋 준비 중 |
| 피지컬 머티리얼 6종 생성 및 Surface Type 매핑 | Skill 4 또는 별도 지시 후 |
| Zone 1~3 레벨 구성 및 Spatial Audio 볼륨 배치 | 세부 구현 지시 후 |
| Cleanup 보류분 | `Content/LevelPrototyping/`, `Content/Characters/Mannequins/Anims/Death/` — KiHoon 재결재 전 삭제 금지 |

---

## 5. 명령어 기반 실행 스킬셋 (Operational Commands)

> **주의:** Skill 2~5는 정의가 미완성입니다. 기훈님이 문서를 업데이트하기 전까지 해당 트리거를 받아도 **구현에 착수하지 말고**, "스킬 정의 업데이트 대기 중"임을 알리십시오.

기훈님이 채팅창에 아래 [트리거 문장]을 입력하면, **D의 기획 검증·임무 배분**을 거친 뒤(또는 기획 명세 수령 후), 승인된 설계를 바탕으로 최소한의 C++ 코드로 기능을 구현하고 **필수 오디오 훅(UFUNCTION)**을 반드시 배치하십시오.

### ⚔️ [Skill 1: Combat_System_Builder] — 정의 확정, 구현은 기획 명세 후
- **트리거 문장:** `전투 시스템 구현해줘`
- **구현 대상:** `AGKCharacter` (C++ 베이스) / `BP_GKCharacter` (Blueprint Child)
- **구현 내용:** 소울라이크 스타일 3단 공격 콤보, 스테미나 소모/회복, 히트 스톱(Hit Stop), 회피(구르기)
- **데이터 주도:** 기획 수치는 `§3-4` — `UGKCombatConfig`(Data Asset) 또는 EditDefaultsOnly. C++ 하드코딩 금지.
- **필수 오디오 훅:** §3-3 표준 시그니처 전부 (`OnFootstep` 포함)
- **선행 조건:** **D 기획 검증 통과** + KiHoon 기획 명세(수치·입력·애니 경로) (C++ 모듈은 완료)

### 🔒 [Skill 2~5] — 정의 보류
| Skill | 트리거 (예정) | 상태 |
| :--- | :--- | :--- |
| Stage_Progression_Manager | `단계별 해금 시스템 만들어줘` | 정의 업데이트 대기 |
| Animation_Notify_Injector | `애니메이션에 오디오 노티파이 박아줘` | 정의 업데이트 대기 |
| Physical_Material_Setup | `재질 시스템 세팅해줘` | 정의 업데이트 대기 |
| Level_Zone_Builder | `Zone [번호] 레벨 구성해줘` | 정의 업데이트 대기 |

Skill 2~5의 상세 Action·오디오 훅 목록은 `CLAUDE.md` §4에 임시 참조용으로 남겨두되, **본 문서 업데이트 전까지 구현 착수 금지**입니다.
