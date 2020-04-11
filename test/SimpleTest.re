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
}

let start = () => {
  let client = Exthost.Client.start(
  ~namedPipe="/tmp/sock2.sock",
  ~initData,
  ~handler=_=>None,
  ~onError=msg => failwith(msg),
  (),
  )
  |> Result.get_ok;
  { client: client};
};

let finish = (expect: RelyInternal.DefaultMatchers.matchers(unit), ctx) => {
    let { client } = ctx;
    Client.close(client);
    expect.equal(1, 1); 
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
