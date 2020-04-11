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
  let loop = Luv.Loop.init() |> Result.get_ok;

  let client = Exthost.Client.start(
  ~namedPipe=NamedPipe.create("test"),
  ~initData,
  ~handler=_=>None,
  ~onError=msg => failwith(msg),
  (),
  )
  |> Result.get_ok;

  let redirect = [
    Luv.Process.inherit_fd(~fd=0, ~from_parent_fd=0, ()),
    Luv.Process.inherit_fd(~fd=1, ~from_parent_fd=1, ()),
    Luv.Process.inherit_fd(~fd=2, ~from_parent_fd=2, ()),
  ];
  let proc = Luv.Process.spawn(~redirect, "node", ["node", "--version"])
  |> Result.get_ok;

  let done_ = Luv.Loop.run(~loop, ~mode=`DEFAULT, ());
  prerr_endline ("Done? " ++ (done_ ? "true": "false"));
  { client: client, loop: loop };
};

let finish = (expect: RelyInternal.DefaultMatchers.matchers(unit), ctx) => {
    let { client, loop } = ctx;
  let done_ = Luv.Loop.run(~loop, ~mode=`NOWAIT, ());
  prerr_endline ("Done? " ++ (done_ ? "true": "false"));
    Client.close(client);
    expect.equal(1, 1); 
    let { client, loop } = ctx;
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
