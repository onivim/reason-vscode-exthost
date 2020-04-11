module Packet: {
  type packetType =
    | Unspecified
    | Regular
    | Control
    | Ack
    | KeepAlive
    | Disconnect;

  module Header: {
    type t = {
      packetType,
      id: int,
      ack: int,
      length: int,
    };

    let ofBytes: bytes => result(t, string);
    let toBytes: t => bytes;
    let toString: t => string;
  };

  type t = {
    header: Header.t,
    body: Bytes.t,
  };

  let create: (~bytes: Bytes.t, ~packetType: packetType, ~id: int) => t;
};

type msg =
  | Connected
  | Received(Packet.t)
  | Error(string)
  | Disconnected
  | Closing;

type t;

let start: (~namedPipe: string, ~dispatch: msg => unit) => result(t, string);

let send: (~packet: Packet.t, t) => unit;

let close: t => unit;
