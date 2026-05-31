"""
Ashen Ossuary — Meshy AI Asset Import Script
에이전트 E 작성 | 실행 방법: UE Editor > Tools > Execute Python Script > 이 파일 선택

리깅 완료 후 SK_GK_ObsidianKnight.fbx 가 생성된 경우:
  스크립트 하단 SKELETAL_ASSETS 섹션도 자동 임포트됩니다.

임포트 대상:
  ✅ 확정 에셋
  - SM_GK_ObsidianKnight   (플레이어 캐릭터 — 확정)
  - SM_GK_Dreadblade       (플레이어 검 — 확정, Emission 발광 포함)
  - SM_GK_KnightHelmet     (헬멧)
  - SM_GK_HeavyCloak       (망토)

  📦 보관 에셋 (레퍼런스용, 임포트는 됨)
  - SM_GK_DarkplateKnight  (구버전 캐릭터)
  - SM_GK_KnightSword      (구버전 검)

  각 에셋에 5종 PBR 텍스처 세트 (BC/N/M/R/E)
"""

import unreal
import os

# ─────────────────────────────────────────────
#  경로 설정
# ─────────────────────────────────────────────
ART_ROOT = r"C:\UE\GK\art"

ASSETS = [
    {
        "name":        "DarkplateKnight",
        "fbx":         r"Character\SM_GK_DarkplateKnight.fbx",
        "ue_mesh_path": "/Game/Characters/Player/Mesh",
        "ue_tex_path":  "/Game/Characters/Player/Textures",
        "textures": {
            "BC": r"Character\T_GK_DarkplateKnight_BC.png",
            "N":  r"Character\T_GK_DarkplateKnight_N.png",
            "M":  r"Character\T_GK_DarkplateKnight_M.png",
            "R":  r"Character\T_GK_DarkplateKnight_R.png",
            "E":  r"Character\T_GK_DarkplateKnight_E.png",
        },
    },
    {
        "name":        "KnightHelmet",
        "fbx":         r"Helmet\SM_GK_KnightHelmet.fbx",
        "ue_mesh_path": "/Game/Characters/Player/Mesh",
        "ue_tex_path":  "/Game/Characters/Player/Textures",
        "textures": {
            "BC": r"Helmet\T_GK_KnightHelmet_BC.png",
            "N":  r"Helmet\T_GK_KnightHelmet_N.png",
            "M":  r"Helmet\T_GK_KnightHelmet_M.png",
            "R":  r"Helmet\T_GK_KnightHelmet_R.png",
            "E":  r"Helmet\T_GK_KnightHelmet_E.png",
        },
    },
    {
        "name":        "HeavyCloak",
        "fbx":         r"Cloak\SM_GK_HeavyCloak.fbx",
        "ue_mesh_path": "/Game/Characters/Player/Mesh",
        "ue_tex_path":  "/Game/Characters/Player/Textures",
        "textures": {
            "BC": r"Cloak\T_GK_HeavyCloak_BC.png",
            "N":  r"Cloak\T_GK_HeavyCloak_N.png",
            "M":  r"Cloak\T_GK_HeavyCloak_M.png",
            "R":  r"Cloak\T_GK_HeavyCloak_R.png",
            "E":  r"Cloak\T_GK_HeavyCloak_E.png",
        },
    },
    {
        "name":        "KnightSword",
        "fbx":         r"Sword\SM_GK_KnightSword.fbx",
        "ue_mesh_path": "/Game/Weapons/Sword/Mesh",
        "ue_tex_path":  "/Game/Weapons/Sword/Textures",
        "textures": {
            "BC": r"Sword\T_GK_KnightSword_BC.png",
            "N":  r"Sword\T_GK_KnightSword_N.png",
            "M":  r"Sword\T_GK_KnightSword_M.png",
            "R":  r"Sword\T_GK_KnightSword_R.png",
            "E":  r"Sword\T_GK_KnightSword_E.png",
        },
    },
    # ── v2 에셋 (2026-05-31 추가) ──────────────────────────────────────
    {
        "name":        "ObsidianKnight",
        "fbx":         r"Character2\SM_GK_ObsidianKnight.fbx",
        "ue_mesh_path": "/Game/Characters/Player/Mesh",
        "ue_tex_path":  "/Game/Characters/Player/Textures",
        "textures": {
            "BC": r"Character2\T_GK_ObsidianKnight_BC.png",
            "N":  r"Character2\T_GK_ObsidianKnight_N.png",
            "M":  r"Character2\T_GK_ObsidianKnight_M.png",
            "R":  r"Character2\T_GK_ObsidianKnight_R.png",
            "E":  r"Character2\T_GK_ObsidianKnight_E.png",
        },
    },
    {
        "name":        "Dreadblade",
        "fbx":         r"Sword2\SM_GK_Dreadblade.fbx",
        "ue_mesh_path": "/Game/Weapons/Sword/Mesh",
        "ue_tex_path":  "/Game/Weapons/Sword/Textures",
        "textures": {
            "BC": r"Sword2\T_GK_Dreadblade_BC.png",
            "N":  r"Sword2\T_GK_Dreadblade_N.png",
            "M":  r"Sword2\T_GK_Dreadblade_M.png",
            "R":  r"Sword2\T_GK_Dreadblade_R.png",
            "E":  r"Sword2\T_GK_Dreadblade_E.png",   # 157KB — 실제 발광 이펙트
        },
    },
]

