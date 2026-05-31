"""
Ashen Ossuary — 구버전 리깅 에셋 정리
에이전트 E 작성

실행: UE Editor > Tools > Execute Python Script > 이 파일
     → import_meshy_assets.py 재실행
     → setup_retargeting.py 실행
"""

import unreal

TO_DELETE = [
    "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight",
    "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight_Skeleton",
    "/Game/Characters/Player/Mesh/IK_Mannequin",
    "/Game/Characters/Player/Mesh/IK_GK_ObsidianKnight",
    "/Game/Characters/Player/Mesh/RTG_Mannequin_ObsidianKnight",
    "/Game/Characters/Player/Mesh/MI_GK_ObsidianKnight",
]

unreal.log("[GK Cleanup] 구버전 리깅 에셋 삭제 시작...")
for path in TO_DELETE:
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
        unreal.log(f"[GK Cleanup] 삭제: {path}")
    else:
        unreal.log(f"[GK Cleanup] 없음 (건너뜀): {path}")

unreal.log("[GK Cleanup] 완료. 이제 다음 순서로 실행하세요:")
unreal.log("  1. scripts/import_meshy_assets.py  (Mixamo SK 재임포트)")
unreal.log("  2. scripts/setup_retargeting.py    (IK Rig 재생성)")
