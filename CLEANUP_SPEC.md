# CLEANUP_SPEC.md - 레포지토리 정리 및 핵심 메카닉스 구현 명세서

> **문서 우선순위:** `AI_AGENTS_GUIDE.md` §0·§2 > §3 > §4 > 본 문서.

## 1. 개요 및 목적
- 본 프로젝트는 언리얼 엔진 5(UE5)와 Wwise를 연동한 3인칭 액션 RPG 'Ashen Ossuary' 테크니컬 오디오 포트폴리오 빌드를 목적으로 한다.
- 개발 효율화를 위해 게임플레이 코어 로직 및 시스템 코딩은 AI 에이전트가 전담하며, 사운드 디자이너 KiHoon은 오디오 구현, 믹싱, 최적화에만 집중할 수 있도록 구조를 분리한다.
- **전투 방식:** 근접 무기(Melee) 전투. 캐릭터는 무기를 장착한다.

---

## 2. 1단계: 레거시 코드 및 에셋 청소 (Cleanup)

**상태: ✅ 1차 Cleanup 검증 완료 (2026-05-25, 에이전트 B/C)** — #2·#3·#4 삭제 실행, 코드 단 Cleanup 반영, `GKEditor Win64 Development` 빌드 성공. #1·#5 보류 유지.

> **빌드 검증 완료** — UE 5.7 (`E:\UE_5.7`)에서 `.sln` 생성 및 `GKEditor Win64 Development` 빌드 성공 (2026-05-25).
> **참조 분석 완료 (2026-05-25, 에이전트 A)** — 아래 후보 전체에 대해 유지 대상 파일(`Lvl_ThirdPerson.umap`, `BP_ThirdPersonCharacter`, `ABP_Unarmed`, `__ExternalActors__/**`) 에서 참조 0건 확인. 끊김 위험 없음.
> **코드 단 Cleanup 완료 (2026-05-25, 에이전트 B/C)** — `ProjectName=Ashen Ossuary` 반영, `AGKCharacter` 캡슐·이동·카메라·스태미나 기본값을 `EditDefaultsOnly`로 외부화하고 `OnConstruction()`/`BeginPlay()`에서 적용.

- 현재 프로젝트의 레포지토리를 전수 조사하여, '3인칭 근접 액션 RPG 기본 플레이어' 빌드와 무관한 레거시 시스템, 미사용 변수, 더미 컴포넌트를 삭제하거나 참조를 제거한다.
- 컴파일 에러가 없는 상태를 유지하며, 베이스 캐릭터 클래스와 핵심 게임모드만 남긴다.

### 청소 후보 결재 표 (KiHoon 확정 항목에 체크)

| # | 대상 경로 | 파일 수 | 용량 | 삭제 근거 | 위험도 | KiHoon 결재 |
| :-: | :--- | ---: | ---: | :--- | :---: | :--- |
| 1 | `Content/LevelPrototyping/` | 29 | 2.9 MB | 근접 전투 데모와 무관한 프로토타이핑 도구(JumpPad, Door, WobbleTarget, 그리드 재질). 유지 대상에서 참조 없음 | 🟢 낮음 | `[ ] 삭제` `[V] 보류` `[ ] 제외` |
| 2 | `Content/Characters/Mannequins/Anims/Rifle/` | 39 | 14.2 MB | **원거리 전투 범위 아님** (§3-2 Melee 전용 확정). Walk·Jog·Aim·Fire 등 소총 전용 모션 전체. 참조 없음 | 🟢 낮음 | ✅ **삭제 완료** |
| 3 | `Content/Characters/Mannequins/Anims/Pistol/` | 29 | 12.8 MB | **원거리 전투 범위 아님** (§3-2). 권총 전용 모션 전체. 참조 없음 | 🟢 낮음 | ✅ **삭제 완료** |
| 4 | `Content/Characters/Mannequins/Anims/Unarmed/Attack/` | 4 | 3.5 MB | 임시 맨손 공격 모션 (`MM_Attack_01~03`, `MM_ChargedAttack`). 무기 전용 애니 수령 후 교체 예정. `ABP_Unarmed`에서 참조 없음 확인 | 🟡 중간 | ✅ **삭제 완료** |
| 5 | `Content/Characters/Mannequins/Anims/Death/` | 6 | 1.8 MB | 맨손 사망 모션 (`MM_Death_*`). 무기 전용 사망 애니 수령 후 교체 예정. 유지 대상 참조 없음 | 🟡 중간 | `[ ] 삭제` `[V] 보류` `[ ] 제외` |