# ─────────────────────────────────────────────
#  텍스처 압축 설정 (채널별)
# ─────────────────────────────────────────────
TEX_SETTINGS = {
    "BC": {
        "compression_settings": unreal.TextureCompressionSettings.TC_DEFAULT,
        "srgb": True,
    },
    "N": {
        "compression_settings": unreal.TextureCompressionSettings.TC_NORMALMAP,
        "srgb": False,
    },
    "M": {
        "compression_settings": unreal.TextureCompressionSettings.TC_GRAYSCALE,
        "srgb": False,
    },
    "R": {
        "compression_settings": unreal.TextureCompressionSettings.TC_GRAYSCALE,
        "srgb": False,
    },
    "E": {
        "compression_settings": unreal.TextureCompressionSettings.TC_DEFAULT,
        "srgb": True,
    },
}


def _make_import_task(source_path, dest_package_path, dest_name, options=None):
    task = unreal.AssetImportTask()
    task.filename          = source_path
    task.destination_path  = dest_package_path
    task.destination_name  = dest_name
    task.replace_existing  = True
    task.automated         = True
    task.save              = True
    if options:
        task.options = options
    return task


def import_static_mesh(asset_info):
    name     = asset_info["name"]
    fbx_path = os.path.join(ART_ROOT, asset_info["fbx"])
    ue_path  = asset_info["ue_mesh_path"]

    options = unreal.FbxImportUI()
    options.import_mesh            = True
    options.import_textures        = False   # PNG를 별도 임포트하므로 FBX 내장 텍스처 스킵
    options.import_materials       = False
    options.import_as_skeletal     = False
    options.static_mesh_import_data.combine_meshes            = True
    options.static_mesh_import_data.auto_generate_collision   = True
    options.static_mesh_import_data.generate_lightmap_u_vs    = True

    task = _make_import_task(fbx_path, ue_path, f"SM_GK_{name}", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

    result = task.imported_object_paths
    if result:
        unreal.log(f"[GK Import] ✓ SM_GK_{name}  →  {ue_path}/SM_GK_{name}")
    else:
        unreal.log_warning(f"[GK Import] ✗ SM_GK_{name} 임포트 실패 — FBX 경로 확인: {fbx_path}")
    return result


def import_textures(asset_info):
    name    = asset_info["name"]
    ue_path = asset_info["ue_tex_path"]

    for suffix, rel_path in asset_info["textures"].items():
        src  = os.path.join(ART_ROOT, rel_path)
        tname = f"T_GK_{name}_{suffix}"

        task = _make_import_task(src, ue_path, tname)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

        # 압축 설정 후처리
        tex_pkg = f"{ue_path}/{tname}"
        tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
        if tex and isinstance(tex, unreal.Texture2D):
            cfg = TEX_SETTINGS[suffix]
            tex.set_editor_property("compression_settings", cfg["compression_settings"])
            tex.set_editor_property("srgb", cfg["srgb"])
            unreal.EditorAssetLibrary.save_asset(tex_pkg)
            unreal.log(f"[GK Import] ✓ {tname}  ({suffix})")
        else:
            unreal.log_warning(f"[GK Import] ✗ {tname} 텍스처 설정 실패 — 경로: {src}")


def create_material_instance(asset_info):
    """
    SM_GK_{name}에 연결할 Material Instance를 생성합니다.
    BC/N/M/R 4채널 연결. Emission 맵이 거의 비어있으므로 기본 비활성.
    """
    name     = asset_info["name"]
    ue_path  = asset_info["ue_mesh_path"]
    tex_path = asset_info["ue_tex_path"]

    mi_path = f"{ue_path}/MI_GK_{name}"

    # 기존 MI 있으면 재사용
    if unreal.EditorAssetLibrary.does_asset_exist(mi_path):
        mi = unreal.EditorAssetLibrary.load_asset(mi_path)
    else:
        # 엔진 내장 기본 PBR M_Basic_Wall 또는 새 MI
        factory = unreal.MaterialInstanceConstantFactoryNew()
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            f"MI_GK_{name}", ue_path, unreal.MaterialInstanceConstant, factory
        )

    if not mi:
        unreal.log_warning(f"[GK Import] ✗ MI_GK_{name} 생성 실패")
        return

    tex_map = {
        "BaseColor": f"{tex_path}/T_GK_{name}_BC",
        "Normal":    f"{tex_path}/T_GK_{name}_N",
        "Metallic":  f"{tex_path}/T_GK_{name}_M",
        "Roughness": f"{tex_path}/T_GK_{name}_R",
    }

    for param_name, tex_pkg in tex_map.items():
        tex = unreal.EditorAssetLibrary.load_asset(tex_pkg)
        if tex:
            unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
                mi, param_name, tex
            )

    unreal.EditorAssetLibrary.save_asset(mi_path)
    unreal.log(f"[GK Import] ✓ MI_GK_{name} 생성 완료")

    # Static Mesh에 MI 할당
    mesh_pkg  = f"{ue_path}/SM_GK_{name}"
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_pkg)
    if mesh and isinstance(mesh, unreal.StaticMesh):
        slot_count = mesh.get_num_sections(0)
        for i in range(slot_count):
            unreal.StaticMesh.set_material(mesh, i, mi)
        unreal.EditorAssetLibrary.save_asset(mesh_pkg)
        unreal.log(f"[GK Import] ✓ SM_GK_{name} 머티리얼 {slot_count}슬롯 할당")


