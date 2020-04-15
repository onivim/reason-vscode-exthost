open TestFramework;
open Exthost;

module Header = Transport.Packet.Header;
module Packet = Transport.Packet;

let spawnNode = (~onExit, ~args) => {
  Luv.Process.spawn(
    ~on_exit=onExit,
    ~redirect=[
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stdin,
        ~from_parent_fd=Luv.Process.stdin,
        (),
      ),
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stdout,
        ~from_parent_fd=Luv.Process.stderr,
        (),
      ),
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stderr,
        ~from_parent_fd=Luv.Process.stderr,
        (),
      ),
    ],
    "node",
    ["node", ...args],
  )
  |> Result.get_ok;
};

let wait = condition => {
  let start = Unix.gettimeofday();
  let delta = () => Unix.gettimeofday() -. start;

  while (!condition() && delta() < 1.0) {
    let _: bool = Luv.Loop.run(~mode=`NOWAIT, ());
    Unix.sleepf(0.1);
  };

  if (!condition()) {
    failwith("Condition failed!");
  };
};

describe("Transport", ({describe, _}) => {
  describe("process sanity checks", ({test, _}) => {
    test("start node", ({expect}) => {
      let exits = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => exits := true;
      let _ = spawnNode(~onExit, ~args=["--version"]);

      wait(() => exits^ == true);
    });

    test("node process GC", ({expect}) => {
      let collected = ref(false);

      let checkCollected = () => {
        Gc.full_major();
        collected^ == true;
      };

      let exits = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => exits := true;
      let proc = spawnNode(~onExit, ~args=["--version"]);
      wait(() => exits^ == true);
      //Gc.finalise_last(() => collected := true, proc);
      //wait(checkCollected);
    });
  });

  describe("server", ({test, _}) => {
    test("disconnect from server--side", ({expect}) => {
      let namedPipe = NamedPipe.create("server-test") |> NamedPipe.toString;

      let messages = ref([]);

      let dispatch = msg => messages := [msg, ...messages^];

      let exits = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => exits := true;
      let _ = spawnNode(~onExit, ~args=["node/client.js", namedPipe]);
      let transport = Transport.start(~namedPipe, ~dispatch) |> Result.get_ok;

      wait(() => messages^ |> List.exists(msg => msg == Transport.Connected));

      Transport.close(transport);

      wait(() => {exits^});

      /*wait(() => {
          messages^ |> List.exists(msg => msg == Transport.Disconnected)
        });*/

      let collected = ref(false);

      Gc.finalise_last(() => collected := true, transport);
      wait(() => {
        Gc.full_major();
        collected^;
      });

      expect.equal(exits^, true);
    });
    test("disconnect from client-side", ({expect}) => {
      let namedPipe = NamedPipe.create("server-test") |> NamedPipe.toString;

      let messages = ref([]);

      let dispatch = msg => messages := [msg, ...messages^];

      let exits = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => exits := true;
      let _ =
        spawnNode(
          ~onExit,
          ~args=["node/immediate-disconnect-client.js", namedPipe],
        );
      let transport = Transport.start(~namedPipe, ~dispatch) |> Result.get_ok;

      wait(() => messages^ |> List.exists(msg => msg == Transport.Connected));

      wait(() => {exits^});

      wait(() => {
        messages^ |> List.exists(msg => msg == Transport.Disconnected)
      });

      let collected = ref(false);

      Gc.finalise_last(() => collected := true, transport);
      wait(() => {
        Gc.full_major();
        collected^;
      });

      expect.equal(exits^, true);
    });
    test("echo", ({expect}) => {
      let namedPipe = NamedPipe.create("server-test") |> NamedPipe.toString;

      let messages = ref([]);

      let dispatch = msg => messages := [msg, ...messages^];

      let exits = ref(false);
      let onExit = (proc, ~exit_status, ~term_signal) => exits := true;
      let _ = spawnNode(~onExit, ~args=["node/echo-client.js", namedPipe]);
      let transport = Transport.start(~namedPipe, ~dispatch) |> Result.get_ok;

      wait(() => messages^ |> List.exists(msg => msg == Transport.Connected));

      let bytes = "Hello, world!" |> Bytes.of_string;
      let packet =
        Transport.Packet.create(~bytes, ~packetType=Regular, ~id=0);
      Transport.send(~packet, transport);

      wait(() => {
        messages^
        |> List.exists(
             fun
             | Transport.Received(packet) =>
               packet.Packet.body |> Bytes.to_string == "Hello, world!"
             | _ => false,
           )
      });

      let bytes = "Hello, again!" |> Bytes.of_string;
      let packet =
        Transport.Packet.create(~bytes, ~packetType=Regular, ~id=1);
      Transport.send(~packet, transport);

      wait(() => {
        messages^
        |> List.exists(
             fun
             | Transport.Received(packet) =>
               packet.Packet.body |> Bytes.to_string == "Hello, again!"
             | _ => false,
           )
      });

      Transport.close(transport);
      wait(() => {exits^});

      let collected = ref(false);

      Gc.finalise_last(() => collected := true, transport);
      wait(() => {
        Gc.full_major();
        collected^;
      });

      expect.equal(exits^, true);
    });
  });
});
