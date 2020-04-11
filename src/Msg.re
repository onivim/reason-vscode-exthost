[@deriving show]
type t =
  | Connected
  | Ready
  | Commands(Commands.msg)
  | DebugService(DebugService.msg)
  | Telemetry(Telemetry.msg)
  | Initialized
  | Disconnected
  | Unhandled
  | Unknown;
