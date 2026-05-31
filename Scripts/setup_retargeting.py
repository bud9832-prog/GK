"""
Ashen Ossuary — SK_GK_ObsidianKnight 애니메이션 리타게팅 셋업
에이전트 E | 2026-05-31

실행 방법: UE Editor > Tools > Execute Python Script > 이 파일 선택

목적:
  SK_GK_ObsidianKnight (본 이름 22개가 UE5 Mannequin과 완전 동일)가
  기존 Mannequin 애니메이션을 사용할 수 있도록 설정합니다.

  전략 A — 직접 스켈레톤 재할당 (권장, 본 이름 동일이므로 가능):
    SK_GK_ObsidianKnight 스켈레탈 메시의 스켈레톤을
    SK_Mannequin_Skeleton 으로 교체합니다.
    → 별도 리타게터 없이 모든 Mannequin 애니메이션을 즉시 사용 가능.

  전략 B — IK Rig + IK Retargeter 생성 (전략 A 실패 시 또는 추가 옵션):
    IK_Mannequin         ← 소스 IK Rig (Mannequin 스켈레톤)
    IK_GK_ObsidianKnight ← 타겟 IK Rig (ObsidianKnight 스켈레톤)
    RTG_Mannequin_ObsidianKnight ← IK Retargeter

  출력 경로: /Game/Characters/Player/Mesh/

  본 이름 (Mannequin ↔ ObsidianKnight 1:1 동일):
    root / pelvis / spine_01~03 / neck_01 / head
    clavicle_l/r / upperarm_l/r / lowerarm_l/r / hand_l/r
    thigh_l/r / calf_l/r / foot_l/r / ball_l/r
"""

import unreal

# ─────────────────────────────────────────────────────────────
#  경로 설정
# ─────────────────────────────────────────────────────────────

# Mannequin 에셋 경로 — SK_Mannequin.uasset 과 함께 자동 생성된 _Skeleton
MANNEQUIN_MESH_PATH = "/Game/Characters/Mannequins/Meshes/SK_Mannequin"
MANNEQUIN_SKEL_PATH = "/Game/Characters/Mannequins/Meshes/SK_Mannequin_Skeleton"

# ObsidianKnight 에셋 경로 (import_meshy_assets.py 또는 직접 임포트 후 생성)
OBSIDIAN_MESH_PATH  = "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight"
OBSIDIAN_SKEL_PATH  = "/Game/Characters/Player/Mesh/SK_GK_ObsidianKnight_Skeleton"

OUTPUT_PATH         = "/Game/Characters/Player/Mesh"

# IK Rig / Retargeter 저장 경로
IK_MANNEQUIN_PATH      = f"{OUTPUT_PATH}/IK_Mannequin"
IK_OBSIDIAN_PATH       = f"{OUTPUT_PATH}/IK_GK_ObsidianKnight"
RETARGETER_PATH        = f"{OUTPUT_PATH}/RTG_Mannequin_ObsidianKnight"

# ─────────────────────────────────────────────────────────────
#  리타겟 체인 정의 (Mannequin 표준 — ObsidianKnight와 동일)
#  형식: (체인_이름, 시작_본, 끝_본)
# ─────────────────────────────────────────────────────────────
RETARGET_CHAINS = [
    ("Root",      "root",       "root"),
    ("Pelvis",    "pelvis",     "pelvis"),
    ("Spine",     "spine_01",   "spine_03"),
    ("Head",      "neck_01",    "head"),
    ("LeftArm",   "clavicle_l", "hand_l"),
    ("RightArm",  "clavicle_r", "hand_r"),
    ("LeftLeg",   "thigh_l",    "foot_l"),
    ("RightLeg",  "thigh_r",    "foot_r"),
]


# ─────────────────────────────────────────────────────────────
#  헬퍼
# ─────────────────────────────────────────────────────────────

