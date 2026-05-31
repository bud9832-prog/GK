"""
Retarget Root + 높이 오프셋 일괄 수정 스크립트
UE Editor > Tools > Execute Python Script > 이 파일 선택

setup_retargeting.py 실행 후, 공중 부양/루트 본 None 문제를 자동으로 수정합니다.
"""
import unreal

IK_KNIGHT_PATH   = "/Game/KnightAnimation/Demo/DemoCharacters/IK_Knight_Base"
IK_OBSIDIAN_PATH = "/Game/Characters/Player/Mesh/IK_GK_ObsidianKnight"
RTG_PATH         = "/Game/Characters/Player/Mesh/RTG_Knight_ObsidianKnight"

unreal.log("=" * 55)
unreal.log("  Retarget Root 픽스 시작")
unreal.log("=" * 55)

# ── 1. IK_Knight_Base — Retarget Root = root ──────────────
ik_knight = unreal.EditorAssetLibrary.load_asset(IK_KNIGHT_PATH)
if ik_knight:
    ctrl = unreal.IKRigController.get_controller(ik_knight)
    try:
        result = ctrl.set_retarget_root("root")
        unreal.log(f"[Fix] ✓ IK_Knight_Base  Retarget Root = root  (결과: {result})")
    except Exception as e:
        unreal.log_warning(f"[Fix] ✗ IK_Knight_Base set_retarget_root 실패: {e}")
    unreal.EditorAssetLibrary.save_loaded_asset(ik_knight)
else:
    unreal.log_warning(f"[Fix] IK_Knight_Base 로드 실패: {IK_KNIGHT_PATH}")
    unreal.log_warning("[Fix]   setup_retargeting.py 를 먼저 실행하세요.")

# ── 2. IK_GK_ObsidianKnight — Retarget Root = Hips ───────
ik_obsidian = unreal.EditorAssetLibrary.load_asset(IK_OBSIDIAN_PATH)
if ik_obsidian:
    ctrl = unreal.IKRigController.get_controller(ik_obsidian)
    try:
        result = ctrl.set_retarget_root("Hips")
        unreal.log(f"[Fix] ✓ IK_GK_ObsidianKnight  Retarget Root = Hips  (결과: {result})")
    except Exception as e:
        unreal.log_warning(f"[Fix] ✗ IK_GK_ObsidianKnight set_retarget_root 실패: {e}")
    unreal.EditorAssetLibrary.save_loaded_asset(ik_obsidian)
else:
    unreal.log_warning(f"[Fix] IK_GK_ObsidianKnight 로드 실패: {IK_OBSIDIAN_PATH}")

# ── 3. RTG — 높이 오프셋 자동 계산 후 적용 ───────────────
# Asset Registry 강제 스캔 후 로드
ar = unreal.AssetRegistryHelpers.get_asset_registry()
ar.scan_paths_synchronous(["/Game/Characters/Player/Mesh"], True)
rtg = unreal.EditorAssetLibrary.load_asset(RTG_PATH)
if rtg:
    try:
        rc = unreal.IKRetargeterController.get_controller(rtg)

        # 소스/타겟 IK Rig 재연결
        if ik_knight:
            rc.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, ik_knight)
            unreal.log("[Fix] ✓ RTG Source IK Rig = IK_Knight_Base")
        if ik_obsidian:
            rc.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, ik_obsidian)
            unreal.log("[Fix] ✓ RTG Target IK Rig = IK_GK_ObsidianKnight")

        # 타겟 리타깃 포즈 — Hips Z 오프셋 보정
        # Mixamo Hips 는 T-포즈에서 약 90cm 높이 → 소스 root(0) 대비 오프셋 제거
        try:
            pose_name = rc.get_current_retarget_pose_name(unreal.RetargetSourceOrTarget.TARGET)
            zero_transform = unreal.Transform(
                location=unreal.Vector(0.0, 0.0, 0.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0)
            )
            rc.set_retarget_pose_for_bone(
                unreal.RetargetSourceOrTarget.TARGET,
                pose_name, "Hips", zero_transform
            )
            unreal.log("[Fix] ✓ RTG Target Hips 포즈 오프셋 리셋")
        except Exception as e:
            unreal.log_warning(f"[Fix]   포즈 오프셋 자동 설정 실패 (무시): {e}")

        unreal.EditorAssetLibrary.save_loaded_asset(rtg)
        unreal.log("[Fix] ✓ RTG 저장 완료")

    except Exception as e:
        unreal.log_warning(f"[Fix] RTG 수정 실패: {e}")
else:
    unreal.log_warning(f"[Fix] RTG 로드 실패: {RTG_PATH}")
    unreal.log_warning("[Fix]   RTG 저장 이름이 다를 수 있습니다. Content Browser에서 확인하세요.")

unreal.log("=" * 55)
unreal.log("  완료. RTG_Knight_ObsidianKnight 를 닫았다가 다시 여세요.")
unreal.log("=" * 55)
