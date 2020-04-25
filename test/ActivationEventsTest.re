open TestFramework;

open Exthost;

describe("ActivationEventsTest", ({test, _}) => {
  describe("* (wildcard activation)", ({test, _}) => {
    test("close - extensions", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(OnDidActivateExtension({extensionId, _})) =>
          extensionId == "oni-always-activate"
        | _ => false;

      Test.startWithExtensions(["oni-always-activate"])
      |> Test.waitForReady
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  })
});
