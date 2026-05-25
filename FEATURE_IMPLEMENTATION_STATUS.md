# FEATURE_IMPLEMENTATION_STATUS.md - 피처 구현 상태 관리

> 목적: 피처 단위 완료/진행/보류 상태를 한눈에 추적하는 기준 문서.

## 1) 완료된 피처

| 피처 | 상태 | 완료일 | 담당 | 검증 | 핵심 결과 |
| :--- | :---: | :---: | :--- | :--- | :--- |
| Cleanup (엔진 코드/설정/플러그인 정리) | ✅ 완료 | 2026-05-25 | B/C 검증, D 승인 | Win64 빌드 + PIE/Wwise 초기화 로그 통과 | One-shot 포함 최종 승인 완료, 비Windows Engine.ini 재추적 해소, 불필요 pdb untrack 완료 |
| Wwise 플러그인 정리 (1차~2차) | ✅ 완료 | 2026-05-25 | B/C 검증, D 승인 | Win64 빌드 재현 성공, Error/Warning 0 | WinGC/StaticCRT 대상 정리 및 추적 상태 정상화, 필수 include/lib 및 핵심 모듈 보존 |

## 2) 진행 예정 피처

| 피처 | 상태 | 선행 조건 | 다음 액션 |
| :--- | :---: | :--- | :--- |
| Skill 1: Combat System Builder | ⏸ 대기 | KiHoon 전투 기획 명세(수치/입력/몽타주) + D/P 승인 루프 완료 | P 기획서 + 아트 어셋 체크리스트 제출 → D 검증 → A/B/C 착수 |

## 3) 보류/추가 검토 항목

| 항목 | 상태 | 비고 |
| :--- | :---: | :--- |
| `Content/LevelPrototyping/` 정리 | 보류 | KiHoon 재결재 필요 |
| `Content/Characters/Mannequins/Anims/Death/` 정리 | 보류 | 무기 전용 사망 애니 확정 후 결정 |
| Wwise 구버전 모듈 (`Source/WwiseSoundEngine_2022_1`, `2023_1`) | 보류 | Build.cs 의존성으로 즉시 삭제 불가, D 별도 승인 필요 |

## 4) 업데이트 규칙

- 피처를 “완료”로 올릴 때는 아래 3개를 모두 충족해야 한다.
  1) 구현/정리 작업 완료
  2) 빌드/런타임 검증 통과
  3) D 최종 승인
- 상태 변경 시 본 문서를 먼저 갱신한 뒤, 관련 상세 문서(`CLEANUP_SPEC.md` 등)와 동기화한다.
