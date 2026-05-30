# SM_AO_ProtoSword Import Guide

## 용도

`SM_AO_ProtoSword`는 `Ashen Ossuary` 전투 시스템의 소켓, 스케일, 콤보 판정 시각화를 확인하기 위한 프로토타입 한손검입니다.

- 최종 상용 퀄리티 에셋이 아닙니다.
- 엔진 임포트 원본은 `SM_AO_ProtoSword.obj`와 `SM_AO_ProtoSword.mtl`입니다.
- 기준 단위는 센티미터입니다.
- 검날 방향은 Unreal 기준 `+X`입니다.
- Pivot/origin은 손잡이 중앙 근처에 있습니다.

## 권장 Unreal 임포트 경로

```text
/Game/Weapons/Prototype/SM_AO_ProtoSword
```

## 임포트 설정

- Import Uniform Scale: `1.0`
- Combine Meshes: `true`
- Generate Missing Collisions: `true`
- Import Materials: `true`
- Import Textures: `false`
- Normal Import Method: `Import Normals`

## 장착 기준

- 우선 소켓 후보: `hand_r_socket`
- 소켓에서 검날이 캐릭터 전방 또는 공격 방향을 향하도록 회전 보정합니다.
- 전투 판정은 최종 무기 에셋 확정 전까지 별도 Trace/Collision 박스로 검증하는 것을 권장합니다.

## 체크리스트 반영

`ART_ASSET_CHECKLIST.md`의 "플레이어 기본 무기 메쉬" 항목은 이 에셋으로 프로토타입 수령 상태입니다. 최종 무기 메쉬는 별도 Fab/제작 에셋으로 교체해야 합니다.