def _load(path: str, label: str):
    """에셋 로드 + 존재 여부 확인. 없으면 None 반환."""
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.log_warning(f"[GK Retarget] ✗ {label} 없음: {path}")
        return None
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        unreal.log_warning(f"[GK Retarget] ✗ {label} 로드 실패: {path}")
    return asset


def _create_or_load(asset_name: str, package_path: str, asset_class, factory):
    """에셋이 이미 있으면 로드, 없으면 새로 생성."""
    full_path = f"{package_path}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(full_path):
        unreal.log(f"[GK Retarget]   기존 에셋 로드: {full_path}")
        return unreal.EditorAssetLibrary.load_asset(full_path)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, package_path, asset_class, factory
    )
    if asset:
        unreal.log(f"[GK Retarget]   ✓ 생성: {full_path}")
    else:
        unreal.log_warning(f"[GK Retarget]   ✗ 생성 실패: {full_path}")
    return asset


# ─────────────────────────────────────────────────────────────
#  전략 A — 직접 스켈레톤 재할당
#  본 이름이 완전 동일하므로 ObsidianKnight SK 메시가
#  Mannequin 스켈레톤을 직접 사용하도록 변경합니다.
# ─────────────────────────────────────────────────────────────

def strategy_a_reassign_skeleton() -> bool:
    """
    SK_GK_ObsidianKnight 의 스켈레톤을 SK_Mannequin_Skeleton 으로 교체합니다.
    성공 시 True, 실패 시 False.
    성공하면 별도 IK Retargeter 없이 Mannequin 애니메이션을 바로 사용 가능합니다.
    """
    unreal.log("[GK Retarget] ── 전략 A: 직접 스켈레톤 재할당 시도 ──")

    obsidian_mesh = _load(OBSIDIAN_MESH_PATH, "SK_GK_ObsidianKnight")
    mannequin_skel = _load(MANNEQUIN_SKEL_PATH, "SK_Mannequin_Skeleton")

    if not obsidian_mesh or not mannequin_skel:
        unreal.log_warning("[GK Retarget] 전략 A 전제 에셋 없음 → 건너뜀")
        return False

    if not isinstance(obsidian_mesh, unreal.SkeletalMesh):
        unreal.log_warning("[GK Retarget] SK_GK_ObsidianKnight 가 SkeletalMesh 가 아님")
        return False

    try:
        # 스켈레톤 재할당 — 본 이름이 동일하므로 UE5 가 그대로 수용합니다
        obsidian_mesh.set_editor_property("skeleton", mannequin_skel)
        unreal.EditorAssetLibrary.save_asset(OBSIDIAN_MESH_PATH)
        unreal.log(
            "[GK Retarget] ✓ 전략 A 성공: SK_GK_ObsidianKnight 스켈레톤 → SK_Mannequin_Skeleton\n"
            "              이제 모든 Mannequin 애니메이션을 바로 사용할 수 있습니다."
        )
        return True
    except Exception as e:
        unreal.log_warning(f"[GK Retarget] 전략 A 실패 ({e}) → 전략 B 로 전환")
        return False


# ─────────────────────────────────────────────────────────────
#  전략 B — IK Rig 생성
# ─────────────────────────────────────────────────────────────

