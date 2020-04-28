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

module TerminalService: {
  [@deriving show]
  type msg =
    | SendProcessTitle({
        terminalId: int,
        title: string,
      })
    | SendProcessData({
        terminalId: int,
        data: string,
      })
    | SendProcessPid({
        terminalId: int,
        pid: int,
      })
    | SendProcessExit({
        terminalId: int,
        exitCode: int,
      });
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
    | ExtensionService(ExtensionService.msg)
    | Telemetry(Telemetry.msg)
    | TerminalService(TerminalService.msg)
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

  module TerminalService: {
    let spawnExtHostProcess:
      (
        ~id: int,
        ~shellLaunchConfig: Types.ShellLaunchConfig.t,
        ~activeWorkspaceRoot: Types.Uri.t,
        ~cols: int,
        ~rows: int,
        ~isWorkspaceShellAllowed: bool,
        Client.t
      ) =>
      unit;
  };
};

module Protocol = Protocol;
module Transport = Transport;
module Utility = Utility;
