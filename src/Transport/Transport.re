module Packet = {
  module Constants = {
    let headerByteLength = 13;
  };

  type packetType =
    | Unspecified
    | Regular
    | Control
    | Ack
    | KeepAlive
    | Disconnect;

  let typeOfInt =
    fun
    | 0 => Ok(Unspecified)
    | 1 => Ok(Regular)
    | 2 => Ok(Control)
    | 3 => Ok(Ack)
    | 4 => Ok(KeepAlive)
    | 5 => Ok(Disconnect)
    | v => Error(Printf.sprintf("Unknown packet type: %d", v));

  let typeToInt =
    fun
    | Unspecified => 0
    | Regular => 1
    | Control => 2
    | Ack => 3
    | KeepAlive => 4
    | Disconnect => 5;

  let typeToString =
    fun
    | Unspecified => "Unspecified"
    | Regular => "Regular"
    | Control => "Control"
    | Ack => "Ack"
    | KeepAlive => "KeepAlive"
    | Disconnect => "Disconnect";

  // The header for a packet is defined as follows by the VSCode extension host:
  // Byte 0 - uint8 - message type
  // Byte 1 - uint32 (big endian) - id
  // Byte 5 - uint32 (big endian) - ack
  // Byte 9 - unit32 (big endian) - data length
  module Header = {
    type t = {
      packetType,
      id: int,
      ack: int,
      length: int,
    };

    let ofBytes = bytes =>
      if (Bytes.length(bytes) != Constants.headerByteLength) {
        Error(
          Printf.sprintf("Incorrect header size: %d", Bytes.length(bytes)),
        );
      } else {
        let packetTypeUint = Bytes.get_uint8(bytes, 0);
        let id = Bytes.get_int32_be(bytes, 1) |> Int32.to_int;
        let ack = Bytes.get_int32_be(bytes, 5) |> Int32.to_int;
        let length = Bytes.get_int32_be(bytes, 9) |> Int32.to_int;

        packetTypeUint
        |> typeOfInt
        |> Result.map(packetType => {packetType, id, ack, length});
      };

    let toString = ({packetType, id, ack, length}) =>
      Printf.sprintf(
        "Type: %s Id: %d Ack: %d Length: %d",
        packetType |> typeToString,
        id,
        ack,
        length,
      );

    let toBytes = ({packetType, id, ack, length}) => {
      let bytes = Bytes.create(Constants.headerByteLength);
      let packetTypeInt = packetType |> typeToInt;
      Bytes.set_uint8(bytes, 0, packetTypeInt);
      Bytes.set_int32_be(bytes, 1, id |> Int32.of_int);
      Bytes.set_int32_be(bytes, 5, ack |> Int32.of_int);
      Bytes.set_int32_be(bytes, 9, length |> Int32.of_int);
      bytes;
    };
  };

  type t = {
    header: Header.t,
    body: Bytes.t,
  };

  let create = (~bytes: Bytes.t, ~packetType, ~id: int) => {
    let length = Bytes.length(bytes);
    let header =
      Header.{
        packetType,
        id,
        // TODO: Set up ack
        ack: 0,
        length,
      };

    {header, body: bytes};
  };

  let toBytes = ({header, body}) => {
    let headerBytes = header |> Header.toBytes;
    Bytes.cat(headerBytes, body);
  };

  let toString = ({header, body}) =>
    Printf.sprintf(
      "[Header]: %s\n [Body]:\n---|%s|---\n",
      header |> Header.toString,
      body |> Bytes.to_string,
    );

  module Parser = {
    type state =
      | WaitingForHeader
      | WaitingForBody(Header.t);

    type t = {
      state,
      bytes: Bytes.t,
    };

    let initial = {state: WaitingForHeader, bytes: Bytes.create(0)};

    let addBytes = (bytes: Bytes.t, parser) => {
      let bytes = Bytes.cat(parser.bytes, bytes);
      {...parser, bytes};
    };

    let parseOne = ({state, bytes as accumulatedBytes}: t) => {
      let totalLen = Bytes.length(bytes);

      switch (state) {
      | WaitingForHeader =>
        if (totalLen >= Constants.headerByteLength) {
          let headerBytes =
            Bytes.sub(accumulatedBytes, 0, Constants.headerByteLength);
          let remainingBytes =
            Bytes.sub(
              accumulatedBytes,
              Constants.headerByteLength,
              totalLen - Constants.headerByteLength,
            );

          let header = Header.ofBytes(headerBytes) |> Result.get_ok;

          ({state: WaitingForBody(header), bytes: remainingBytes}, None);
        } else {
          (
            // We didn't get enough bytes to parse the header - so keep accumulating
            {state: WaitingForHeader, bytes: accumulatedBytes},
            None,
          );
        }
      | WaitingForBody(header) =>
        let {length, _}: Header.t = header;

        // Do we have enough bytes?
        if (totalLen >= length) {
          let body = Bytes.sub(accumulatedBytes, 0, length);
          let packet = {header, body};
          let remainingBytes =
            Bytes.sub(accumulatedBytes, length, totalLen - length);
          ({state: WaitingForHeader, bytes: remainingBytes}, Some(packet));
        } else {
          ({state: WaitingForBody(header), bytes: accumulatedBytes}, None);
        };
      };
    };

    let parse = (bytes, initialParser) => {
      let parser = addBytes(bytes, initialParser);

      let canParseMore = parser => {
        switch (parser.state) {
        | WaitingForHeader =>
          Bytes.length(parser.bytes) >= Constants.headerByteLength
        | WaitingForBody({length, _}) => Bytes.length(parser.bytes) >= length
        };
      };

      let rec loop = (parser, messages) =>
        if (canParseMore(parser)) {
          let (newParser, maybeMessage) = parseOne(parser);
          let newMessages =
            switch (maybeMessage) {
            | Some(msg) => [msg, ...messages]
            | None => messages
            };

          loop(newParser, newMessages);
        } else {
          (parser, messages);
        };

      let (parser, revMessages) = loop(parser, []);
      let messages = List.rev(revMessages);
      (parser, messages);
    };
  };
};