def _build_ik_rig(ik_rig_name: str, sk_mesh_path: str) -> "unreal.IKRigDefinition | None":
    """
    지정 스켈레탈 메시에 대한 IK Rig 에셋을 생성하고
    Mannequin 표준 리타겟 체인을 등록합니다.
    """
    sk_mesh = _load(sk_mesh_path, ik_rig_name + " (SK mesh)")
    if not sk_mesh:
        return None

    try:
        factory = unreal.IKRigDefinitionFactory()
    except AttributeError:
        unreal.log_warning(
            "[GK Retarget] IKRigDefinitionFactory 를 찾을 수 없습니다.\n"
            "              UE 5.x Python 바인딩을 확인하세요 (IKRig 플러그인 활성화 필요)."
        )
        return None

    ik_rig = _create_or_load(ik_rig_name, OUTPUT_PATH, unreal.IKRigDefinition, factory)
    if not ik_rig:
        return None

    try:
        controller = unreal.IKRigController.get_controller(ik_rig)
    except Exception as e:
        unreal.log_warning(f"[GK Retarget] IKRigController 획득 실패: {e}")
        return None

    # 프리뷰 메시 설정 (스켈레톤 연결)
    try:
        controller.set_preview_skeletal_mesh(sk_mesh)
    except Exception as e:
        unreal.log_warning(f"[GK Retarget] set_preview_skeletal_mesh 실패: {e}")

    # 리타겟 루트 지정
    try:
        controller.set_retarget_root("root")
    except Exception as e:
        unreal.log_warning(f"[GK Retarget] set_retarget_root 실패: {e}")

    # 리타겟 체인 등록
    for chain_name, start_bone, end_bone in RETARGET_CHAINS:
        if chain_name == "Root":
            continue  # 루트는 set_retarget_root 로 이미 처리
        try:
            controller.add_retarget_chain(chain_name, start_bone, end_bone)
            unreal.log(f"[GK Retarget]   체인 등록: {chain_name} ({start_bone} → {end_bone})")
        except Exception as e:
            unreal.log_warning(f"[GK Retarget]   체인 등록 실패 [{chain_name}]: {e}")

    unreal.EditorAssetLibrary.save_asset(f"{OUTPUT_PATH}/{ik_rig_name}")
    unreal.log(f"[GK Retarget] ✓ IK Rig 완료: {ik_rig_name}")
    return ik_rig


def strategy_b_create_ik_rigs() -> "tuple[unreal.IKRigDefinition, unreal.IKRigDefinition] | tuple[None, None]":
    """Mannequin 소스 + ObsidianKnight 타겟 IK Rig 를 생성합니다."""
    unreal.log("[GK Retarget] ── 전략 B: IK Rig 생성 ──")

    source_ik = _build_ik_rig("IK_Mannequin",         MANNEQUIN_MESH_PATH)
    target_ik = _build_ik_rig("IK_GK_ObsidianKnight", OBSIDIAN_MESH_PATH)

    return source_ik, target_ik


# ─────────────────────────────────────────────────────────────
#  전략 B — IK Retargeter 생성
# ─────────────────────────────────────────────────────────────

def create_retargeter(
    source_ik: "unreal.IKRigDefinition",
    target_ik:  "unreal.IKRigDefinition",
) -> bool:
    """
    source_ik → target_ik 를 연결하는 IK Retargeter 를 생성합니다.
    본 이름이 동일하므로 체인 매핑이 자동으로 1:1 매칭됩니다.
    """
    unreal.log("[GK Retarget] ── IK Retargeter 생성 ──")

    try:
        factory = unreal.IKRetargeterFactory()
    except AttributeError:
        unreal.log_warning(
            "[GK Retarget] IKRetargeterFactory 를 찾을 수 없습니다.\n"
            "              IKRig 플러그인이 활성화되어 있는지 확인하세요."
        )
        return False

    retargeter = _create_or_load(
        "RTG_Mannequin_ObsidianKnight", OUTPUT_PATH,
        unreal.IKRetargeter, factory
    )
    if not retargeter:
        return False

    try:
        rt_controller = unreal.IKRetargetingController.get_controller(retargeter)
    except Exception as e:
        unreal.log_warning(f"[GK Retarget] IKRetargetingController 획득 실패: {e}")
        return False

    # 소스·타겟 IK Rig 연결
    # UE5.x 버전에 따라 메서드명이 다를 수 있음 — 두 경로 모두 시도
    linked = False
    for set_fn, src_arg, tgt_arg in [
        # UE 5.3+ : set_ik_rig(source_or_target_enum, ik_rig)
        (
            lambda c, s, t: (
                c.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, s),
                c.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, t),
            ),
            source_ik, target_ik,
        ),
    ]:
        try:
            set_fn(rt_controller, src_arg, tgt_arg)
            linked = True
            unreal.log("[GK Retarget]   소스·타겟 IK Rig 연결 완료")
            break
        except Exception:
            pass

    if not linked:
        # fallback: 에디터 프로퍼티 직접 설정
        try:
            retargeter.set_editor_property("source_ik_rig_asset", source_ik)
            retargeter.set_editor_property("target_ik_rig_asset", target_ik)
            linked = True
            unreal.log("[GK Retarget]   (fallback) 에디터 프로퍼티로 IK Rig 연결")
        except Exception as e2:
            unreal.log_warning(
                f"[GK Retarget] IK Rig 연결 실패: {e2}\n"
                "              에디터에서 RTG_Mannequin_ObsidianKnight 를 열어\n"
                "              Source = IK_Mannequin, Target = IK_GK_ObsidianKnight 로\n"
                "              수동 지정해 주세요."
            )

    unreal.EditorAssetLibrary.save_asset(RETARGETER_PATH)
    unreal.log("[GK Retarget] ✓ RTG_Mannequin_ObsidianKnight 저장 완료")
    return linked


