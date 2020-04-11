[@deriving show]
type t =
  | Connected
  | Ready
  | TODOMESSAGE
  | Telemetry(Telemetry.msg)
  | Initialized
  | Disconnected
  | Unhandled
  | Unknown;
