# SM_AO_OssuaryBlade Import Guide

## 용도

`SM_AO_OssuaryBlade`는 컨셉 아트의 잿빛 묘실 분위기에 맞춘 히어로 스타일 한손검입니다. 최종 상용 에셋은 아니지만, 단순 박스 프로토타입이 아니라 플레이어 무기 실루엣과 스크린샷용 분위기 확인까지 겸하는 에셋입니다.

- 엔진 임포트 원본은 `SM_AO_OssuaryBlade.obj`와 `SM_AO_OssuaryBlade.mtl`입니다.
- 기준 단위는 센티미터입니다.
- 검날 방향은 Unreal 기준 `+X`입니다.
- Pivot/origin은 손잡이 중앙 근처에 있습니다.
- 전체 길이는 약 181cm로, 일반 한손검보다 크고 소울라이크 히어로 무기 실루엣에 가깝습니다.

## 권장 Unreal 임포트 경로

```text
/Game/Weapons/Prototype/SM_AO_OssuaryBlade
```

## 임포트 설정

- Import Uniform Scale: `1.0`
- Combine Meshes: `true`
- Generate Missing Collisions: `true`
- Import Materials: `true`
- Import Textures: `false`
- Normal Import Method: `Compute Normals` 또는 `Import Normals`

## 확인 포인트

- Static Mesh 에디터에서 `F`를 눌러 프레임을 맞춥니다.
- 블레이드가 `+X` 방향으로 길게 뻗어 있어야 합니다.
- 손에 붙였을 때 너무 크면 장착 액터 스케일을 `0.75`부터 조정합니다.
- `AO_DimEmber` 머티리얼은 나중에 Unreal 머티리얼에서 Emissive로 바꾸면 중앙 균열 연출에 쓸 수 있습니다.

## 장착 기준

- 우선 소켓 후보: `hand_r_socket`
- 큰 무기 실루엣이라 캐릭터 손목 회전 보정이 필요할 수 있습니다.
- 전투 판정은 최종 무기 에셋 확정 전까지 별도 Trace/Collision 박스로 검증합니다.
