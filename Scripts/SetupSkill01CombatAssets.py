# SKILL_01 v2 — Input / Data / BP editor setup (Agent B)
# Run: UnrealEditor-Cmd.exe GK.uproject -ExecutePythonScript=Scripts/SetupSkill01CombatAssets.py

import os
import unreal

PROJECT_CONTENT = "/Game"
INPUT_ACTIONS_PATH = f"{PROJECT_CONTENT}/Input/Actions"
DATA_PATH = f"{PROJECT_CONTENT}/Data"
BP_PATH = f"{PROJECT_CONTENT}/ThirdPerson/Blueprints"

CSV_PATH = os.path.join(unreal.Paths.project_content_dir(), "Data", "DT_ComboAttacks.csv")

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EDITOR_ASSET_LIB = unreal.EditorAssetLibrary


def log(msg: str) -> None:
    unreal.log(f"[SetupSkill01] {msg}")


def save_asset(asset) -> None:
    if asset:
        EDITOR_ASSET_LIB.save_loaded_asset(asset, only_if_is_dirty=False)


def compile_blueprint(bp: unreal.Blueprint) -> None:
    if hasattr(unreal, "BlueprintEditorLibrary"):
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    elif hasattr(unreal, "KismetCompilerLibrary"):
        unreal.KismetCompilerLibrary.compile_blueprint(bp)


def get_or_create_input_action(name: str) -> unreal.InputAction:
    asset_path = f"{INPUT_ACTIONS_PATH}/{name}"
    if EDITOR_ASSET_LIB.does_asset_exist(asset_path):
        log(f"InputAction exists: {name}")
        return EDITOR_ASSET_LIB.load_asset(asset_path)

    template_path = f"{INPUT_ACTIONS_PATH}/IA_Move"
    if not EDITOR_ASSET_LIB.does_asset_exist(template_path):
        raise RuntimeError("Template IA_Move not found")

    action = EDITOR_ASSET_LIB.duplicate_asset(template_path, asset_path)
    if not action:
        action = ASSET_TOOLS.create_asset(name, INPUT_ACTIONS_PATH, unreal.InputAction, None)

    if not action:
        raise RuntimeError(f"Failed to create InputAction: {name}")

    action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
    save_asset(action)
    log(f"Created InputAction: {name}")
    return action


def create_combo_data_table() -> unreal.DataTable:
    asset_path = f"{DATA_PATH}/DT_ComboAttacks"
    if EDITOR_ASSET_LIB.does_asset_exist(asset_path):
        EDITOR_ASSET_LIB.delete_asset(asset_path)

    factory_class = unreal.load_class(None, "/Script/UnrealEd.DataTableFactory")
    factory = unreal.new_object(factory_class)
    factory.set_editor_property("struct", unreal.GKComboAttackRow.static_struct())

    table = ASSET_TOOLS.create_asset("DT_ComboAttacks", DATA_PATH, unreal.DataTable, factory)
    if not table:
        raise RuntimeError("Failed to create DT_ComboAttacks asset")

    if not os.path.isfile(CSV_PATH):
        raise RuntimeError(f"CSV not found: {CSV_PATH}")

    with open(CSV_PATH, "r", encoding="utf-8") as csv_file:
        csv_text = csv_file.read()

    unreal.DataTableFunctionLibrary.fill_data_table_from_csv_string(table, csv_text)

    save_asset(table)
    log("Created DT_ComboAttacks with 3 rows")
    return table


def get_or_create_data_asset(name: str, asset_class) -> unreal.Object:
    asset_path = f"{DATA_PATH}/{name}"
    if EDITOR_ASSET_LIB.does_asset_exist(asset_path):
        log(f"DataAsset exists: {name}")
        return EDITOR_ASSET_LIB.load_asset(asset_path)

    asset = ASSET_TOOLS.create_asset(name, DATA_PATH, asset_class, None)
    if not asset:
        raise RuntimeError(f"Failed to create DataAsset: {name}")

    save_asset(asset)
    log(f"Created DataAsset: {name}")
    return asset


def make_key(key_name: str):
    if hasattr(unreal, "string_to_key"):
        return unreal.string_to_key(key_name)
    key = unreal.Key()
    key.set_editor_property("key_name", unreal.Name(key_name))
    return key


def add_mapping(imc: unreal.InputMappingContext, action: unreal.InputAction, key_name: str) -> None:
    mapping = unreal.EnhancedActionKeyMapping()
    mapping.set_editor_property("action", action)
    mapping.set_editor_property("key", make_key(key_name))

    mappings = list(imc.get_editor_property("mappings"))
    for existing in mappings:
        if existing.get_editor_property("action") == action and str(existing.get_editor_property("key")) == key_name:
            return

    mappings.append(mapping)
    imc.set_editor_property("mappings", mappings)
    log(f"Mapped {action.get_name()} -> {key_name}")


def setup_imc_default(
    ia_evade_sprint: unreal.InputAction,
    ia_attack: unreal.InputAction,
    ia_heal: unreal.InputAction,
    ia_lock_on: unreal.InputAction,
) -> None:
    imc = EDITOR_ASSET_LIB.load_asset(f"{PROJECT_CONTENT}/Input/IMC_Default")
    if not imc:
        raise RuntimeError("IMC_Default not found")

    add_mapping(imc, ia_evade_sprint, "SpaceBar")
    add_mapping(imc, ia_attack, "LeftMouseButton")
    add_mapping(imc, ia_heal, "E")
    add_mapping(imc, ia_lock_on, "MiddleMouseButton")

    save_asset(imc)
    log("Updated IMC_Default")


