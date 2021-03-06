open Extension;

open Exthost;
open TestLib;

module Log = (val Timber.Log.withNamespace("Test"));

module InitData = Extension.InitData;
module Uri = Types.Uri;

type t = {
  client: Client.t,
  extensionProcess: Luv.Process.t,
  processHasExited: ref(bool),
  messages: ref(list(Msg.t)),
};

let noopHandler = _ => None;
let noopErrorHandler = _ => ();

let startWithExtensions =
    (
      ~pid=Luv.Pid.getpid(),
      ~handler=noopHandler,
      ~onError=noopErrorHandler,
      extensions,
    ) => {
  let messages = ref([]);

  let wrappedHandler = msg => {
    Msg.show(msg) |> prerr_endline;
    messages := [msg, ...messages^];
    handler(msg);
  };

  Timber.App.enable();
  Timber.App.setLevel(Timber.Level.trace);

  let rootPath = Rench.Path.join(Sys.getcwd(), "test_collateral/extensions");

  let extensions =
    extensions
    |> List.map(Rench.Path.join(rootPath))
    |> List.map(p => Rench.Path.join(p, "package.json"))
    |> List.map(Scanner.load(~prefix=None, ~category=Scanner.Bundled))
    |> List.filter_map(v => v)
    |> List.map((Extension.Scanner.ScanResult.{manifest, path, _}) => {
         InitData.Extension.ofManifestAndPath(manifest, path)
       });

  extensions |> List.iter(m => m |> InitData.Extension.show |> prerr_endline);

  let logsLocation = Filename.temp_file("test", "log") |> Uri.fromPath;
  let logFile = Filename.get_temp_dir_name() |> Uri.fromPath;

  let parentPid = pid;

  let initData =
    InitData.create(
      ~version="9.9.9",
      ~parentPid,
      ~logsLocation,
      ~logFile,
      extensions,
    );
  let pipe = NamedPipe.create("pipe-for-test");
  let pipeStr = NamedPipe.toString(pipe);
  let client =
    Client.start(
      ~namedPipe=pipe,
      ~initData,
      ~handler=wrappedHandler,
      ~onError,
      (),
    )
    |> ResultEx.tap_error(msg => prerr_endline(msg))
    |> Result.get_ok;

  let processHasExited = ref(false);

  let onExit = (_, ~exit_status as _: int64, ~term_signal as _: int) => {
    processHasExited := true;
  };

  let extHostScriptPath =
    Rench.Path.join(
      Sys.getcwd(),
      "node/node_modules/@onivim/vscode-exthost/out/bootstrap-fork.js",
    );

  let extensionProcess =
    Node.spawn(
      ~additionalEnv=[
        (
          "AMD_ENTRYPOINT",
          "vs/workbench/services/extensions/node/extensionHostProcess",
        ),
        ("VSCODE_IPC_HOOK_EXTHOST", pipeStr),
        ("VSCODE_PARENT_PID", parentPid |> string_of_int),
      ],
      ~onExit,
      [extHostScriptPath],
    );
  {client, extensionProcess, processHasExited, messages};
};

let close = context => {
  Client.close(context.client);
  context;
};

let activateByEvent = (~event, context) => {
  Request.ExtensionService.activateByEvent(~event, context.client);
  context;
};

let executeContributedCommand = (~command, context) => {
  Request.Commands.executeContributedCommand(
    ~arguments=[],
    ~command,
    context.client,
  );
  context;
};

let waitForProcessClosed = ({processHasExited, _}) => {
  Waiter.wait(~timeout=15.0, ~name="Wait for node process to close", () =>
    processHasExited^
  );
};

let waitForMessage = (~name, f, {messages, _} as context) => {
  Waiter.wait(~name="Wait for message: " ++ name, () =>
    List.exists(f, messages^)
  );

  context;
};
let waitForReady = context => {
  waitForMessage(~name="Ready", msg => msg == Ready, context);
};

let withClient = (f, context) => {
  f(context.client);
  context;
};

let terminate = context => {
  Client.terminate(context.client);
  context;
};
