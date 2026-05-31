"""
Ashen Ossuary — ObsidianKnight 리타게팅 셋업 (UE5.7 수정판)
에이전트 E 작성

실행: UE Editor > Tools > Execute Python Script > 이 파일 선택

전략 A: SK_Mannequin 의 Skeleton 을 SK_GK_ObsidianKnight 에 직접 재할당
         → 본 이름이 동일하므로 성공 시 Mannequin 애니 즉시 사용 가능
전략 B: 전략 A 실패 시 IK Rig + IK Retargeter 수동 생성 안내
"""

import unreal

# SK_Mannequin 은 USkeleton 에셋 — 실제 SkeletalMesh 는 SKM_Manny_Simple
MANNEQUIN_MESH_PATH   = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
OBSIDIAN_MESH_PATH    = "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight"
PLAYER_MESH_DIR       = "/Game/Characters/Player/Mesh"

unreal.log("=" * 60)
unreal.log("  Ashen Ossuary — ObsidianKnight 리타게팅 셋업 시작")
unreal.log("=" * 60)


# ── 전략 A: 스켈레톤 직접 재할당 ──────────────────────────────────────
unreal.log("[GK Retarget] ── 전략 A: 스켈레톤 직접 재할당 시도 ──")

strategy_a_ok = False

mannequin_mesh = unreal.EditorAssetLibrary.load_asset(MANNEQUIN_MESH_PATH)
obsidian_mesh  = unreal.EditorAssetLibrary.load_asset(OBSIDIAN_MESH_PATH)

if not mannequin_mesh:
    unreal.log_warning(f"[GK Retarget] ✗ Mannequin 메시 없음: {MANNEQUIN_MESH_PATH}")
elif not isinstance(mannequin_mesh, unreal.SkeletalMesh):
    unreal.log_warning(f"[GK Retarget] ✗ {MANNEQUIN_MESH_PATH} 이 SkeletalMesh 가 아님")
elif not obsidian_mesh:
    unreal.log_warning(f"[GK Retarget] ✗ ObsidianKnight 메시 없음: {OBSIDIAN_MESH_PATH}")
    unreal.log_warning("[GK Retarget]   import_meshy_assets.py 를 먼저 실행하세요.")
elif not isinstance(obsidian_mesh, unreal.SkeletalMesh):
    unreal.log_warning(f"[GK Retarget] ✗ {OBSIDIAN_MESH_PATH} 이 SkeletalMesh 가 아님")
else:
    mannequin_skeleton = mannequin_mesh.skeleton
    if not mannequin_skeleton:
        unreal.log_warning("[GK Retarget] ✗ Mannequin 에서 Skeleton 추출 실패")
    else:
        unreal.log(f"[GK Retarget]   Mannequin Skeleton: {mannequin_skeleton.get_path_name()}")
        try:
            # 스켈레톤 재할당
            unreal.EditorAssetLibrary.checkout_loaded_asset(obsidian_mesh)
            obsidian_mesh.set_editor_property("skeleton", mannequin_skeleton)
            unreal.EditorAssetLibrary.save_loaded_asset(obsidian_mesh)
            strategy_a_ok = True
            unreal.log("[GK Retarget] ✓ 전략 A 성공 — ObsidianKnight 가 Mannequin 애니를 직접 사용합니다.")
            unreal.log("[GK Retarget]   Content > Characters > Player > Mesh > SK_GK_ObsidianKnight")
            unreal.log("[GK Retarget]   를 열어 미리보기에서 Mannequin 애니 재생 가능 여부 확인하세요.")
        except Exception as e:
            unreal.log_warning(f"[GK Retarget] ✗ 전략 A 실패: {e}")


