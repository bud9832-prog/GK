# Verification environment setup (Agent B)
# - Ensures BP_ThirdPersonGameMode -> BP_GKCharacter DefaultPawnClass chain
# - Creates/updates non-WorldPartition test map: /Game/Temp/Lvl_CombatVerify
# - Points DefaultEngine.ini startup/default maps to the verify level
#
# Run:
#   UnrealEditor-Cmd.exe GK.uproject -ExecutePythonScript=Scripts/SetupVerificationEnvironment.py

import os
import re
import unreal

PROJECT_CONTENT = "/Game"
BP_PATH = f"{PROJECT_CONTENT}/ThirdPerson/Blueprints"
VERIFY_LEVEL_PATH = f"{PROJECT_CONTENT}/Temp/Lvl_CombatVerify"
GM_BP_PATH = f"{BP_PATH}/BP_ThirdPersonGameMode"
GK_CHAR_BP_PATH = f"{BP_PATH}/BP_GKCharacter"
GLOBAL_GM_PATH = "/Script/GK.GKGameMode"

EDITOR_ASSET_LIB = unreal.EditorAssetLibrary


def log(msg: str) -> None:
    unreal.log(f"[SetupVerifyEnv] {msg}")


def save_asset(asset) -> None:
    if asset:
        EDITOR_ASSET_LIB.save_loaded_asset(asset, only_if_is_dirty=False)


def compile_blueprint(bp: unreal.Blueprint) -> None:
    if hasattr(unreal, "BlueprintEditorLibrary"):
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    elif hasattr(unreal, "KismetCompilerLibrary"):
        unreal.KismetCompilerLibrary.compile_blueprint(bp)


def get_class_name(obj) -> str:
    if not obj:
        return "None"
    if hasattr(obj, "get_name"):
        return obj.get_name()
    return str(obj)


def ensure_game_mode_chain() -> None:
    bp_gk = EDITOR_ASSET_LIB.load_asset(GK_CHAR_BP_PATH)
    if not bp_gk:
        raise RuntimeError(f"Missing blueprint: {GK_CHAR_BP_PATH}")

    gm_bp = EDITOR_ASSET_LIB.load_asset(GM_BP_PATH)
    if not gm_bp:
        raise RuntimeError(f"Missing blueprint: {GM_BP_PATH}")

    compile_blueprint(gm_bp)
    gm_cdo = unreal.get_default_object(gm_bp.generated_class())
    gk_class = bp_gk.generated_class()
    gm_cdo.set_editor_property("default_pawn_class", gk_class)

    compile_blueprint(gm_bp)
    save_asset(gm_bp)

    verify_cdo = unreal.get_default_object(gm_bp.generated_class())
    verify_pawn = verify_cdo.get_editor_property("default_pawn_class")
    log(f"GameMode DefaultPawnClass -> {get_class_name(verify_pawn)}")
    if verify_pawn != gk_class:
        raise RuntimeError("DefaultPawnClass verification failed on BP_ThirdPersonGameMode")


def disable_world_partition_if_present(editor_world) -> None:
    if not editor_world:
        return

    world_settings = editor_world.get_world_settings()
    if not world_settings:
        return

    for prop_name in ("enable_world_partition", "b_enable_world_partition"):
        try:
            if world_settings.get_editor_property(prop_name):
                world_settings.set_editor_property(prop_name, False)
                log(f"Disabled WorldPartition via {prop_name}")
                return
        except Exception:
            pass

    for lib_name in (
        "EditorWorldPartitionLibrary",
        "WorldPartitionEditorBlueprintLibrary",
    ):
        lib = getattr(unreal, lib_name, None)
        if not lib:
            continue
        for method_name in (
            "disable_world_partition",
            "remove_editor_world_partition",
        ):
            method = getattr(lib, method_name, None)
            if callable(method):
                try:
                    method(editor_world)
                    log(f"Disabled WorldPartition via {lib_name}.{method_name}")
                    return
                except Exception as exc:
                    log(f"{lib_name}.{method_name} failed: {exc}")


def spawn_actor(actor_class, location, rotation):
    spawned = unreal.EditorLevelLibrary.spawn_actor_from_class(
        actor_class,
        location,
        rotation,
    )
    if not spawned:
        raise RuntimeError(f"Failed to spawn {actor_class}")
    return spawned


