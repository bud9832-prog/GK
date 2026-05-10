# Agent Skills for Ashen Ossuary Project

## Skill: Combat_System_Builder
- **Trigger:** "전투 시스템 구현해줘" 또는 "콤보 로직 짜줘"
- **Action:** `BP_PlayerCharacter`에 콤보 시스템, 스테미나, 히트 스톱 구현
- **Audio Hook:** 모든 공격 판정 시점에 `OnAttackHit` Event Dispatcher 노출 필수

## Skill: Stage_Progression_Manager
- **Trigger:** "단계별 해금 시스템 만들어줘"
- **Action:** `GameMode` 내 `CurrentStage` 변수 생성 및 스킬 해금 로직 구현
- **Audio Hook:** `OnStageCleared`, `OnSkillUnlocked` Dispatcher 노출

## Skill: Animation_Notify_Injector
- **Trigger:** "애니메이션에 오디오 노티파이 박아줘"
- **Action:** 모든 공격/이동 몽타주 스캔 후 타격/발소리 타이밍에 `AN_Audio_*` Notify 배치

## Skill: Physical_Material_Setup
- **Trigger:** "재질 시스템 세팅해줘"
- **Action:** Physical Material 6종 생성 및 레벨 메시 할당
- **Audio Hook:** Wwise Switch Group과 매핑되는 데이터 에셋 구조 생성

## Skill: Portfolio_Reporter
- **Trigger:** "기술 리포트 업데이트해줘"
- **Action:** 최근 구현 기능 분석 후 `README.md`를 Challenge-Solution-Result 형식으로 업데이트

## Skill: Level_Zone_Builder
- **Trigger:** "Zone [번호] 레벨 구성해줘"
- **Action:** 기획서 명세에 따른 지오메트리 배치 및 물리 재질 할당
- **Audio Hook:** 리버브 존(Reverb Volume) 위치 및 크기 제안서 작성