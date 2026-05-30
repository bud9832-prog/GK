# FEATURE_IMPLEMENTATION_STATUS.md - 피처 구현 상태 관리

> 목적: 피처 단위 완료/진행/보류 상태를 한눈에 추적하는 기준 문서.

## 1) 완료된 피처

| 피처 | 상태 | 완료일 | 담당 | 검증 | 핵심 결과 |
| :--- | :---: | :---: | :--- | :--- | :--- |
| Cleanup (엔진 코드/설정/플러그인 정리) | ✅ 완료 | 2026-05-25 | B/C 검증, D 승인 | Win64 빌드 + PIE/Wwise 초기화 로그 통과 | One-shot 포함 최종 승인 완료, 비Windows Engine.ini 재추적 해소, 불필요 pdb untrack 완료 |
| Wwise 플러그인 정리 (1차~3차) | ✅ 완료 | 2026-05-25 | B/C 검증, D 승인 | Win64 빌드 재현 성공, Error/Warning 0, PIE/Wwise 초기화 통과 | WinGC/StaticCRT 정리 + 구버전 모듈(2022_1/2023_1) 제거 완료, 최신 2024_1+Null 체인 유지 |
| Wwise 리스너 분리 (패닝=카메라, 거리=플레이어) | ✅ 완료 | 2026-05-25 | A 설계, B 구현, C 검증, D 승인 | Win64 빌드 성공, PIE/Wwise 초기화 정상, LogWwise Error 0 | SpatialAudioListener(카메라) 유지 + CharacterAkComponent 거리 Probe 분리, PossessedBy 재설정/UnPossessed 해제 적용 |
| 에이전트 운영 절차 업데이트 (A→B→C 엄수 + D 간소화 예외) | ✅ 완료 | 2026-05-31 | D 집도 | 문서 동기화 검토 통과 (`AI_AGENTS_GUIDE.md`/`CLAUDE.md`) | A→B→C 순서 강제, B·C 빌드 보고 의무, C 리뷰어 우선 원칙, D의 PIE 수동 검증 명시 지시 규칙 반영 |

## 2) 진행 예정 피처

| 피처 | 상태 | 선행 조건 | 다음 액션 |
| :--- | :---: | :--- | :--- |
| `Design/GAME_DESIGN_PILLARS.md` — 게임 디자인 기본 정의서 | 🟢 KiHoon 최종 승인 (2026-05-29, v2 입력 §7-2/§8 포함) + D 조건부 통과 4건 SKILL_01 반영 완료 | — | 후속 변경 시 동기화만 |
| `Design/SKILL_01_COMBAT_SPEC.md` v2 — Stage 1 전투 명세 (Run/Sprint, 1·2·3 콤보, 회피, 회복약, 락온) | 🟢 KiHoon 최종 승인 + D 기술 검증 완료 (2026-05-29) — §11-1 수치 12건 확정, §11-2 P 위임 확정, §11-3 D 위임 3건 채택 | KiHoon·D 명세 검증 완료 | 명세 변경 필요 시에만 동기화. §4-1 슬림화로 INPUT_MAPPING.md 참조 정리 (2026-05-30) |
| `Design/INPUT_MAPPING.md` — 입력 매핑 단일 출처(SSOT) | 🟡 P 1차 산출물 제출 (2026-05-30) | SKILL_01 v2 §4 + Pillars §8 + B 자동화 스크립트 키 매핑 통합. SKILL_01 §4-1·Pillars §8 참조 동기화 완료 | KiHoon 1차 검토 → (필요 시) D 기술 검증 → 후속 Skill 추가 시 갱신 |
| Skill 1: Combat System Builder 구현 | 🔴 **보류 종료 (최종 Go 미승인, 2026-05-29)** | PIE 런타임 증거 미수집 (tap/hold, Sprint→Evade, 상태 전이, LogWwise Error 0) | 재개 트리거: `SKILL_01 v2 보류 건 재개 - C 런타임 증거 수집부터` |

## 3) 보류/추가 검토 항목

| 항목 | 상태 | 비고 |
| :--- | :---: | :--- |
| `Content/LevelPrototyping/` 정리 | 보류 | KiHoon 재결재 필요 |
| `Content/Characters/Mannequins/Anims/Death/` 정리 | 보류 | 무기 전용 사망 애니 확정 후 결정 |
| Skill 1 런타임 최종 게이트 | 보류 | 빌드/정적 확인은 통과. PIE 증거 수집 전까지 D 최종 Go 금지 |

## 4) 업데이트 규칙

- 피처를 “완료”로 올릴 때는 아래 3개를 모두 충족해야 한다.
  1) 구현/정리 작업 완료
  2) 빌드/런타임 검증 통과
  3) D 최종 승인
- 상태 변경 시 본 문서를 먼저 갱신한 뒤, 필요한 참조 문서(`AI_AGENTS_GUIDE.md`, `ART_ASSET_CHECKLIST.md`)와 동기화한다.
