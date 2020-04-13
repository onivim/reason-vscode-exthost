open TestFramework;
open Exthost;
open Exthost.Types;

let initData =
  InitData.create(
    ~version="9.9.9",
    ~parentPid=1,
    ~logsLocation=Uri.fromPath("/tmp/loggy"),
    ~logFile=Uri.fromPath("/tmp/log-file"),
    [],
  );

type context = {
  client: Client.t,
  loop: Luv.Loop.t,
}

let start = () => {
  let loop = Luv.Loop.default();

  let namedPipe = NamedPipe.create("test");
  let client = Exthost.Client.start(
  ~namedPipe,
  ~initData,
  ~handler={(msg)=>{prerr_endline(Msg.show(msg)); None}},
  ~onError=msg => failwith(msg),
  (),
  )
  |> Result.get_ok;

  let redirect = [
    Luv.Process.inherit_fd(~fd=0, ~from_parent_fd=0, ()),
    Luv.Process.inherit_fd(~fd=1, ~from_parent_fd=1, ()),
    Luv.Process.inherit_fd(~fd=2, ~from_parent_fd=2, ()),
  ];
  prerr_endline ("here!");
  let environment =[
  ("PATH", Sys.getenv("PATH")),
  ("AMD_ENTRYPOINT", "vs/workbench/services/extensions/node/extensionHostProcess"),
  ("VSCODE_IPC_HOOK_EXTHOST", namedPipe |> NamedPipe.toString)
  ];
  let proc = Luv.Process.spawn(~environment,~working_directory="/Users/bryphe/vscode-exthost-v2", ~redirect, "/usr/local/bin/node", ["/usr/local/bin/node", "/Users/bryphe/vscode-exthost-v2/out/bootstrap-fork.js"])
  let proc = switch (proc) {
  | Error(err) => 
  let msg = Luv.Error.strerror(err);
  prerr_endline(msg); failwith(msg);
  | Ok(v) => v;
  }
  //|> Result.get_ok;
  prerr_endline ("here2");

  { client: client, loop: loop };
};

let finish = (expect: RelyInternal.DefaultMatchers.matchers(unit), ctx) => {
    let { client, loop } = ctx;


   while (!Luv.Loop.run(~loop, ~mode=`DEFAULT, ())) {
  
   }
    //Client.close(client);
  let done_ = Luv.Loop.run(~loop, ~mode=`NOWAIT, ());
};

describe("initial test", ({test, _}) => {
  test("test", ({expect}) => {
    expect.equal(1, 1)
  })

  test("Connect / Disconnect Test", ({expect}) => {
  
    start()
    |> finish(expect);
  });
});
