print_endline("Hello, world!");

open Rench;
open Exthost;

module InitData = Extension.InitData;
module Uri = Types.Uri;

Printexc.record_backtrace(true);

Timber.App.enable();
Timber.App.setLevel(Timber.Level.trace);

let extensions =
  Path.join(Sys.getcwd(), "test_collateral/extensions")
  |> Extension.Scanner.scan(~prefix=None, ~category=Bundled)
  |> List.map(InitData.Extension.ofScanner);

extensions |> List.iter(m => m |> InitData.Extension.show |> prerr_endline);

prerr_endline("LENGTH: " ++ string_of_int(List.length(extensions)));
let initData =
  InitData.create(
    ~version="9.9.9",
    ~parentPid=1,
    ~logsLocation=Uri.fromPath("/tmp/loggy"),
    ~logFile=Uri.fromPath("/tmp/log-file"),
    extensions,
  );

let handler = msg => {
  Msg.show(msg) |> prerr_endline;
  None;
};

let onError = prerr_endline;

let pipe = NamedPipe.create("hello");
prerr_endline("PIPE: " ++ (pipe |> NamedPipe.toString));

let pipeStr = NamedPipe.toString(pipe);

let client = Client.start(~namedPipe=pipe, ~initData, ~handler, ~onError, ());

let env = Luv.Env.environ() |> Result.get_ok;

let spawnNode = (~onExit, ~args) => {
  Luv.Process.spawn(
    ~on_exit=onExit,
    ~environment=[
      (
        "AMD_ENTRYPOINT",
        "vs/workbench/services/extensions/node/extensionHostProcess",
      ),
      ("VSCODE_IPC_HOOK_EXTHOST", pipeStr),
      ...env,
    ],
    /*~redirect=[
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stdin,
          ~from_parent_fd=Luv.Process.stdin,
          (),
        ),
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stdout,
          ~from_parent_fd=Luv.Process.stderr,
          (),
        ),
        Luv.Process.inherit_fd(
          ~fd=Luv.Process.stderr,
          ~from_parent_fd=Luv.Process.stderr,
          (),
        ),
      ],*/
    "/usr/local/bin/node",
    ["/usr/local/bin/node", ...args],
  )
  |> Result.get_ok;
};
let onExit = (_, ~exit_status as _: int64, ~term_signal as _: int) => ();

spawnNode(
  ~onExit,
  ~args=["/Users/bryphe/vscode-exthost-v2/out/bootstrap-fork.js"],
);

Luv.Loop.run() |> ignore;
