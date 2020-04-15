open TestFramework;



let spawnNode = (~onExit, ~args) => {
  Luv.Process.spawn(
    ~on_exit=onExit,
    ~redirect=[
        Luv.Process.inherit_fd(~fd=Luv.Process.stdin, ~from_parent_fd=Luv.Process.stdin, ()),
        Luv.Process.inherit_fd(~fd=Luv.Process.stdout, ~from_parent_fd=Luv.Process.stderr, ()),
        Luv.Process.inherit_fd(~fd=Luv.Process.stderr, ~from_parent_fd=Luv.Process.stderr, ()),
    ],
    "node",
    ["node", ...args]
  ) |> Result.get_ok;
};

let wait = (condition) => {
  let start = Unix.gettimeofday();
  let delta = () => Unix.gettimeofday() -. start;

  while(!condition() && delta() < 1.0) {
    let _: bool = Luv.Loop.run(~mode=`ONCE, ());
    Unix.sleepf(0.1);
  }
};

describe("Transport", ({describe, _}) => {
  describe("process sanity checks", ({test, _}) => {
    test("start node", ({expect}) => {
      let exits = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => exits := true;
      let _ = spawnNode(~onExit, ~args=["--version"]);

      wait(() => exits^ == true);
    })
    
    test("node process GC", ({expect}) => {
      let collected = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => ();
      let proc = spawnNode(~onExit, ~args=["--version"]);

      Gc.finalise_last(() => collected := true, proc);

      wait(() => {
        Gc.full_major();
        collected^ == true;
      })
    })
  });
});