# ── 전략 B: IK Rig 생성 (전략 A 실패 시) ────────────────────────────
if not strategy_a_ok:
    unreal.log("[GK Retarget] ── 전략 B: IK Rig 생성 (UE5.7 API) ──")

    def create_ik_rig(asset_name, skeletal_mesh):
        full_path = f"{PLAYER_MESH_DIR}/{asset_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(full_path):
            unreal.EditorAssetLibrary.delete_asset(full_path)

        # IKRigDefinition 에셋 직접 생성
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        ik_rig = asset_tools.create_asset(
            asset_name, PLAYER_MESH_DIR,
            unreal.IKRigDefinition,
            unreal.IKRigDefinitionFactory()
        )
        if not ik_rig:
            unreal.log_warning(f"[GK Retarget] ✗ {asset_name} 생성 실패")
            return None

        controller = unreal.IKRigController.get_controller(ik_rig)
        if not controller:
            unreal.log_warning(f"[GK Retarget] ✗ {asset_name} Controller 없음")
            return None

        # 프리뷰 메시 설정 — SkeletalMesh 타입만 허용
        if isinstance(skeletal_mesh, unreal.SkeletalMesh):
            try:
                controller.set_skeletal_mesh(skeletal_mesh)
            except Exception:
                try:
                    ik_rig.set_editor_property("preview_skeletal_mesh", skeletal_mesh)
                except Exception as e2:
                    unreal.log_warning(f"[GK Retarget]   프리뷰 메시 설정 실패 (무시): {e2}")
        else:
            unreal.log_warning(f"[GK Retarget]   프리뷰 메시가 SkeletalMesh 가 아님 — 건너뜀")

        # 본 체인 등록 (UE5.7: goal_name 인수 필요)
        chains = [
            ("Root",     "root",       "root"),
            ("Pelvis",   "pelvis",     "pelvis"),
            ("Spine",    "spine_01",   "spine_03"),
            ("Head",     "neck_01",    "head"),
            ("LeftArm",  "clavicle_l", "hand_l"),
            ("RightArm", "clavicle_r", "hand_r"),
            ("LeftLeg",  "thigh_l",    "ball_l"),
            ("RightLeg", "thigh_r",    "ball_r"),
        ]
        registered = 0
        for chain_name, start, end in chains:
            try:
                controller.add_retarget_chain(chain_name, start, end, "")
                registered += 1
            except Exception as e:
                unreal.log_warning(f"[GK Retarget]   체인 [{chain_name}] 실패: {e}")

        unreal.EditorAssetLibrary.save_loaded_asset(ik_rig)
        unreal.log(f"[GK Retarget] ✓ {asset_name} — 체인 {registered}/{len(chains)}개 등록")
        return ik_rig

    manny_mesh_obj    = unreal.EditorAssetLibrary.load_asset(MANNEQUIN_MESH_PATH)
    obsidian_mesh_obj = unreal.EditorAssetLibrary.load_asset(OBSIDIAN_MESH_PATH)

    ik_mannequin = None
    ik_obsidian  = None

    if manny_mesh_obj:
        ik_mannequin = create_ik_rig("IK_Mannequin", manny_mesh_obj)
    if obsidian_mesh_obj:
        ik_obsidian  = create_ik_rig("IK_GK_ObsidianKnight", obsidian_mesh_obj)

    # IK Retargeter 생성
    rtg_ok = False
    if ik_mannequin and ik_obsidian:
        unreal.log("[GK Retarget] ── IK Retargeter 생성 ──")
        rtg_path = f"{PLAYER_MESH_DIR}/RTG_Mannequin_ObsidianKnight"
        try:
            if unreal.EditorAssetLibrary.does_asset_exist(rtg_path):
                unreal.EditorAssetLibrary.delete_asset(rtg_path)

            asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
            # UE5.7 팩토리 이름 탐색
            factory = None
            for cls_name in ["IKRetargeterFactory", "IKRigRetargeterFactory"]:
                try:
                    factory_cls = getattr(unreal, cls_name)
                    factory = factory_cls()
                    break
                except AttributeError:
                    continue

            if factory:
                rtg = asset_tools.create_asset(
                    "RTG_Mannequin_ObsidianKnight", PLAYER_MESH_DIR,
                    unreal.IKRetargeter, factory
                )
                if rtg:
                    controller = unreal.IKRetargeterController.get_controller(rtg)
                    if controller:
                        controller.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, ik_mannequin)
                        controller.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, ik_obsidian)
                    unreal.EditorAssetLibrary.save_loaded_asset(rtg)
                    rtg_ok = True
                    unreal.log(f"[GK Retarget] ✓ RTG_Mannequin_ObsidianKnight 생성 완료")
            else:
                unreal.log_warning("[GK Retarget] ✗ IKRetargeterFactory 없음 — 수동 생성 필요")
        except Exception as e:
            unreal.log_warning(f"[GK Retarget] ✗ Retargeter 생성 실패: {e}")

    # 전략 B 실패 시 수동 안내
    if not rtg_ok:
        unreal.log("")
        unreal.log("[GK Retarget] ★ 수동 작업 안내 (2분 소요) ★")
        unreal.log("  1. Content Browser > Characters > Player > Mesh")
        unreal.log("  2. 우클릭 > Animation > IK Retargeter")
        unreal.log("  3. Source IK Rig: IK_Mannequin 선택")
        unreal.log("  4. Target IK Rig: IK_GK_ObsidianKnight 선택")
        unreal.log("  5. 저장 후 'Retarget Animation Assets' 버튼 클릭")
        unreal.log("  6. 원하는 Mannequin 애니 선택 → Export")


# ── 결과 요약 ──────────────────────────────────────────────────────────
unreal.log("=" * 60)
unreal.log("  결과 요약")
unreal.log("=" * 60)
unreal.log(f"  전략 A (스켈레톤 직접 재할당): {'✓ 성공' if strategy_a_ok else '✗ 실패'}")
if not strategy_a_ok:
    unreal.log(f"  IK_Mannequin              : {'✓' if 'ik_mannequin' in dir() and ik_mannequin else '✗'}")
    unreal.log(f"  IK_GK_ObsidianKnight      : {'✓' if 'ik_obsidian' in dir() and ik_obsidian else '✗'}")
unreal.log("=" * 60)
