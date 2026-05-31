"""
Ashen Ossuary — ObsidianKnight 리타게팅 셋업 (KnightAnimation 팩 기준)
에이전트 E 작성

소스: IK_Knight_Base (SKEL_Knight_Base 기반으로 신규 생성)
      ※ Rig_Knight_Base 는 Control Rig 타입이므로 IK Retargeter 에서 사용 불가
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
     → Source IK Rig: IK_Knight_Base  ← (Rig_Knight_Base 아님!)
     저장: RTG_Knight_ObsidianKnight
     → Target IK Rig: IK_GK_ObsidianKnight
     → 에셋 리타깃 → Anim_Knight_* 전체 선택 → Export
"""

import unreal

KNIGHT_SKEL_PATH   = "/Game/KnightAnimation/Demo/DemoCharacters/SKEL_Knight_Base"
OBSIDIAN_MESH_PATH = "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight"
PLAYER_MESH_DIR    = "/Game/Characters/Player/Mesh"
KNIGHT_IK_DIR      = "/Game/KnightAnimation/Demo/DemoCharacters"

unreal.log("=" * 60)
unreal.log("  ObsidianKnight 리타게팅 셋업 (KnightAnimation 기준)")
unreal.log("=" * 60)

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# ── 소스 IK Rig 생성 (SKEL_Knight_Base → IK_Knight_Base) ─────────────
IK_KNIGHT_PATH = f"{KNIGHT_IK_DIR}/IK_Knight_Base"
if unreal.EditorAssetLibrary.does_asset_exist(IK_KNIGHT_PATH):
    unreal.EditorAssetLibrary.delete_asset(IK_KNIGHT_PATH)

knight_skel_asset = unreal.EditorAssetLibrary.load_asset(KNIGHT_SKEL_PATH)

# SKEL_Knight_Base 가 SkeletalMesh 인지 Skeleton 인지 확인
if isinstance(knight_skel_asset, unreal.SkeletalMesh):
    knight_mesh = knight_skel_asset
    unreal.log("[GK Retarget] SKEL_Knight_Base = SkeletalMesh")
elif isinstance(knight_skel_asset, unreal.Skeleton):
    knight_mesh = None
    unreal.log("[GK Retarget] SKEL_Knight_Base = Skeleton (프리뷰 메시 미설정)")
else:
    knight_mesh = None
    unreal.log_warning(f"[GK Retarget] SKEL_Knight_Base 로드 실패: {KNIGHT_SKEL_PATH}")

ik_knight = asset_tools.create_asset(
    "IK_Knight_Base", KNIGHT_IK_DIR,
    unreal.IKRigDefinition,
    unreal.IKRigDefinitionFactory()
)

knight_rig = None
if ik_knight:
    ctrl = unreal.IKRigController.get_controller(ik_knight)

    if knight_mesh:
        try:
            ctrl.set_skeletal_mesh(knight_mesh)
        except Exception:
            try:
                ik_knight.set_editor_property("preview_skeletal_mesh", knight_mesh)
            except Exception as e:
                unreal.log_warning(f"[GK Retarget]   Knight 프리뷰 메시 설정 실패: {e}")

    # UE5 Mannequin 호환 본 이름 기준 체인
    # Knight 팩이 Mannequin 호환 네이밍을 쓰는 경우 자동 매핑됨
    KNIGHT_CHAINS = [
        ("Root",           "root",        "root"),
        ("Pelvis",         "pelvis",      "pelvis"),
        ("Spine",          "spine_01",    "spine_03"),
        ("Neck",           "neck_01",     "neck_01"),
        ("Head",           "head",        "head"),
        ("LeftClavicle",   "clavicle_l",  "clavicle_l"),
        ("LeftArm",        "upperarm_l",  "hand_l"),
        ("RightClavicle",  "clavicle_r",  "clavicle_r"),
        ("RightArm",       "upperarm_r",  "hand_r"),
        ("LeftLeg",        "thigh_l",     "foot_l"),
        ("RightLeg",       "thigh_r",     "foot_r"),
    ]

    ok = 0
    for chain_name, start, end in KNIGHT_CHAINS:
        try:
            ctrl.add_retarget_chain(chain_name, start, end, "")
            ok += 1
        except Exception as e:
            unreal.log_warning(f"[GK Retarget]   Knight 체인 [{chain_name}] 실패: {e}")

    unreal.EditorAssetLibrary.save_loaded_asset(ik_knight)
    knight_rig = ik_knight
    unreal.log(f"[GK Retarget] ✓ IK_Knight_Base 생성 — 체인 {ok}/{len(KNIGHT_CHAINS)}개")
    if ok < len(KNIGHT_CHAINS):
        unreal.log("[GK Retarget]   ※ 실패한 체인은 SKEL_Knight_Base 의 실제 본 이름을 확인 후")
        unreal.log("[GK Retarget]     에디터에서 IK_Knight_Base 를 열고 수동으로 추가하세요.")
else:
    unreal.log_warning("[GK Retarget] ✗ IK_Knight_Base 생성 실패")

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
        unreal.log("  → Source IK Rig: IK_Knight_Base  ← ★ Rig_Knight_Base 아님!")
        unreal.log("    위치: KnightAnimation/Demo/DemoCharacters/IK_Knight_Base")
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
