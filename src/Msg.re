[@deriving show]
type t =
  | Connected
  | Ready
  | TODOMESSAGE
  | Commands(Command.msg)
  | DebugService(DebugService.msg)
  | Telemetry(Telemetry.msg)
  | Initialized
  | Disconnected
  | Unhandled
  | Unknown;
