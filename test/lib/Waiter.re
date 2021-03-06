let wait = (~timeout=5.0, ~name="TODO", condition) => {
  let start = Unix.gettimeofday();
  let delta = () => Unix.gettimeofday() -. start;

  while (!condition() && delta() < timeout) {
    let _: bool = Luv.Loop.run(~mode=`NOWAIT, ());
    Unix.sleepf(0.1);
  };

  if (!condition()) {
    failwith("Condition failed: " ++ name);
  };
};

let waitForCollection = (~name, item) => {
  let collected = ref(false);

  let checkCollected = () => {
    Gc.full_major();
    collected^ == true;
  };

  Gc.finalise_last(() => collected := true, item);
  wait(~name="Waiting for GC to collect: " ++ name, checkCollected);
};