def get_or_create_bp_gk_character(
    combat_config: unreal.GKCombatConfig,
    player_stats_config: unreal.GKPlayerStatsConfig,
    imc: unreal.InputMappingContext,
    ia_move: unreal.InputAction,
    ia_look: unreal.InputAction,
    ia_evade_sprint: unreal.InputAction,
    ia_attack: unreal.InputAction,
    ia_heal: unreal.InputAction,
    ia_lock_on: unreal.InputAction,
) -> unreal.Blueprint:
    asset_path = f"{BP_PATH}/BP_GKCharacter"
    existing = EDITOR_ASSET_LIB.load_asset(asset_path)
    if existing:
        bp = existing
        log("BP_GKCharacter exists — updating defaults")
    else:
        bp_factory_class = unreal.load_class(None, "/Script/UnrealEd.BlueprintFactory")
        bp_factory = unreal.new_object(bp_factory_class)
        bp_factory.set_editor_property("parent_class", unreal.GKCharacter)
        bp = ASSET_TOOLS.create_asset("BP_GKCharacter", BP_PATH, unreal.Blueprint, bp_factory)
        if not bp:
            raise RuntimeError("Failed to create BP_GKCharacter")
        log("Created BP_GKCharacter")

    sk_mesh = EDITOR_ASSET_LIB.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")
    if not sk_mesh:
        sk_mesh = EDITOR_ASSET_LIB.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple")
    anim_bp = EDITOR_ASSET_LIB.load_asset("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed")

    compile_blueprint(bp)
    bp_cdo = unreal.get_default_object(bp.generated_class())

    mesh_comp = bp_cdo.get_editor_property("mesh")
    if sk_mesh:
        mesh_comp.set_editor_property("skeletal_mesh_asset", sk_mesh)
    if anim_bp and hasattr(anim_bp, "generated_class"):
        mesh_comp.set_editor_property("anim_class", anim_bp.generated_class())

    bp_cdo.set_editor_property("default_mapping_context", imc)
    bp_cdo.set_editor_property("move_action", ia_move)
    bp_cdo.set_editor_property("look_action", ia_look)
    bp_cdo.set_editor_property("evade_sprint_action", ia_evade_sprint)
    bp_cdo.set_editor_property("attack_action", ia_attack)
    bp_cdo.set_editor_property("heal_action", ia_heal)
    bp_cdo.set_editor_property("lock_on_action", ia_lock_on)
    bp_cdo.set_editor_property("combat_config", combat_config)
    bp_cdo.set_editor_property("player_stats_config", player_stats_config)

    compile_blueprint(bp)
    save_asset(bp)
    log("Configured BP_GKCharacter defaults")
    return bp


def update_game_mode_default_pawn(bp_gk_character: unreal.Blueprint) -> None:
    gm_bp = EDITOR_ASSET_LIB.load_asset(f"{BP_PATH}/BP_ThirdPersonGameMode")
    if not gm_bp:
        raise RuntimeError("BP_ThirdPersonGameMode not found")

    compile_blueprint(gm_bp)
    gm_cdo = unreal.get_default_object(gm_bp.generated_class())
    gm_cdo.set_editor_property("default_pawn_class", bp_gk_character.generated_class())

    compile_blueprint(gm_bp)
    save_asset(gm_bp)
    log("Updated BP_ThirdPersonGameMode DefaultPawnClass -> BP_GKCharacter")


def main() -> None:
    log("Starting SKILL_01 editor asset setup")

    ia_move = EDITOR_ASSET_LIB.load_asset(f"{INPUT_ACTIONS_PATH}/IA_Move")
    ia_look = EDITOR_ASSET_LIB.load_asset(f"{INPUT_ACTIONS_PATH}/IA_Look")
    if not ia_move or not ia_look:
        raise RuntimeError("IA_Move / IA_Look must exist before running this script")

    ia_evade_sprint = get_or_create_input_action("IA_EvadeSprint")
    ia_attack = get_or_create_input_action("IA_Attack")
    ia_heal = get_or_create_input_action("IA_Heal")
    ia_lock_on = get_or_create_input_action("IA_LockOn")

    combo_table = create_combo_data_table()

    combat_config = get_or_create_data_asset("DA_CombatConfig", unreal.GKCombatConfig)
    combat_config.set_editor_property("combo_attack_table", combo_table)
    save_asset(combat_config)

    player_stats_config = get_or_create_data_asset("DA_PlayerStatsConfig", unreal.GKPlayerStatsConfig)
    save_asset(player_stats_config)

    imc = EDITOR_ASSET_LIB.load_asset(f"{PROJECT_CONTENT}/Input/IMC_Default")
    setup_imc_default(ia_evade_sprint, ia_attack, ia_heal, ia_lock_on)

    bp_gk = get_or_create_bp_gk_character(
        combat_config,
        player_stats_config,
        imc,
        ia_move,
        ia_look,
        ia_evade_sprint,
        ia_attack,
        ia_heal,
        ia_lock_on,
    )

    update_game_mode_default_pawn(bp_gk)

    EDITOR_ASSET_LIB.save_directory(DATA_PATH, only_if_is_dirty=False, recursive=True)
    EDITOR_ASSET_LIB.save_directory(INPUT_ACTIONS_PATH, only_if_is_dirty=False, recursive=True)
    EDITOR_ASSET_LIB.save_directory(BP_PATH, only_if_is_dirty=False, recursive=True)

    log("SKILL_01 editor asset setup complete")


if __name__ == "__main__":
    main()