# ─────────────────────────────────────────────────────────────
#  메인
# ─────────────────────────────────────────────────────────────

def run():
    unreal.log("=" * 60)
    unreal.log("  Ashen Ossuary — ObsidianKnight 리타게팅 셋업 시작")
    unreal.log("=" * 60)

    # ── 전략 A: Mannequin 스켈레톤 직접 재할당 ──────────────────
    # 본 이름이 동일하므로 이 방법이 성공하면 Retargeter 없이 바로 사용 가능
    a_success = strategy_a_reassign_skeleton()

    # ── 전략 B: IK Rig + Retargeter ─────────────────────────────
    # 전략 A 성공 여부에 관계없이 IK Rig + Retargeter 도 생성합니다
    # (추후 커스텀 애니메이션 리타게팅에도 활용 가능)
    source_ik, target_ik = strategy_b_create_ik_rigs()
    rtg_success = False
    if source_ik and target_ik:
        rtg_success = create_retargeter(source_ik, target_ik)

    # ── 결과 요약 ────────────────────────────────────────────────
    unreal.log("=" * 60)
    unreal.log("  결과 요약")
    unreal.log("=" * 60)
    unreal.log(f"  전략 A (스켈레톤 직접 재할당): {'✓ 성공' if a_success else '✗ 실패/건너뜀'}")
    unreal.log(f"  IK_Mannequin              : {'✓' if source_ik else '✗'}")
    unreal.log(f"  IK_GK_ObsidianKnight      : {'✓' if target_ik else '✗'}")
    unreal.log(f"  RTG_Mannequin_ObsidianKnight: {'✓' if rtg_success else '✗ (수동 연결 필요)'}")
    unreal.log("")

    if a_success:
        unreal.log("  ✅ 전략 A 성공 — SK_GK_ObsidianKnight 가 Mannequin 스켈레톤을 사용합니다.")
        unreal.log("     Content > Characters > Player > Mesh > SK_GK_ObsidianKnight 에서")
        unreal.log("     MM_Idle / BS_Idle_Walk_Run 등 Mannequin 애니를 바로 적용 가능합니다.")
    else:
        unreal.log("  ⚠️  전략 A 실패 — IK Retargeter 를 통해 애니메이션을 리타게팅하세요.")
        unreal.log("     Content > Characters > Player > Mesh > RTG_Mannequin_ObsidianKnight")
        unreal.log("     에서 'Export Animations' 또는 'Retarget Animation Assets' 사용")

    if not rtg_success and (source_ik or target_ik):
        unreal.log("")
        unreal.log("  [수동 작업 필요] RTG_Mannequin_ObsidianKnight 열기")
        unreal.log("    Source IK Rig → IK_Mannequin")
        unreal.log("    Target IK Rig → IK_GK_ObsidianKnight")

    unreal.log("=" * 60)


run()
