open TestFramework;

describe("LifecycleTest", ({test, _}) => {
  test("close - no extensions", _ => {
    Test.startWithExtensions([])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
  test("close - extensions", _ => {
    Test.startWithExtensions(["oni-always-activate"])
    |> Test.waitForReady
    |> Test.terminate
    |> Test.waitForProcessClosed
  });
});
