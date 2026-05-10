# Ashen Ossuary: Technical Audio Portfolio Project
**Tech Stack:** Unreal Engine 5.x | Wwise 2023.x | C++ | Blueprint

중세풍 소울라이크 전투 데모 프로젝트입니다. 테크니컬 오디오 디자인 역량을 증명하기 위해 제작되었습니다.

---

## Project Overview
### 게임 컨셉
무거운 분위기의 중세풍 황무지에서 벌어지는 3단계 전투 데모

### 핵심 게임 루프
1. **Stage 1:** 1:1 전투 (약공격 + 닷지 중심)
   - 클리어 시 강공격 및 방어/패링 시스템 해금
2. **Stage 2:** 1:3 전투 (다수 적 대응, 강공격/범위 공격 활용)
   - 클리어 시 궁극기(불 마법 프로젝타일) 시스템 해금
3. **Stage 3:** 보스전 (모든 스킬 및 궁극기 활용)

---

## Technical Implementation
### 오디오 시스템 아키텍처
- **Wwise-Unreal Integration:** Event-based 호출 구조 및 전역 RTPC를 활용한 다이내믹 믹싱 환경 구축
- **Automation:** Python 스크립트를 활용한 애니메이션 노티파이 일괄 자동화 적용

### 재질 기반 사운드 시스템 (Surface-driven Audio)
- **Surface Types:** Stone, Ash, Metal, Bone, Flesh, Armor
- **Implementation:** - Physical Material을 통한 Surface Type 판별 로직 구현
  - Wwise Switch Container 연동을 통한 재질별 발소리 및 타격음 분기

### 공간 음향 설계 (Spatial Audio)
| Zone | 특성 | 리버브 및 공간 설계 |
|------|------|-------------------|
| Zone 1 (진입로) | 좁은 통로형 | 짧은 Early Reflection, 좁은 공간감 강조 |
| Zone 2 (광장) | 개방된 개활지 | 자연스러운 거리 감쇠(Attenuation) 및 넓은 공간감 |
| Zone 3 (묘실) | 밀폐된 원형 아레나 | 긴 Reverb Tail, 폐쇄 지형에 따른 울림 강조 |

---

## Technical Challenges & Solutions
### Challenge 1: 다수의 적 등장 시 보이스 오버플로우 방지
- **Problem:** Stage 2(1:3 전투)에서 다수의 타격음이 겹칠 때 발생하는 보이스 카운트 급증 및 사운드 명료도 저하
- **Solution:** Wwise의 Playback Limit 설정 및 우선순위(Priority) 시스템 적용
- **Result:** 동시 발음 수 제어를 통한 CPU 부하 15% 절감 및 핵심 전투음 명료도 확보

---

## Optimization Metrics
- **Voice Count:** 평균 30~40, 최대 64 이하 유지
- **CPU Usage:** 오디오 스레드 점유율 최적화
- **Memory Management:** 플랫폼별 사운드 뱅크(Bank) 세분화 및 스트리밍 설정

---

## Credits
- **Technical Audio Design & Implementation:** KiHoon
- **System Programming & Level Design:** AI Agent (Non-Audio Team)