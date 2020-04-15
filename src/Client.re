type reply = unit;

type t = Protocol.t;

module Log = (val Timber.Log.withNamespace("Client"));

let start =
    (
      ~initialConfiguration=Types.Configuration.empty,
      ~namedPipe,
      ~initData: Types.InitData.t,
      ~handler: Msg.t => option(reply),
      ~onError: string => unit,
      (),
    ) => {
  let protocolClient: ref(option(Protocol.t)) = ref(None);
  let send = message =>
    switch (protocolClient^) {
    | None => ()
    | Some(protocol) => Protocol.send(~message, protocol)
    };

  let dispatch = msg => {
    Protocol.Message.(
      switch (msg) {
      | Incoming.Ready =>
        Log.info("Ready");

        send(Outgoing.Initialize({requestId: 1, initData}));
        handler(Ready) |> ignore;

      | Incoming.Initialized =>
        Log.info("Initialized");

        let rpcId =
          "ExtHostConfiguration" |> Handlers.stringToId |> Option.get;

        send(
          Outgoing.RequestJSONArgs({
            requestId: 2,
            rpcId,
            method: "$initializeConfiguration",
            args:
              Types.Configuration.empty
              |> Types.Configuration.to_yojson
              |> (json => `List([json])),
            usesCancellationToken: false,
          }),
        );
        handler(Initialized) |> ignore;

        let rpcId = "ExtHostWorkspace" |> Handlers.stringToId |> Option.get;
        send(
          Outgoing.RequestJSONArgs({
            requestId: 3,
            rpcId,
            method: "$initializeWorkspace",
            args: `List([]),
            usesCancellationToken: false,
          }),
        );

      | Incoming.ReplyError({requestId, payload}) =>
        switch (payload) {
        | Message(str) => onError(str)
        | Empty => onError("Unknown / Empty")
        }
      | Incoming.RequestJSONArgs({requestId, rpcId, method, args}) =>
        Handlers.handle(rpcId, method, args)
        |> Result.iter(msg => handler(msg) |> ignore);
        send(Outgoing.ReplyOKEmpty({requestId: requestId}));
      | _ => ()
      }
    );
  };

  let protocol = Protocol.start(~namedPipe, ~dispatch, ~onError);

  protocol |> Result.iter(pc => protocolClient := Some(pc));

  protocol;
};

let close = Protocol.close;
