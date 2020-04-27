open TestFramework;

open Exthost;

describe("ActivationEventsTest", ({test, _}) => {
  describe("* (wildcard activation)", _ => {
    test("close - extensions", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(OnDidActivateExtension({extensionId, _})) =>
          String.equal(extensionId, "oni-always-activate")
        | _ => false;

      Test.startWithExtensions(["oni-always-activate"])
      |> Test.waitForReady
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  })
  /*describe("onCommand", _ => {
    test("onCommand:extension.helloWorld", _ => {
      let waitForActivation =
        fun
        | Msg.ExtensionService(OnDidActivateExtension({extensionId, _})) =>
          extensionId == "oni-activation-events"
        | _ => false;

      Test.startWithExtensions(["oni-activation-events"])
      |> Test.waitForReady
      |> Test.executeContributedCommand(~command="extension.helloWorld")
      |> Test.waitForMessage(~name="Activation", waitForActivation)
      |> Test.terminate
      |> Test.waitForProcessClosed;
    })
  })*/
});
