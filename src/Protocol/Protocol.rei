module Message: {
  [@deriving (show, yojson({strict: false}))]
  type ok =
    | Empty
    | Json(Yojson.Safe.t)
    | Bytes(bytes);

  [@deriving (show, yojson({strict: false}))]
  type error =
    | Empty
    | Message(string);

  // Messages INCOMING from the node extension host -> reason
  module Incoming: {
    [@deriving (show, yojson({strict: false}))]
    type t =
      | Connected
      | Initialized
      | Ready
      | Terminate
      | RequestJSONArgs({
          requestId: int,
          rpcId: int,
          method: string,
          args: Yojson.Safe.t,
          usesCancellationToken: bool,
        })
      // TODO
      | RequestMixedArgs
      | Acknowledged({requestId: int})
      | Cancel({requestId: int})
      | ReplyOk({
          requestId: int,
          payload: ok,
        })
      | ReplyError({
          requestId: int,
          payload: error,
        })
      | Unknown(bytes)
      | Disconnected;
  };

  // Messages OUTGOING from reason -> node extension host
  module Outgoing: {
    [@deriving (show, yojson({strict: false}))]
    type t =
      | Initialize({
          requestId: int,
          initData: Extension.InitData.t,
        })
      | RequestJSONArgs({
          requestId: int,
          rpcId: int,
          method: string,
          args: Yojson.Safe.t,
          usesCancellationToken: bool,
        })
      | ReplyOKEmpty({requestId: int})
      | ReplyOKJSON({
          requestId: int,
          json: Yojson.Safe.t,
        })
      | ReplyError({
          requestId: int,
          error: string,
        })
      | Terminate;
  };

  let ofPacket: Transport.Packet.t => result(Incoming.t, string);
  let toPacket: (~id: int, Outgoing.t) => Transport.Packet.t;
};

type t;

let start:
  (
    ~namedPipe: string,
    ~dispatch: Message.Incoming.t => unit,
    ~onError: string => unit
  ) =>
  result(t, string);

let send: (~message: Message.Outgoing.t, t) => unit;

let close: t => unit;
