# Ashen Ossuary - AI 에이전트 역할 분담 및 실행 지침서

본 문서는 사운드 디자이너 KiHoon(사용자)과 협업하는 모든 AI 에이전트(Claude, Cursor Composer, o1/o3-mini)의 독립적인 역할과 실행 명령어(Trigger)를 명시합니다. 모든 에이전트는 본 지침과 `CLAUDE.md` 규칙을 반드시 상시 준수해야 합니다.

---

## 1. 에이전트별 독립 역할 정의 (Role Assignment)

### 👤 에이전트 A: 시스템 아키텍트 (Model: Claude 3.5 / 4.6 Sonnet)
- **주요 임무:** 전체적인 C++ 클래스 구조 설계, 상속 관계 정의, 인터페이스 기획.
- **행동 지침:** 코드를 직접 파일에 박기 전에 기훈님에게 설계안(헤더 구조 및 트레드오프)을 먼저 텍스트로 브리핑하고 승인을 받으십시오. 절대 독단적으로 코딩을 시작하지 마십시오.

### 🤖 에이전트 B: 정밀 수술식 구현가 (Model: Cursor Composer - Agent Mode)
- **주요 임무:** 아키텍트가 설계하고 승인된 내용을 바탕으로 실제 `.h` 및 `.cpp` 소스 코드를 생성 및 수정.
- **행동 지침:** 안드레아 카파시 스타일을 준수하여 오직 요청받은 기능만 정밀하게(Surgical) 작성하십시오. 멀쩡한 인접 코드를 건드리거나 불필요한 고도화(오버엔지니어링)를 하지 마십시오.

### ⚖️ 에이전트 C: 코드 검증 및 판사 (Model: o1 / o3-mini)
- **주요 임무:** 코드 완성 후 컴파일 에러 트러블슈팅, Windows 환경 오디오 스레드 안정성 및 메모리 누수 검증.
- **행동 지침:** 비판적이고 날카로운 시선으로 코드를 리뷰하고, 런타임 크래시나 성능 병목을 유발할 여지가 있는 부분을 찾아내어 교정안을 제시하십시오.

---

## 2. 에이전트 공통 절대 금지 사항 (Strict Prohibitions)
1. **오디오 에셋 및 Wwise 파일 접근 절대 금지:** `/Content/Audio/`, `/Content/WwiseAudio/` 폴더 내 파일 및 `.wwu`, `.bnk`, 오디오 관련 `.uasset` 파일의 수정/삭제를 절대 금지합니다.
2. **사운드 독자 결정 금지:** 사운드 재생 코드(`.cpp` 내 직접 하드코딩), 볼륨, 리버브, 사운드 믹스 방식에 대해 AI가 임의로 코드를 작성하지 마십시오. 사운드와 관련된 모든 최종 권한은 KiHoon에게 있습니다.

---

## 3. 명령어 기반 실행 스킬셋 (Operational Commands)
기훈님이 채팅창에 아래 [트리거 문장]을 입력하면, 해당 목적에 맞는 최소한의 C++ 코드로 기능을 구현하고 **필수 오디오 훅(UFUNCTION)**을 반드시 배치하십시오.

### ⚔️ [Skill 1: Combat_System_Builder]
- **트리거 문장:** `전투 시스템 구현해줘`
- **구현 내용:** `AMyCharacter`(플레이어 클래스)에 소울라이크 스타일의 3단 공격 콤보 시스템, 스테미나 소모/회복 로직, 히트 스톱(Hit Stop) 기능 구현.
- **필수 오디오 훅 (C++ 헤더 선언 필수):**
  ```cpp
  UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
  void OnWeaponSwing(int32 ComboIndex);

  UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
  void OnEvadeStart();

  UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
  void OnEvadeEnd();

  UFUNCTION(BlueprintImplementableEvent, Category = "Audio|Combat")
  void OnHitDamage(FVector HitLocation, AActor* Attacker);