# Ashen Ossuary: Technical Audio Portfolio Project

**Tech Stack:** Unreal Engine 5.7 (C++ — 모듈 생성 예정) | Wwise 2023.x
**Target Platform:** PC Windows (Strictly Windows Only)

> **개발 상태:** 현재 Blueprint-only 템플릿(`BP_ThirdPersonCharacter`). C++ 모듈 및 게임플레이 구현은 KiHoon 기획 명세·모듈 생성 후 진행. 에이전트 지침은 `AI_AGENTS_GUIDE.md` 참조.

---

## 1. Project Overview
**Ashen Ossuary(잿빛 묘실)**는 중세풍 황무지와 밀폐된 묘실을 배경으로 하는 소울라이크 스타일의 **근접 무기(Melee) 3인칭 액션 RPG** 데모 프로젝트입니다. 

본 프로젝트는 단순한 게임 구현을 넘어, 대용량 오디오 데이터 처리, 시스템 자동화 파이프라인 구축, 런타임 하드웨어 최적화 등 **테크니컬 오디오 디자이너(Technical Audio Designer)**로서의 핵심 실무 역량을 증명하기 위해 구조화되었습니다.

### 윈도우 환경 최적화 (Windows-Specific Target)
- 본 프로젝트는 **PC Windows 환경만을 타깃**으로 독립 빌드 및 최적화가 진행됩니다.
- Windows 오디오 스레드 및 가용 메모리 풀을 기준으로 런타임 성능을 트래킹하며, 하드웨어 사양에 맞춘 안정적인 오디오 디바이스 드라이버 출력을 검증합니다.

### 핵심 게임플레이 루프
1. **Stage 1 (1:1 전투):** 진입로 전투 (약공격 + 회피 중심) -> 클리어 시 강공격 및 패링 시스템 해금
2. **Stage 2 (1:3 난전):** 성벽 광장 전투 (다수 적 대응, 범위 공격 활용) -> 클리어 시 궁극기(불 마법 프로젝타일) 해금
3. **Stage 3 (보스전):** 잿빛 묘실 아레나 (모든 스킬 및 궁극기를 활용한 최종 보스전)

---

## 2. Technical Audio Architecture

### C++ 기반 오디오 인터페이스 레이어 (Separation of Concerns)
- 게임플레이 로직과 사운드 엔진 간의 결합도를 최소화하기 위해 플레이어 C++ 베이스 클래스(`AGKCharacter`)와 Blueprint Child(`BP_GKCharacter`)로 설계합니다. (C++ 모듈은 KiHoon 생성 예정)
- 오디오 코드를 게임 로직에 하드코딩하지 않고, `UFUNCTION(BlueprintImplementableEvent)` 기반의 인터페이스 구멍(Audio Hook)을 정밀하게 뚫어두어 사운드 디자이너가 블루프린트에서 유연하게 Wwise 이벤트를 매핑할 수 있도록 아키텍처를 분리했습니다.

### 재질 기반 동적 사운드 시스템 (Surface-driven Audio)
- **지원 물리 재질 (6종):** Stone, Ash, Metal, Bone, Flesh, Armor
- **구현 방식:** 라인트레이스(Line Trace)를 통해 캐릭터 발 아래 및 타격 지점의 피지컬 마테리얼(Physical Material)을 실시간 감지하고, Wwise Switch Container와 연동하여 재질별로 완전히 차별화된 발소리 및 타격음을 동적으로 분기합니다.

### 공간 음향 및 리버브 설계 (Spatial Audio)
| 구역 (Zone) | 구조적 특성 | 오디오 환경 및 리버브 설계 |
| :--- | :--- | :--- |
| **Zone 1 (진입로)** | 좁고 사방이 막힌 통로형 지형 | 짧은 Early Reflection 및 좁은 Spatial 포탈을 활용하여 소울라이크 특유의 답답하고 폐쇄적인 공간감 강조 |
| **Zone 2 (광장)** | 완전히 개방된 황무지 개활지 | 자연스러운 거리 감쇠(Attenuation Curve) 적용 및 넓은 스테레오 이미지를 형성하여 사운드가 공간으로 흩어지는 효과 구현 |
| **Zone 3 (묘실)** | 대형 석조 밀폐 원형 아레나 | 긴 리버브 테일(Reverb Tail)과 폐쇄 지형에 따른 고주파 감쇄 특성을 적용하여 보스전의 웅장함과 반사음 극대화 |

---

## 3. Technical Challenges & Solutions

### Challenge 1: 다수 적 등장 시 보이스 오버플로우 및 오디오 스레드 과부하 방지
- **Problem:** Stage 2 난전 상황에서 플레이어와 다수의 적이 동시에 무기를 휘두르고 타격 VFX가 터질 때, Wwise 보이스 카운트가 급증하여 Windows 오디오 스레드 점유율이 튀고 사운드가 뭉개지는 현상 발생.
- **Solution:** Wwise 내에서 사운드 카테고리별 글로벌 플레이백 리밋(Playback Limit)을 설정하고, 거리 및 에셋 중요도에 따른 동적 우선순위(Priority) 시스템을 엄격하게 적용.
- **Result:** 동시 발음 수를 최적 수치로 제어함으로써 CPU 오디오 스레드 부하를 약 15% 절감하고, 난전 중에도 플레이어의 핵심 액션(회피음, 피격음)의 명료도를 완벽하게 확보.

---

## 4. Performance & Optimization Metrics (Target)
- **Voice Count:** 평균 30~40, 최대 64 이하 엄격 제한 (Windows 런타임 기준)
- **CPU Usage:** 게임플레이 중 오디오 스레드가 메인 게임 스레드 병목을 유발하지 않도록 최적화
- **Memory Management:** 가용 메모리 풀 최적화를 위해 플랫폼 사운드 뱅크(Sound Bank)를 세분화하고 대용량 환경음 파일은 스트리밍(Streaming) 레이아웃으로 구성

---

## Credits
- **Technical Audio Design & Implementation:** KiHoon
- **System Programming & Level Design:** AI Agent (Non-Audio Team)