> **#4·#5 위험도 중간 이유:** `ABP_Unarmed`(Unarmed 로코모션 레이어)와 같은 폴더 계층에 있어 향후 신규 ABP 제작 시 모션 원본 필요 여부를 KiHoon(아트·애니메이션 관리자)이 판단해야 함. 텍스트 참조 기준으로는 안전.

### 절대 유지 대상 (결재 불필요)
| 대상 | 이유 |
| :--- | :--- |
| Wwise 플러그인·`Content/WwiseAudio/`·`GK_WwiseProject/` | §2 절대 금지 — KiHoon 소유, AI 수정 금지 |
| `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter` | `BP_GKCharacter` 전환 전까지 유지 (§3-1) |
| `Content/Characters/Mannequins/Anims/Unarmed/` (Attack/ 제외) | 현재 플레이어 로코모션(`ABP_Unarmed`, 이동 모션) 사용 중 |
| `Content/Input/` | Enhanced Input (`IMC_Default`, `IA_*`) — 플레이어 입력 핵심 |
| `Config/Windows/`, `Config/Default*.ini`의 Windows 런타임 필수 항목 | Win64 빌드 및 런타임 기준 설정 |

### 플랫폼 Config 정리 정책 (Windows-only)
- 프로젝트 타깃은 Windows 단일.
- **Engine ini Cleanup 완료 (2026-05-25, 에이전트 B):** 비Windows `*Engine.ini` 7개 삭제 후 Win64 빌드 통과. 삭제로 비워진 `Config/Android/`, `Config/IOS/`, `Config/TVOS/`, `Config/VisionOS/`, `Config/LinuxArm64/` 빈 폴더 제거.
- **유지 (Wwise 초기화·§2):** `Config/Linux/LinuxGame.ini`, `Config/Mac/MacGame.ini`, `Platforms/Android/Config/AndroidGame.ini`
- **잔여 검토:** `Config/DefaultEngine.ini` 내 Linux/Mac TargetSettings, AndroidFileServer `SecurityToken` — KiHoon 별도 확인.

### Wwise 플러그인 Cleanup 진행 현황 (Windows-only, 2026-05-25)
- **1차 완료/승인 (B 실행, C 검증):** `ThirdParty/WinGC_vc160/**`, `ThirdParty/WinGC_vc170/**`, 영문/일문/중문 `.chm` 삭제. Win64 빌드 통과.
- **2차 1회 완료/승인 (B 실행, C 검증):** `ThirdParty/x64_vc160/**`, `ThirdParty/x64_vc170/Debug/**` 삭제. Win64 빌드 재현 성공, Error/Warning 0.
- **2차 2회 완료/승인 (B 실행, C 검증):** `ThirdParty/x64_vc170/Profile/**`, `Release/**`, `Debug(StaticCRT)/**` 내 `pdb` 158개 삭제. Win64 빌드 재현 성공, Error/Warning 0.
- **One-shot 완료/승인 (B 실행, C 검증):** `b942f5e`에서 재추적된 비Windows `*Engine.ini` 7개 재삭제 + `.gitignore` 재오염 방지 규칙 추가 + `x64_vc170 Profile(StaticCRT)/Release(StaticCRT)`의 `pdb` 69개 `git rm --cached` untrack 완료. Win64 빌드/PIE/Wwise 초기화 로그 통과.
- **유지 확정:** `ThirdParty/include/**`, `ThirdParty/x64_vc170/Profile(StaticCRT)/**`, `Wwise_UE_Integration_ko.chm`, `Source/WwiseSoundEngine_2022_1/**`, `Source/WwiseSoundEngine_2023_1/**`, `Source/WwiseSoundEngine_2024_1/**`, `Source/WwiseSoundEngine_Null/**`
- **2차 상태:** `pdb` 정리 트랙(1회+2회) 완료.
- **후속 검토(3차 후보):** `Source/WwiseSoundEngine_2022_1/**`, `Source/WwiseSoundEngine_2023_1/**` 등 구버전 모듈 정리는 D 별도 승인 후 진행.
- **런타임 잔여 점검:** PIE 기준 Wwise 초기화 로그 확인 1회.

---

## 3. 2단계: 핵심 게임플레이 메카닉스 구현 (Core Mechanics)

**상태: ⏸ KiHoon 기획 명세 수령 후** (C++ 모듈 `Source/GK/`는 완료)

**[데이터 주도]** §3-1~3-3의 **수치·윈도우·몽타주 참조**는 C++ 매직 넘버 금지. `UGKCombatConfig`(Data Asset) 또는 `EditDefaultsOnly`로 에디터·테이블에서 조정. 상세는 `AI_AGENTS_GUIDE.md` §3-4.

청소가 완료된 베이스 플레이어 클래스(`AGKCharacter` / `BP_GKCharacter`)에 아래 기능을 구현한다.