def ensure_floor_and_light() -> None:
    plane_mesh = EDITOR_ASSET_LIB.load_asset("/Engine/BasicShapes/Plane")
    if plane_mesh:
        floor = spawn_actor(
            unreal.StaticMeshActor,
            unreal.Vector(0.0, 0.0, 0.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        floor.static_mesh_component.set_static_mesh(plane_mesh)
        floor.set_actor_scale3d(unreal.Vector(20.0, 20.0, 1.0))
        log("Spawned floor plane")

    light_class = getattr(unreal, "DirectionalLight", None)
    if not light_class:
        light_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
    if light_class:
        spawn_actor(
            light_class,
            unreal.Vector(0.0, 0.0, 500.0),
            unreal.Rotator(-45.0, 45.0, 0.0),
        )
        log("Spawned directional light")


def ensure_player_start() -> None:
    player_start_class = getattr(unreal, "PlayerStart", None)
    if not player_start_class:
        player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")

    existing = unreal.GameplayStatics.get_all_actors_of_class(
        unreal.EditorLevelLibrary.get_editor_world(),
        player_start_class,
    )
    if existing:
        log(f"PlayerStart already present ({len(existing)})")
        return

    spawned = spawn_actor(
        player_start_class,
        unreal.Vector(0.0, 0.0, 120.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    log(f"Spawned PlayerStart: {spawned.get_name()}")


def set_level_game_mode_override() -> None:
    editor_world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = editor_world.get_world_settings()
    gm_class = unreal.load_class(None, GLOBAL_GM_PATH)
    if not gm_class:
        raise RuntimeError(f"Missing GameMode class: {GLOBAL_GM_PATH}")
    world_settings.set_editor_property("default_game_mode", gm_class)
    log(f"Level GameMode override -> {GLOBAL_GM_PATH}")


def ensure_verify_level() -> None:
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if EDITOR_ASSET_LIB.does_asset_exist(VERIFY_LEVEL_PATH):
        log(f"Loading existing level: {VERIFY_LEVEL_PATH}")
        if not level_subsystem.load_level(VERIFY_LEVEL_PATH):
            raise RuntimeError(f"Failed to load level: {VERIFY_LEVEL_PATH}")
    else:
        log(f"Creating new level: {VERIFY_LEVEL_PATH}")
        if not level_subsystem.new_level(VERIFY_LEVEL_PATH):
            raise RuntimeError(f"Failed to create level: {VERIFY_LEVEL_PATH}")

    editor_world = unreal.EditorLevelLibrary.get_editor_world()
    disable_world_partition_if_present(editor_world)

    set_level_game_mode_override()
    ensure_player_start()
    ensure_floor_and_light()

    if not level_subsystem.save_current_level():
        raise RuntimeError("Failed to save verify level")

    log(f"Saved verify level: {VERIFY_LEVEL_PATH}")


def update_default_engine_maps() -> None:
    engine_ini_path = os.path.join(unreal.Paths.project_dir(), "Config", "DefaultEngine.ini")
    if not os.path.isfile(engine_ini_path):
        raise RuntimeError(f"Missing config file: {engine_ini_path}")

    with open(engine_ini_path, "r", encoding="utf-8") as ini_file:
        content = ini_file.read()

    verify_map = f"{VERIFY_LEVEL_PATH}.{VERIFY_LEVEL_PATH.split('/')[-1]}"
    replacements = {
        r"^EditorStartupMap=.*$": f"EditorStartupMap={verify_map}",
        r"^GameDefaultMap=.*$": f"GameDefaultMap={verify_map}",
        r"^GlobalDefaultGameMode=.*$": f"GlobalDefaultGameMode={GLOBAL_GM_PATH}",
    }

    for pattern, replacement in replacements.items():
        content, count = re.subn(pattern, replacement, content, count=1, flags=re.MULTILINE)
        if count != 1:
            raise RuntimeError(f"Failed to update config key via pattern: {pattern}")

    with open(engine_ini_path, "w", encoding="utf-8", newline="\n") as ini_file:
        ini_file.write(content)

    log(f"DefaultEngine.ini -> EditorStartupMap/GameDefaultMap={verify_map}")
    log(f"DefaultEngine.ini -> GlobalDefaultGameMode={GLOBAL_GM_PATH}")


def main() -> None:
    log("Starting verification environment setup")
    ensure_game_mode_chain()
    ensure_verify_level()
    update_default_engine_maps()
    log("Verification environment setup complete")
    log("PIE checklist: Output Log must contain [GK|Possession] PossessedBy ... BP_GKCharacter ...")


if __name__ == "__main__":
    main()
