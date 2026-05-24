# CLEANUP_SPEC.md - 레포지토리 정리 및 핵심 메카닉스 구현 명세서

> **문서 우선순위:** `AI_AGENTS_GUIDE.md` §3·§4 > 본 문서. 클래스 명칭·보류 항목은 `AI_AGENTS_GUIDE.md`를 따릅니다.

## 1. 개요 및 목적
- 본 프로젝트는 언리얼 엔진 5(UE5)와 Wwise를 연동한 3인칭 액션 RPG 'Ashen Ossuary' 테크니컬 오디오 포트폴리오 빌드를 목적으로 한다.
- 개발 효율화를 위해 게임플레이 코어 로직 및 시스템 코딩은 AI 에이전트가 전담하며, 사운드 디자이너 KiHoon은 오디오 구현, 믹싱, 최적화에만 집중할 수 있도록 구조를 분리한다.
- **전투 방식:** 근접 무기(Melee) 전투. 캐릭터는 무기를 장착한다.

---

## 2. 1단계: 레거시 코드 및 에셋 청소 (Cleanup)

**상태: ⏸ KiHoon 별도 지시 대기**

- 현재 프로젝트의 레포지토리를 전수 조사하여, '3인칭 근접 액션 RPG 기본 플레이어' 빌드와 무관한 레거시 시스템, 미사용 변수, 더미 컴포넌트를 삭제하거나 참조를 제거한다.
- 컴파일 에러가 없는 상태를 유지하며, 베이스 캐릭터 클래스와 핵심 게임모드만 남긴다.

### 청소 시 참고 (삭제 여부는 KiHoon 확정 후)
| 대상 | 판단 기준 |
| :--- | :--- |
| `Content/LevelPrototyping/` (JumpPad, Door 등) | 근접 전투 데모와 무관하면 삭제 후보 |
| `Mannequins/Anims/Rifle`, `Pistol` | **원거리 전투 범위 아님** — 삭제 후보 |
| `Mannequins/Anims/Unarmed/Attack/` | 임시 공격 모션. 무기 전용 애니 수령 후 교체 |
| Wwise 플러그인·`/Content/WwiseAudio/` | **절대 삭제·수정 금지** |
| `BP_ThirdPersonCharacter` | C++ `AGKCharacter` 도입 전까지 **유지** |

---

## 3. 2단계: 핵심 게임플레이 메카닉스 구현 (Core Mechanics)

**상태: ⏸ C++ 모듈 생성 + KiHoon 기획 명세 수령 후**

청소가 완료된 베이스 플레이어 클래스(`AGKCharacter` / `BP_GKCharacter`)에 아래 기능을 구현한다.

### 3-1. 3단 공격 콤보 시스템
- **기능:** 기본 공격 버튼 입력 시 공격 1 → 공격 2 → 공격 3으로 연계되는 소울라이크 콤보. **근접 무기** 휘두르기 기준.
- **사양:** 애니메이션 전환 및 Combo Window 계산 로직 포함. 몽타주 경로·입력 윈도우 수치는 기획 명세에서 확정.

### 3-2. 회피 시스템 (구르기)
- **기능:** 회피 버튼 입력 시 지정 방향으로 구르기.
- **사양:** 가속도 제어 및 Invincibility Frame 처리. i-frame 길이·스테미나 소모량은 기획 명세에서 확정.

### 3-3. 스테미나 / 히트 스톱 (Skill 1 범위)
- 스테미나 소모/회복, Hit Stop은 Skill 1(`전투 시스템 구현해줘`) 트리거 시 함께 구현. 수치는 기획 명세에서 확정.

---

## 4. 3단계: 테크니컬 오디오 인터페이스 설계 (Audio Interface Holes)

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
- [ ] KiHoon: 전투 기획 명세 (수치, 입력, 몽타주 경로)
- [ ] KiHoon: 1단계 Cleanup 삭제 대상 확정
- [ ] KiHoon: Skill 2~5 및 Zone 레벨 세부 지시 (해당 작업 시)
