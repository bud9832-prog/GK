# SM_GK_OssuaryBlade — Unreal Engine Import Guide

## 에셋 개요

| 항목 | 값 |
|------|-----|
| 파일 | `SM_GK_OssuaryBlade.obj` + `SM_GK_OssuaryBlade.mtl` |
| 총 길이 | 약 138 cm (포멜 끝 X=-44 → 블레이드 팁 X=+94) |
| 버텍스 수 | 161 |
| 머티리얼 슬롯 | 8개 (아래 표 참고) |
| 피벗 포인트 | 그립 중앙 (월드 원점) |
| 방향 | +X = 블레이드 방향 |

## 1. Unreal Engine 임포트 설정

`Content Browser` → 우클릭 → **Import to …**

| 설정 항목 | 값 |
|-----------|-----|
| **Import Uniform Scale** | `1.0` (단위: cm) |
| **Combine Meshes** | ☑ 체크 |
| **Generate Missing Collisions** | ☑ (자동 UCX 생성) |
| **Import Materials** | ☑ (MTL 기반 재질 자동 생성) |
| **Import Textures** | ☐ (텍스처 없음 — 나중에 수동 연결) |
| **Normal Import Method** | `Import Normals` |

## 2. 저장 경로 (권장)

```
/Game/Assets/Weapons/OssuaryBlade/
  SM_GK_OssuaryBlade          ← Static Mesh
  MI_GK_OssuaryBlade_Blade    ← Material Instance (DarkSteel + BrightEdge)
  MI_GK_OssuaryBlade_Guard    ← Material Instance (Iron + Bone)
  MI_GK_OssuaryBlade_Grip     ← Material Instance (Leather + Band)
  MI_GK_OssuaryBlade_Pommel   ← Material Instance (Iron + Bone)
```

## 3. 머티리얼 슬롯 → PBR 설정 매핑

임포트 후 아래 파라미터로 Material Instance를 설정합니다.

| 슬롯 이름 | Base Color (sRGB) | Roughness | Metallic | 특이사항 |
|-----------|-------------------|-----------|----------|----------|
| `M_GK_Blade_DarkSteel` | `#0F1014` (blue-black) | 0.72 | 0.95 | 블레이드 평면 |
| `M_GK_Blade_BrightEdge` | `#8C8E95` (silver) | 0.25 | 1.0 | 날 에지 — 밝게 |
| `M_GK_Guard_Iron` | `#111111` (charcoal) | 0.80 | 0.90 | 가드 본체 |
| `M_GK_Guard_Bone` | `#B8A685` (ivory) | 0.60 | 0.0 | 가드 뼈 인레이; SSS 0.15 권장 |
| `M_GK_Grip_Leather` | `#1A0F07` (dark brown) | 0.90 | 0.0 | 가죽 그립 |
| `M_GK_Grip_Band` | `#2E2820` (bone-iron) | 0.65 | 0.50 | 밴딩 링 |
| `M_GK_Pommel_Iron` | `#181820` (polished dark) | 0.55 | 0.95 | 칼라 링 |
| `M_GK_Pommel_Bone` | `#A69672` (carved bone) | 0.55 | 0.0 | 포멜 캡; **Emissive 크랙**: `(0.30, 0.08, 0.02)` 권장 |

> `M_GK_Pommel_Bone` 에미시브: 포멜 조각 홈에 희미한 잿빛 불씨 효과를 주면 컨셉 아트의 `DimEmber` 톤과 일치합니다.

## 4. 소켓 / 어태치먼트

- 블루프린트에서 `hand_r_socket`에 어태치 시:
  - **Relative Location** `(0, 0, 0)` 우선 확인
  - 블레이드가 앞(+X)으로 향해야 정상
  - 필요 시 Relative Rotation `Z: 90°` 조정

## 5. UV 언래핑 (프로덕션 전 필수)

현재 메시는 **명시적 UV 없음** — Unreal 자동 UV로도 기본 머티리얼 확인은 가능합니다.  
최종 텍스처 적용 전에 **Blender 또는 Maya에서 UV Unwrap** 작업을 권장합니다.

권장 UV 채널 레이아웃:
- **UV0**: 일반 텍스처링 (섹션별 분리)
- **UV1**: 라이트맵 UV (균등 분포)

추천 UV 언래핑 방법:
- 블레이드: 평면 언래핑 (사이드 뷰 기준)
- 가드: 박스 프로젝션
- 그립: 실린더 언래핑
- 포멜: 스피어리컬 프로젝션

## 6. 텍스처 세트 (제작 예정)

```
T_GK_OssuaryBlade_Blade_BC      Base Color
T_GK_OssuaryBlade_Blade_MROA    Metallic / Roughness / AO
T_GK_OssuaryBlade_Blade_N       Normal Map
T_GK_OssuaryBlade_Guard_BC
T_GK_OssuaryBlade_Guard_MROA
T_GK_OssuaryBlade_Guard_N
T_GK_OssuaryBlade_Grip_BC
T_GK_OssuaryBlade_Pommel_BC
T_GK_OssuaryBlade_Pommel_N
```

## 7. 버전 히스토리

| 버전 | 날짜 | 내용 |
|------|------|------|
| 1.0 | 2026-05-30 | 161 버텍스 프로덕션 베이스 메시 완성. 8개 머티리얼 슬롯, 6-루프 다이아몬드 블레이드, 46cm 가드, 뼈 포멜 |
