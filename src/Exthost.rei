module Types = Types;

module Commands: {
  [@deriving show]
  type msg =
    | RegisterCommand(string)
    | UnregisterCommand(string)
    | ExecuteCommand({
        command: string,
        args: list(Yojson.Safe.t),
        retry: bool,
      })
    | GetCommands;
};

module DebugService: {
  [@deriving show]
  type msg =
    | RegisterDebugTypes(list(string));
};

module Telemetry: {
  [@deriving show]
  type msg =
    | PublicLog({
        eventName: string,
        data: Yojson.Safe.t,
      })
    | PublicLog2({
        eventName: string,
        data: Yojson.Safe.t,
      });
};

module Msg: {
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
    | Unknown({
        method: string,
        args: Yojson.Safe.t,
      });
};

module Client: {
  type t;

  // TODO
  type reply = unit;

  let start:
    (
      ~initialConfiguration: Types.Configuration.t=?,
      ~namedPipe: string,
      ~initData: Types.InitData.t,
      ~handler: Msg.t => option(reply),
      ~onError: string => unit,
      unit
    ) =>
    result(t, string);
};