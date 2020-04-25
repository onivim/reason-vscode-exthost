open TestFramework;

open Exthost;
open TestLib;

module InitData = Types.InitData;
module Uri = Types.Uri;
module Test = {
  type t = {
    client: Client.t,
    extensionProcess: Luv.Process.t,
    processHasExited: ref(bool),
    messages: ref(list(Msg.t)),
  };

  let noopHandler = _ => None;
  let noopErrorHandler = _ => ();

  let startWithExtensions =
      (~handler=noopHandler, ~onError=noopErrorHandler, extensions) => {
    let messages = ref([]);

    let wrappedHandler = msg => {
      messages := [msg, ...messages^];
      handler(msg);
    };

    Timber.App.enable();
    Timber.App.setLevel(Timber.Level.trace);
    let extensions =
      InitData.Extension.[
        {
          identifier: "oni-dev-extension",
          extensionLocation:
            Uri.fromPath(
              "/Users/bryphe/reason-vscode-exthost/test_collateral/extensions/oni-activation-events-tests",
            ),
          version: "9.9.9",
          name: "oni-dev-extension",
          main: Some("./extension.js"),
          engines: "vscode",
          activationEvents: ["*"],
          extensionDependencies: [],
          extensionKind: "ui",
          enableProposedApi: true,
        },
      ];

    let logsLocation = Filename.temp_file("test", "log") |> Uri.fromPath;
    let logFile = Filename.temp_dir_name |> Uri.fromPath;

    let initData =
      InitData.create(
        ~version="9.9.9",
        ~parentPid=1,
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
      |> Result.get_ok;

    let processHasExited = ref(false);

    let parentPid = Luv.Pid.getpid();
    let onExit = (_, ~exit_status as _: int64, ~term_signal as _: int) => {
      processHasExited := true;
    };

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
        ["/Users/bryphe/vscode-exthost-v2/out/bootstrap-fork.js"],
      );
    {client, extensionProcess, processHasExited, messages};
  };

  let close = context => {
    Client.close(context.client);
    context;
  };

  let waitForProcessClosed = ({processHasExited, _}) => {
    Waiter.wait(~timeout=10.0, ~name="Wait for node process to close", () =>
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

  let terminate = context => {
    Client.terminate(context.client);
    context;
  };
};

describe("LifecycleTest", ({test, _}) => {
  test("close", ({expect}) => {
    Test.startWithExtensions([])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
