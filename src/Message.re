[@deriving (show, yojson({strict: false}))]
type t =
  | Connected
  | Ready
  | TODOMESSAGE
  | Initialized
  | Disconnected;
