open TestFramework;

open Exthost;

let waitForProcessExit =
  fun
  | Msg.TerminalService(TerminalService.SendProcessExit(_)) => true
  | _ => false;

describe("TerminalServiceTest", ({test, _}) => {
  test("noop process should give process exited", _ => {
    Test.startWithExtensions([])
    |> Test.waitForReady
    |> Test.withClient(
         Request.TerminalService.spawnExtHostProcess(
           ~id=0,
           ~shellLaunchConfig=
             Types.ShellLaunchConfig.{
               executable: "noop",
               arguments: [],
               name: "noop",
             },
           ~activeWorkspaceRoot=Types.Uri.fromPath(Sys.getcwd()),
           ~cols=10,
           ~rows=10,
           ~isWorkspaceShellAllowed=true,
         ),
       )
    |> Test.waitForMessage(
         ~name="TerminalService.SendProcessExit",
         waitForProcessExit,
       )
    |> Test.terminate
    |> Test.waitForProcessClosed
  })
});
