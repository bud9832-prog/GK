import time
import unreal

TAG = "[VerifyEnvSmoke]"


def log(msg: str) -> None:
    unreal.log(f"{TAG} {msg}")


class SmokeRunner:
    def __init__(self):
        self.level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        self.tick_handle = None
        self.started_at = time.monotonic()
        self.done = False

    def start(self) -> None:
        try:
            unreal.EditorPythonScripting.set_keep_python_script_alive(True)
        except Exception:
            pass

        self.tick_handle = unreal.register_slate_post_tick_callback(self.tick)
        if not self.level_subsystem.load_level("/Game/Temp/Lvl_CombatVerify"):
            raise RuntimeError("Failed to load /Game/Temp/Lvl_CombatVerify")
        log("Loaded verify level, requesting PIE (Play mode)")
        started = False
        editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        if editor_subsystem and hasattr(editor_subsystem, "request_play_session"):
            editor_subsystem.request_play_session(True, False)
            started = True
        elif hasattr(self.level_subsystem, "editor_play_simulate"):
            self.level_subsystem.editor_play_simulate()
            started = True
        elif hasattr(unreal.EditorLevelLibrary, "editor_play"):
            unreal.EditorLevelLibrary.editor_play()
            started = True

        if not started:
            raise RuntimeError("No supported PIE start API found")

    def finish(self, ok: bool, details: str) -> None:
        if self.done:
            return
        self.done = True

        if self.tick_handle is not None:
            try:
                unreal.unregister_slate_post_tick_callback(self.tick_handle)
            except Exception:
                pass
            self.tick_handle = None

        try:
            self.level_subsystem.editor_request_end_play()
        except Exception:
            pass

        log(f"RESULT={'PASS' if ok else 'FAIL'} {details}")
        try:
            unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")
        except Exception:
            pass

    def tick(self, _delta) -> None:
        if self.done:
            return

        if time.monotonic() - self.started_at > 20.0:
            self.finish(False, "timeout waiting for pawn possession")
            return

        if not self.level_subsystem.is_in_play_in_editor():
            return

        pie_worlds = unreal.EditorLevelLibrary.get_pie_worlds(False)
        if not pie_worlds:
            return

        player_controller = unreal.GameplayStatics.get_player_controller(pie_worlds[0], 0)
        if not player_controller:
            return

        pawn = player_controller.get_controlled_pawn()
        if not pawn:
            return

        pawn_class = pawn.get_class().get_name()
        log(
            f"Possession OK controller={player_controller.get_name()} "
            f"pawn={pawn.get_name()} class={pawn_class}"
        )
        if "GKCharacter" not in pawn_class:
            self.finish(False, f"unexpected pawn class: {pawn_class}")
            return

        self.finish(True, f"possessed {pawn_class}")


SmokeRunner().start()
