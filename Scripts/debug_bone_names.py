"""
본 이름 진단 스크립트 — 리타게팅 루트 본 설정용
UE Editor > Tools > Execute Python Script > 이 파일 선택
"""
import unreal

KNIGHT_SKEL_PATH   = "/Game/KnightAnimation/Demo/DemoCharacters/SKEL_Knight_Base"
OBSIDIAN_MESH_PATH = "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight"

def print_bones(asset_path, label):
    asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    if not asset:
        unreal.log_warning(f"[디버그] 로드 실패: {asset_path}")
        return

    skeleton = None
    if isinstance(asset, unreal.SkeletalMesh):
        skeleton = asset.skeleton
    elif isinstance(asset, unreal.Skeleton):
        skeleton = asset

    if not skeleton:
        unreal.log_warning(f"[디버그] 스켈레톤 없음: {asset_path}")
        return

    unreal.log(f"\n{'='*50}")
    unreal.log(f"  {label} 본 목록")
    unreal.log(f"{'='*50}")

    ref_skel = skeleton.get_editor_property("reference_skeleton") if hasattr(skeleton, "get_editor_property") else None

    # 방법 1: ReferenceSkeleton API
    try:
        num = skeleton.get_num_bones()
        for i in range(num):
            name = skeleton.get_bone_name(i)
            parent_idx = skeleton.get_parent_bone(name)
            indent = "  " if i > 0 else ""
            unreal.log(f"  [{i:02d}] {indent}{name}")
        return
    except Exception:
        pass

    # 방법 2: SkeletalMesh bones
    try:
        if isinstance(asset, unreal.SkeletalMesh):
            bones = unreal.EditorSkeletalMeshLibrary.get_lod_info(asset, 0)
            unreal.log(f"  LOD 정보: {bones}")
    except Exception as e:
        unreal.log(f"  [방법2 실패] {e}")

    unreal.log(f"  → 본 이름 직접 확인: Content Browser에서 {label} 더블클릭 > Skeleton Tree")

print_bones(KNIGHT_SKEL_PATH, "Knight (SKEL_Knight_Base)")
print_bones(OBSIDIAN_MESH_PATH, "ObsidianKnight (SK_GK_ObsidianKnight)")

unreal.log("\n[안내] 위 본 목록에서:")
unreal.log("  - Knight의 첫 번째 본(인덱스 00) → IK_Knight_Base Retarget Root 에 입력")
unreal.log("  - ObsidianKnight의 첫 번째 본    → IK_GK_ObsidianKnight Retarget Root 에 입력")
