open BenchFramework;

open Exthost.Transport.Packet;

let options = Reperf.Options.create(~iterations=100, ());

module Constants = {
  let individualPacketSize = 8192;
  let packetCount = 1000;
  let totalSize = packetCount * individualPacketSize;
};

let largeBody =
  String.make(Constants.individualPacketSize, 'a')
  |> Bytes.of_string
  |> Luv.Buffer.from_bytes;
let header =
  Header.{packetType: Regular, id: 0, ack: 0, length: Constants.totalSize}
  |> Header.toBytes
  |> Luv.Buffer.from_bytes;

let parseLargeMessage = () => {
  let (p, _messages) = Parser.initial |> Parser.parse(header);

  let parser = ref(p);

  for (_idx in 0 to Constants.packetCount - 1) {
    let p = parser^;
    let (newParser, _messages) = Parser.parse(largeBody, p);
    parser := newParser;
  };
};

let setup = () => ();

bench(
  ~name="Packet parser bench",
  ~options,
  ~setup,
  ~f=parseLargeMessage,
  (),
);
