"""
Ashen Ossuary — ObsidianKnight 리타게팅 셋업 (KnightAnimation 팩 기준)
에이전트 E 작성

소스: Rig_Knight_Base (이미 존재, KnightAnimation 팩 전용 IK Rig)
타겟: IK_GK_ObsidianKnight (Mixamo 본 이름으로 신규 생성)

완료 시:
  - 콤보 1~5타, 패링, 가드, 달리기, 점프, 사망 등
    Anim_Knight_* 전체를 ObsidianKnight 에 리타게팅 가능

실행 순서:
  1. scripts/import_meshy_assets.py  (SK_GK_ObsidianKnight Mixamo 임포트)
  2. 이 파일
  3. Content Browser 수동 1단계:
     Characters > Player > Mesh 우클릭
     → Animation → IK Retargeter
     → Source: Rig_Knight_Base / 저장: RTG_Knight_ObsidianKnight
     → Target IK Rig: IK_GK_ObsidianKnight
     → 에셋 리타깃 → Anim_Knight_* 전체 선택 → Export
"""

import unreal

KNIGHT_RIG_PATH    = "/Game/KnightAnimation/Demo/DemoCharacters/Rig_Knight_Base"
KNIGHT_SKEL_PATH   = "/Game/KnightAnimation/Demo/DemoCharacters/SKEL_Knight_Base"
OBSIDIAN_MESH_PATH = "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight"
PLAYER_MESH_DIR    = "/Game/Characters/Player/Mesh"

unreal.log("=" * 60)
unreal.log("  ObsidianKnight 리타게팅 셋업 (KnightAnimation 기준)")
unreal.log("=" * 60)

# ── 소스 IK Rig 확인 ─────────────────────────────────────────────────
knight_rig = unreal.EditorAssetLibrary.load_asset(KNIGHT_RIG_PATH)
if knight_rig:
    unreal.log(f"[GK Retarget] ✓ 소스 IK Rig 확인: Rig_Knight_Base")
else:
    unreal.log_warning(f"[GK Retarget] ✗ Rig_Knight_Base 없음: {KNIGHT_RIG_PATH}")
    unreal.log_warning("[GK Retarget]   KnightAnimation 팩이 임포트되어 있는지 확인하세요.")

# ── 타겟 IK Rig 생성 (Mixamo ObsidianKnight) ─────────────────────────
unreal.log("[GK Retarget] IK_GK_ObsidianKnight 생성 중 (Mixamo 본)...")

IK_TARGET_PATH = f"{PLAYER_MESH_DIR}/IK_GK_ObsidianKnight"
if unreal.EditorAssetLibrary.does_asset_exist(IK_TARGET_PATH):
    unreal.EditorAssetLibrary.delete_asset(IK_TARGET_PATH)

obsidian_mesh = unreal.EditorAssetLibrary.load_asset(OBSIDIAN_MESH_PATH)
if not obsidian_mesh or not isinstance(obsidian_mesh, unreal.SkeletalMesh):
    unreal.log_warning(f"[GK Retarget] ✗ SK_GK_ObsidianKnight SkeletalMesh 없음")
    unreal.log_warning("[GK Retarget]   scripts/import_meshy_assets.py 를 먼저 실행하세요.")
else:
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    ik_target = asset_tools.create_asset(
        "IK_GK_ObsidianKnight", PLAYER_MESH_DIR,
        unreal.IKRigDefinition,
        unreal.IKRigDefinitionFactory()
    )

    if ik_target:
        ctrl = unreal.IKRigController.get_controller(ik_target)

        # 프리뷰 메시 설정
        try:
            ctrl.set_skeletal_mesh(obsidian_mesh)
        except Exception:
            try:
                ik_target.set_editor_property("preview_skeletal_mesh", obsidian_mesh)
            except Exception as e:
                unreal.log_warning(f"[GK Retarget]   프리뷰 메시 설정 실패 (무시): {e}")

        # Mixamo 본 이름 기준 체인 등록
        # 소스(Knight)와 체인 이름을 일치시켜야 자동 매핑됨
        MIXAMO_CHAINS = [
            ("Root",       "Hips",          "Hips"),
            ("Pelvis",     "Hips",          "Hips"),
            ("Spine",      "Spine",         "Spine2"),
            ("Neck",       "Neck",          "Neck"),
            ("Head",       "Head",          "Head"),
            ("LeftArm",    "LeftShoulder",  "LeftHand"),
            ("RightArm",   "RightShoulder", "RightHand"),
            ("LeftLeg",    "LeftUpLeg",     "LeftFoot"),
            ("RightLeg",   "RightUpLeg",    "RightFoot"),
        ]

        ok = 0
        for chain_name, start, end in MIXAMO_CHAINS:
            try:
                ctrl.add_retarget_chain(chain_name, start, end, "")
                ok += 1
            except Exception as e:
                unreal.log_warning(f"[GK Retarget]   체인 [{chain_name}] 실패: {e}")

        unreal.EditorAssetLibrary.save_loaded_asset(ik_target)
        unreal.log(f"[GK Retarget] ✓ IK_GK_ObsidianKnight — 체인 {ok}/{len(MIXAMO_CHAINS)}개 등록")

    # ── IK Retargeter 자동 생성 시도 ─────────────────────────────────
    rtg_ok = False
    if ik_target and knight_rig:
        RTG_PATH = f"{PLAYER_MESH_DIR}/RTG_Knight_ObsidianKnight"
        try:
            if unreal.EditorAssetLibrary.does_asset_exist(RTG_PATH):
                unreal.EditorAssetLibrary.delete_asset(RTG_PATH)
            factory = None
            for fname in ["IKRetargeterFactory", "IKRigRetargeterFactory"]:
                try:
                    factory = getattr(unreal, fname)()
                    break
                except AttributeError:
                    pass
            if factory:
                rtg = asset_tools.create_asset(
                    "RTG_Knight_ObsidianKnight", PLAYER_MESH_DIR,
                    unreal.IKRetargeter, factory
                )
                if rtg:
                    rc = unreal.IKRetargeterController.get_controller(rtg)
                    if rc:
                        rc.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, knight_rig)
                        rc.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, ik_target)
                    unreal.EditorAssetLibrary.save_loaded_asset(rtg)
                    rtg_ok = True
                    unreal.log("[GK Retarget] ✓ RTG_Knight_ObsidianKnight 자동 생성!")
        except Exception as e:
            unreal.log_warning(f"[GK Retarget] Retargeter 자동 생성 실패: {e}")

    if not rtg_ok:
        unreal.log("")
        unreal.log("[GK Retarget] ★ 수동 1단계 (2분) ★")
        unreal.log("  Content Browser > Characters > Player > Mesh")
        unreal.log("  우클릭 → Animation → IK Retargeter")
        unreal.log("  → Source IK Rig: Rig_Knight_Base")
        unreal.log("  → 저장 이름: RTG_Knight_ObsidianKnight")
        unreal.log("  열리면 Target 패널에서 IK_GK_ObsidianKnight 선택")
        unreal.log("  → 상단 '에셋 리타깃' → Anim_Knight_* 전체 선택 → Export")
        unreal.log("    저장 위치: /Game/Characters/Player/Animations/")

unreal.log("=" * 60)
unreal.log("  리타게팅 완료 후 사용 가능한 애니메이션:")
unreal.log("  BasicAttack_01~05 / Parrying / Guard / Die")
unreal.log("  Idle / Walk / Run / Sprint / Jump / Fall")
unreal.log("  Hit_Stand_* / Groggy / Skill_Slash·Smash·Stab 등")
unreal.log("=" * 60)
