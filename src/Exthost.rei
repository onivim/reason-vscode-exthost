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

module ExtensionService: {
  [@deriving show]
  type msg =
    | ActivateExtension({
        extensionId: string,
        activationEvent: option(string),
      })
    | WillActivateExtension({extensionId: string})
    | DidActivateExtension({
        extensionId: string,
        //startup: bool,
        codeLoadingTime: int,
        activateCallTime: int,
        activateResolvedTime: int,
      })
    //activationEvent: option(string),
    | ExtensionActivationError({
        extensionId: string,
        errorMessage: string,
      })
    | ExtensionRuntimeError({extensionId: string});
  // TODO: Error?
};

module MessageService: {
  type severity =
  | Ignore
  | Info
  | Warning
  | Error;

  [@deriving show]
  type msg =
    | ShowMessage({
        severity: severity,
        message: string,
        extensionId: option(string),
      })
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

module StatusBar: {
  [@deriving show]
  type msg =
    | SetEntry({
        id: int,
        text: string,
        alignment: int,
        priority: int,
      });
};

module Msg: {
  [@deriving show]
  type t =
    | Connected
    | Ready
    | Commands(Commands.msg)
    | DebugService(DebugService.msg)
    | ExtensionService(ExtensionService.msg)
    | MessageService(MessageService.msg)
    | StatusBar(StatusBar.msg)
    | Telemetry(Telemetry.msg)
    | Initialized
    | Disconnected
    | Unhandled
    | Unknown({
        method: string,
        args: Yojson.Safe.t,
      });
};

module NamedPipe: {
  type t;

  let create: string => t;
  let toString: t => string;
};

module Client: {
  type t;

  // TODO
  type reply = unit;

  let start:
    (
      ~initialConfiguration: Types.Configuration.t=?,
      ~namedPipe: NamedPipe.t,
      ~initData: Extension.InitData.t,
      ~handler: Msg.t => option(reply),
      ~onError: string => unit,
      unit
    ) =>
    result(t, string);

  let close: t => unit;

  let terminate: t => unit;
};

module Request: {
  module Commands: {
    let executeContributedCommand:
      (~arguments: list(Types.Json.t), ~command: string, Client.t) => unit;
  };
  module ExtensionService: {
    let activateByEvent: (~event: string, Client.t) => unit;
  };
};

module Protocol = Protocol;
module Transport = Transport;
module Utility = Utility;
