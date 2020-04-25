open TestFramework;

open Exthost;
open TestLib;

module InitData = Extension.InitData;
module Uri = Types.Uri;
module Test = {
  open Extension;

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
      Msg.show(msg) |> prerr_endline;
      messages := [msg, ...messages^];
      handler(msg);
    };

    Timber.App.enable();
    Timber.App.setLevel(Timber.Level.trace);

    let rootPath =
      Rench.Path.join(Sys.getcwd(), "test_collateral/extensions");

    let extensions =
      extensions
      |> List.map(Rench.Path.join(rootPath))
      |> List.map(p => Rench.Path.join(p, "package.json"))
      |> List.map(Scanner.load(~prefix=None, ~category=Scanner.Bundled))
      |> List.filter_map(v => v)
      |> List.map(InitData.Extension.ofScanner);

    extensions |> List.iter(m => m |> InitData.Extension.show |> prerr_endline);

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
  test("close - no extensions", ({expect}) => {
    Test.startWithExtensions([])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
  test("close - extensions", ({expect}) => {
    Test.startWithExtensions(["oni-always-activate"])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
