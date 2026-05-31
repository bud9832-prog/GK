# Animation montage table setup (Agent B)
# SSOT: Design/ANIMATION_TABLE_SPEC.md
# Run: UnrealEditor-Cmd.exe GK.uproject -ExecutePythonScript=Scripts/SetupAnimationMontageTable.py

import os
import unreal

PROJECT_CONTENT = "/Game"
DATA_PATH = f"{PROJECT_CONTENT}/Data"
BP_PATH = f"{PROJECT_CONTENT}/ThirdPerson/Blueprints"

CSV_PATH = os.path.join(unreal.Paths.project_content_dir(), "Data", "DT_AnimationMontages.csv")

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EDITOR_ASSET_LIB = unreal.EditorAssetLibrary

EXPECTED_PC_ROWS = 16
EXPECTED_ENEMY_ROWS = 6

CRITICAL_ROW_MONTAGES = {
    "PC.Attack_Combo.0": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash",
    "PC.Attack_Combo.1": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump",
    "PC.Attack_Combo.2": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash",
    "PC.Evade": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash",
    "PC.Heal": "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle",
    "PC.HeavyAttack": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump",
    "PC.Parry_Active": "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle",
    "PC.Parry_Recovery": "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle",
    "Enemy.FirstEnemy.Idle": "/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle",
    "Enemy.FirstEnemy.Attack": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash",
}


def log(msg: str) -> None:
    unreal.log(f"[SetupAnimTable] {msg}")


def save_asset(asset) -> None:
    if asset:
        EDITOR_ASSET_LIB.save_loaded_asset(asset, only_if_is_dirty=False)


def verify_critical_montages(table: unreal.DataTable) -> None:
    row_names = {str(name) for name in table.get_row_names()}
    for row_name, asset_path in CRITICAL_ROW_MONTAGES.items():
        if row_name not in row_names:
            raise RuntimeError(f"Critical row missing: {row_name}")

        if not EDITOR_ASSET_LIB.does_asset_exist(asset_path):
            raise RuntimeError(f"Placeholder montage asset missing: {asset_path}")

        montage = EDITOR_ASSET_LIB.load_asset(asset_path)
        if not montage:
            raise RuntimeError(f"Placeholder montage failed to load: {asset_path}")

        log(f"Verified critical row {row_name} -> {asset_path} ({montage.get_name()})")


def create_animation_montage_table() -> unreal.DataTable:
    asset_path = f"{DATA_PATH}/DT_AnimationMontages"
    if EDITOR_ASSET_LIB.does_asset_exist(asset_path):
        EDITOR_ASSET_LIB.delete_asset(asset_path)

    factory_class = unreal.load_class(None, "/Script/UnrealEd.DataTableFactory")
    factory = unreal.new_object(factory_class)
    factory.set_editor_property("struct", unreal.GKAnimMontageRow.static_struct())

    table = ASSET_TOOLS.create_asset("DT_AnimationMontages", DATA_PATH, unreal.DataTable, factory)
    if not table:
        raise RuntimeError("Failed to create DT_AnimationMontages asset")

    if not os.path.isfile(CSV_PATH):
        raise RuntimeError(f"CSV not found: {CSV_PATH}")

    with open(CSV_PATH, "r", encoding="utf-8") as csv_file:
        csv_text = csv_file.read()

    unreal.DataTableFunctionLibrary.fill_data_table_from_csv_string(table, csv_text)

    row_count = len(table.get_row_names())
    if row_count != EXPECTED_PC_ROWS + EXPECTED_ENEMY_ROWS:
        raise RuntimeError(f"Unexpected row count: {row_count} (expected {EXPECTED_PC_ROWS + EXPECTED_ENEMY_ROWS})")

    verify_critical_montages(table)

    save_asset(table)
    log(f"Created DT_AnimationMontages with {row_count} rows")
    return table


def wire_bp_gk_character(anim_table: unreal.DataTable) -> None:
    bp = EDITOR_ASSET_LIB.load_asset(f"{BP_PATH}/BP_GKCharacter")
    if not bp:
        raise RuntimeError("BP_GKCharacter not found")

    bp_cdo = unreal.get_default_object(bp.generated_class())
    bp_cdo.set_editor_property("anim_montage_table", anim_table)

    if hasattr(unreal, "BlueprintEditorLibrary"):
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)

    save_asset(bp)
    log("Wired BP_GKCharacter.anim_montage_table -> DT_AnimationMontages")


def main() -> None:
    log("Starting ANIMATION_TABLE_SPEC setup")
    anim_table = create_animation_montage_table()
    wire_bp_gk_character(anim_table)
    EDITOR_ASSET_LIB.save_directory(DATA_PATH, only_if_is_dirty=False, recursive=True)
    EDITOR_ASSET_LIB.save_directory(BP_PATH, only_if_is_dirty=False, recursive=True)
    log("Setup complete")


if __name__ == "__main__":
    main()