type msg =
  | Connected
  | Received(Packet.t)
  | Error(string)
  | Disconnected
  | Closing;

type t = {
  server: Luv.Pipe.t,
  maybeClient: ref(option(Luv.Pipe.t)),
};

let start = (~namedPipe: string, ~dispatch: msg => unit) => {
  let maybeClient = ref(None);

  let handleError = (msg, err) => {
    let msg = Printf.sprintf("%s: %s\n", msg, Luv.Error.strerror(err));
    dispatch(Error(msg));
  };

  let read = clientPipe => {
    let parser = ref(Packet.Parser.initial);

    let readBuffer = (buffer: Luv.Buffer.t) => {
      let bytes = Luv.Buffer.to_bytes(buffer);

      let (newParser, packets) =
        try(Packet.Parser.parse(bytes, parser^)) {
        // TODO: Proper exception
        | exn =>
          dispatch(Error(Printexc.to_string(exn)));
          (Packet.Parser.initial, []);
        };

      parser := newParser;
      packets |> List.iter(packet => dispatch(Received(packet)));
    };

    maybeClient := Some(clientPipe);
    let handleClosed = () => {
      maybeClient := None;
      dispatch(Disconnected);
      Luv.Handle.close(clientPipe, ignore);
    };

    Luv.Stream.read_start(
      clientPipe,
      fun
      | Error(`EOF) => handleClosed()
      | Error(msg) => handleError("read_start", msg)
      | Ok(buffer) => readBuffer(buffer),
    );
  };

  // Listen for an incoming connection...
  let listen = serverPipe => {
    prerr_endline("Listening...");
    Luv.Pipe.bind(serverPipe, namedPipe) |> ignore;
    Luv.Stream.listen(
      serverPipe,
      listenResult => {
        prerr_endline("Got listen result");
        // Create a pipe for the client
        let clientPipeResult =
          listenResult
          |> (
            r => {
              prerr_endline("Trying to create client pipe...");
              Stdlib.Result.bind(r, _ => Luv.Pipe.init());
            }
          )
          |> (
            r => {
              Stdlib.Result.bind(r, pipe => {
                Luv.Stream.accept(~server=serverPipe, ~client=pipe)
                |> Result.map(_ => pipe)
              });
            }
          );

        switch (clientPipeResult) {
        | Ok(pipe) =>
          dispatch(Connected);
          read(pipe);
        | Error(err) => handleError("listen", err)
        };
      },
    );
  };

  let serverPipeResult =
    Luv.Pipe.init() |> Result.map_error(Luv.Error.strerror);

  serverPipeResult |> Result.iter(listen);

  serverPipeResult |> Result.map(server => {server, maybeClient});
};

let send = (~packet, {maybeClient}) =>
  switch (maybeClient^) {
  | None => prerr_endline("No client, yet!")
  | Some(c) =>
    let bytes = Packet.toBytes(packet);
    let byteLen = bytes |> Bytes.length;
    prerr_endline(Printf.sprintf("Sending %d bytes...", byteLen));
    let buffer = Luv.Buffer.from_bytes(bytes);
    // TODO: FIX PENDING BYTES
    Luv.Stream.write(c, [buffer], (err, count) => {
      prerr_endline(Printf.sprintf("Wrote %d bytes...", count))
    });
  };

let close = ({server, _}) => Luv.Handle.close(server, ignore);
