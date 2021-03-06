open Rench;
open Exthost;

module InitData = Extension.InitData;
module Uri = Types.Uri;

Printexc.record_backtrace(true);

Timber.App.enable();
Timber.App.setLevel(Timber.Level.trace);

let nodePath = Utility.getNodePath();

let extensions =
  Path.join(Sys.getcwd(), "test_collateral/extensions")
  |> Extension.Scanner.scan(~prefix=None, ~category=Bundled)
  |> List.map((Extension.Scanner.ScanResult.{manifest, path, _}) => {
       InitData.Extension.ofManifestAndPath(manifest, path)
     });

extensions |> List.iter(m => m |> InitData.Extension.show |> prerr_endline);

let parentPid = Luv.Pid.getpid();

let logsLocation = Filename.temp_file("test", "log") |> Uri.fromPath;
let logFile = Filename.get_temp_dir_name() |> Uri.fromPath;

let initData =
  InitData.create(
    ~version="9.9.9",
    ~parentPid,
    ~logsLocation,
    ~logFile,
    extensions,
  );

let handler = msg => {
  Msg.show(msg) |> prerr_endline;
  None;
};

let onError = prerr_endline;

let pipe = NamedPipe.create("hello");

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
      ("VSCODE_PARENT_PID", parentPid |> string_of_int),
      ...env,
    ],
    ~redirect=[
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
    ],
    nodePath,
    [nodePath, ...args],
  )
  |> Result.get_ok;
};
let onExit = (_, ~exit_status as _: int64, ~term_signal as _: int) => ();

spawnNode(
  ~onExit,
  ~args=[
    Rench.Path.join(
      Sys.getcwd(),
      "node/node_modules/@onivim/vscode-exthost/out/bootstrap-fork.js",
    ),
  ],
);

Luv.Loop.run() |> ignore;