# ─────────────────────────────────────────────
#  스켈레탈 메시 임포트 (리깅 완료 후)
# ─────────────────────────────────────────────
SKELETAL_ASSETS = [
    {
        "fbx":      r"C:\UE\GK\art\Character2\SK_GK_ObsidianKnight.fbx",
        "ue_path":  "/Game/Characters/Player/Mesh",
        "name":     "SK_GK_ObsidianKnight",
        # 기존 UE5 Mannequin 스켈레톤에 리타게팅하여 애니메이션 재사용
        # 스켈레톤은 SK_Mannequin 메시와 같은 폴더에 자동 생성됨 (/Meshes/)
        "skeleton": "/Game/Characters/Mannequins/Meshes/SK_Mannequin_Skeleton",
    },
]


def import_skeletal_meshes():
    for info in SKELETAL_ASSETS:
        if not os.path.exists(info["fbx"]):
            unreal.log_warning(
                f"[GK Import] SK 파일 없음 (리깅 미완료): {info['fbx']}\n"
                f"           scripts/rig_obsidian_knight.py 먼저 실행하세요."
            )
            continue

        options = unreal.FbxImportUI()
        options.import_mesh        = True
        options.import_textures    = False
        options.import_materials   = False
        options.import_as_skeletal = True

        # 기존 Mannequin 스켈레톤 사용 시도
        skel = unreal.EditorAssetLibrary.load_asset(info["skeleton"])
        if skel:
            options.skeleton = skel
            unreal.log(f"[GK Import] Mannequin 스켈레톤 연결: {info['name']}")
        else:
            unreal.log_warning(
                f"[GK Import] Mannequin 스켈레톤 없음 — 새 스켈레톤 자동 생성: {info['name']}\n"
                f"           스켈레톤 수동 재할당: Scripts/setup_retargeting.py 실행"
            )

        task = _make_import_task(info["fbx"], info["ue_path"], info["name"], options)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

        if task.imported_object_paths:
            unreal.log(f"[GK Import] ✓ {info['name']}  →  {info['ue_path']}")
        else:
            unreal.log_warning(f"[GK Import] ✗ {info['name']} 임포트 실패")


# ─────────────────────────────────────────────
#  메인
# ─────────────────────────────────────────────
def run():
    unreal.log("=" * 60)
    unreal.log("  Ashen Ossuary — Meshy AI Asset Import 시작")
    unreal.log("=" * 60)

    with unreal.ScopedSlowTask(len(ASSETS) * 3, "GK 에셋 임포트 중...") as task:
        task.make_dialog(True)

        for info in ASSETS:
            name = info["name"]

            task.enter_progress_frame(1, f"[1/3] {name} 메쉬 임포트")
            import_static_mesh(info)

            task.enter_progress_frame(1, f"[2/3] {name} 텍스처 임포트")
            import_textures(info)

            task.enter_progress_frame(1, f"[3/3] {name} 머티리얼 생성")
            create_material_instance(info)

    # 리깅 완료된 스켈레탈 메시 임포트
    unreal.log("[GK Import] 스켈레탈 메시 확인 중...")
    import_skeletal_meshes()

    unreal.log("=" * 60)
    unreal.log("  임포트 완료. Content Browser에서 확인하세요:")
    unreal.log("  Characters > Player > Mesh")
    unreal.log("    SM_GK_DarkplateKnight / SM_GK_KnightHelmet / SM_GK_HeavyCloak")
    unreal.log("    SM_GK_ObsidianKnight  (v2 캐릭터)")
    unreal.log("  Weapons > Sword > Mesh")
    unreal.log("    SM_GK_KnightSword / SM_GK_Dreadblade (발광 이펙트 있음)")
    unreal.log("=" * 60)


run()
