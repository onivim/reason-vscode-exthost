[@deriving show]
type t =
  | Connected
  | Ready
  | Commands(Command.msg)
  | DebugService(DebugService.msg)
  | Telemetry(Telemetry.msg)
  | Initialized
  | Disconnected
  | Unhandled
  | Unknown;
