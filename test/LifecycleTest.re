open TestFramework;

open Exthost;

module InitData = Types.InitData;
module Uri = Types.Uri;
module Test = {


  type t = {
    client: Client.t,
    extensionProcess: Luv.Process.t,
  }

  let startWithExtensions = (~handler, ~onError, extensions) => {
     let nodePath = Utility.getNodePath();
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
    let pipe = NamedPipe.create("pipe-for-test");
    let pipeStr = NamedPipe.toString(pipe);
    let client = Client.start(~namedPipe=pipe, ~initData, ~handler, ~onError, ());
  };
  
};

describe("initial test", ({test, _}) => {
  test("test", ({expect}) => {
    expect.equal(0, 1)
  })
});