### 3-1. 3단 공격 콤보 시스템
- **기능:** 기본 공격 버튼 입력 시 공격 1 → 공격 2 → 공격 3으로 연계되는 소울라이크 콤보. **근접 무기** 휘두르기 기준.
- **사양:** 애니메이션 전환 및 Combo Window **로직** 포함.
- **기획 변수 (외부화):** 콤보 입력 윈도우(초), 리셋 시간, `UAnimMontage*` 참조 — Config·에디터.

### 3-2. 회피 시스템 (구르기)
- **기능:** 회피 버튼 입력 시 지정 방향으로 구르기.
- **사양:** 가속도 제어 및 Invincibility Frame **로직**.
- **기획 변수 (외부화):** i-frame 길이, 구르기 속도, 스테미나 소모량 — Config·에디터.

### 3-3. 스테미나 / 히트 스톱 (Skill 1 범위)
- 스테미나 소모/회복, Hit Stop **로직**은 Skill 1 트리거 시 구현.
- **기획 변수 (외부화):** 최대/회복/소모량, 히트 스톱 지속 시간·적용 대상 — Config·에디터.

---

## 4. 3단계: 테크니컬 오디오 인터페이스 설계 (Audio Interface Holes)

**상태:** 훅 **선언** 완료 (`AGKCharacter.h`) — 호출부·Line Trace·노티파이 연동은 Skill 1 / Cleanup 후

**[중요]** 게임플레이 로직 내부에 Wwise 재생 코드를 직접 하드코딩하지 않는다. 각 액션 타이밍에 호출되는 **오디오 전용 인터페이스(구멍)**만 C++에 선언하고, 내부 구현은 비워둔다. Wwise 매핑은 KiHoon이 Blueprint에서 처리한다.

### 4-1. 표준 오디오 훅 목록

`AI_AGENTS_GUIDE.md` §3-3과 동일. 요약:

1. **`OnFootstep(EPhysicalSurface SurfaceType)`** — 발소리 노티파이 시점. Line Trace로 Surface 감지.
2. **`OnWeaponSwing(int32 ComboIndex)`** — 콤보 공격 무기 휘두르기 시작 시점.
3. **`OnHitDamage(FVector HitLocation, AActor* Attacker)`** — 피격 즉시. `Attacker` 필수.
4. **`OnEvadeStart()`** — 구르기 시작 첫 프레임.
5. **`OnEvadeEnd()`** — 구르기 종료, 정상 이동 복귀.

모든 훅은 `UFUNCTION(BlueprintImplementableEvent, Category = "Audio|...")` 형태로 `AGKCharacter` 헤더에 선언한다.

---

## 5. 구현 선행 조건 체크리스트

아래가 충족되기 전 2~4단계 구현에 착수하지 않는다.

- [x] C++ 게임 모듈(`Source/GK/`) 생성 — `AGKCharacter`, `AGKGameMode` 스켈레톤
- [x] 오디오 훅 선언 (`AGKCharacter.h` — 호출부는 미구현)
- [x] UE 5.7 엔진 설치 및 C++ 빌드 검증 (`GKEditor Win64 Development` 성공)
- [ ] KiHoon: 전투 기획 명세 (수치, 입력, 몽타주 경로)
- [x] KiHoon: 1단계 Cleanup 삭제 대상 확정 (#2·#3·#4 삭제, #1·#5 보류)
- [ ] KiHoon: Skill 2~5 및 Zone 레벨 세부 지시 (해당 작업 시)

---

## 6. 아트 어셋 업데이트 이후 전투 기획 착수 체크

내일 아트 어셋 업데이트 이후, D 검증 단계에서 아래를 우선 확인한다.

1. **경로 동결:** 무기/회피/공격 관련 최종 에셋 경로 확정 (`BP_GKCharacter`에서 참조 가능한 형태)
2. **몽타주 맵핑표:** 콤보 1~3, 회피, 피격/히트스톱 연계에 사용할 `UAnimMontage*` 목록 정리
3. **입력 확정:** `IMC_Default` 기준 공격/회피 입력 액션명 및 트리거 방식(탭/홀드) 확정
4. **기획 변수 표:** 스테미나(최대/회복/소모), 콤보 윈도우, i-frame, 히트스톱 시간을 Data-Driven 형식으로 확정
5. **보류 확인:** #1(`LevelPrototyping`)·#5(`Death`)는 전투 기획 영향도 재확인 후 유지/삭제 재결재

> D 노티 원칙: 매 세션 시작 시 위 5항목의 완료/미완료 상태를 1회 요약 보고하고, 누락 항목만 다음 할 일로 안내한다.
