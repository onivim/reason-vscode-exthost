print_endline("Hello, world!");

open Exthost;

module InitData = Types.InitData;
module Uri = Types.Uri;

Printexc.record_backtrace(true);

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

let client = Client.start(~namedPipe=pipe, ~initData, ~handler, ~onError, ());

Luv.Loop.run() |> ignore;
