type handler('a) = (string, Yojson.Safe.t) => result('a, string);
type mapper('a) = 'a => Msg.t;

type t =
  | MainThreadHandler({
      handler: handler('a),
      mapper: mapper('a),
      id: int,
      name: string,
    })
    : t
  | ExtHostHandler({
      id: int,
      name: string,
    })
    : t;

let getName =
  fun
  | MainThreadHandler({name, _}) => name
  | ExtHostHandler({name}) => name;

let getId =
  fun
  | MainThreadHandler({id, _}) => id
  | ExtHostHandler({id, _}) => id;

let setId = (~id) =>
  fun
  | MainThreadHandler(main) => MainThreadHandler({...main, id})
  | ExtHostHandler(ext) => ExtHostHandler({...ext, id});

let defaultHandler = (method, _json) => {
  Error(Printf.sprintf("No handler registered for %s\n", method));
};

let defaultMapper = () => Msg.Unknown;

let mainNotImplemented = (name) => {
  MainThreadHandler({id: (-1), name, handler: defaultHandler, mapper: defaultMapper});
};

let main = (~handler, ~mapper, name) => {
  MainThreadHandler({id: (-1), name, handler, mapper});
};

let ext = name => {
  ExtHostHandler({id: (-1), name});
};

/**
 *
 * MUST BE KEPT IN SYNC WITH:
 * https://github.com/microsoft/vscode/blob/8ceb90a807cf96d34bfbe8b048f30e5a7bc50fd2/src/vs/workbench/api/common/extHost.protocol.ts
 *
 * IDs are generated sequentially in both that code and this code - so if they are in sync, the ids will line up.
 * Important for mapping string <-> ids for extension capabilities.
 */
let handlers =
  [
    mainNotImplemented("MainThreadAuthentication"),
    mainNotImplemented("MainThreadClipboard"),
    main(~handler=Commands.handle, ~mapper=msg=>Msg.Commands(msg), "MainThreadCommands"),
    mainNotImplemented("MainThreadComments"),
    mainNotImplemented("MainThreadConfiguration"),
    mainNotImplemented("MainThreadConsole"),
    main(~handler=DebugService.handle, ~mapper=msg => Msg.DebugService(msg),"MainThreadDebugService"),
    mainNotImplemented("MainThreadDecorations"),
    mainNotImplemented("MainThreadDiagnostics"),
    mainNotImplemented("MainThreadDialogs"),
    mainNotImplemented("MainThreadDocuments"),
    mainNotImplemented("MainThreadDocumentContentProviders"),
    mainNotImplemented("MainThreadTextEditors"),
    mainNotImplemented("MainThreadEditorInsets"),
    mainNotImplemented("MainThreadErrors"),
    mainNotImplemented("MainThreadTreeViews"),
    mainNotImplemented("MainThreadDownloadService"),
    mainNotImplemented("MainThreadKeytar"),
    mainNotImplemented("MainThreadLanguageFeatures"),
    mainNotImplemented("MainThreadLanguages"),
    mainNotImplemented("MainThreadLog"),
    mainNotImplemented("MainThreadMessageService"),
    mainNotImplemented("MainThreadOutputService"),
    mainNotImplemented("MainThreadProgress"),
    mainNotImplemented("MainThreadQuickOpen"),
    mainNotImplemented("MainThreadStatusBar"),
    mainNotImplemented("MainThreadStorage"),
    main(
      ~handler=Telemetry.handle,
      ~mapper=msg => Msg.Telemetry(msg),
      "MainThreadTelemetry",
    ),
    mainNotImplemented("MainThreadTerminalService"),
    mainNotImplemented("MainThreadWebviews"),
    mainNotImplemented("MainThreadUrls"),
    mainNotImplemented("MainThreadWorkspace"),
    mainNotImplemented("MainThreadFileSystem"),
    mainNotImplemented("MainThreadExtensionService"),
    mainNotImplemented("MainThreadSCM"),
    mainNotImplemented("MainThreadSearch"),
    mainNotImplemented("MainThreadTask"),
    mainNotImplemented("MainThreadWindow"),
    mainNotImplemented("MainThreadLabelService"),
    mainNotImplemented("MainThreadNotebook"),
    mainNotImplemented("MainThreadTheming"),
    mainNotImplemented("MainThreadTunnelService"),
    mainNotImplemented("MainThreadTimeline"),
    ext("ExtHostCommands"),
    ext("ExtHostConfiguration"),
    ext("ExtHostDiagnostics"),
    ext("ExtHostDebugService"),
    ext("ExtHostDecorations"),
    ext("ExtHostDocumentsAndEditors"),
    ext("ExtHostDocuments"),
    ext("ExtHostDocumentContentProviders"),
    ext("ExtHostDocumentSaveParticipant"),
    ext("ExtHostEditors"),
    ext("ExtHostTreeViews"),
    ext("ExtHostFileSystem"),
    ext("ExtHostFileSystemEventService"),
    ext("ExtHostLanguageFeatures"),
    ext("ExtHostQuickOpen"),
    ext("ExtHostExtensionService"),
    ext("ExtHostLogService"),
    ext("ExtHostTerminalService"),
    ext("ExtHostSCM"),
    ext("ExtHostSearch"),
    ext("ExtHostTask"),
    ext("ExtHostWorkspace"),
    ext("ExtHostWindow"),
    ext("ExtHostWebviews"),
    ext("ExtHostEditorInsets"),
    ext("ExtHostProgress"),
    ext("ExtHostComments"),
    ext("ExtHostStorage"),
    ext("ExtHostUrls"),
    ext("ExtHostOutputService"),
    ext("ExtHosLabelService"), // SIC
    ext("ExtHostNotebook"),
    ext("ExtHostTheming"),
    ext("ExtHostTunnelService"),
    ext("ExtHostAuthentication"),
    ext("ExtHostTimeline"),
  ]
  |> List.mapi((idx, v) => setId(~id=idx + 1, v));

module Internal = {
  let stringToId =
    handlers
    |> List.map(handler => (getName(handler), getId(handler)))
    |> List.to_seq
    |> Hashtbl.of_seq;

  let idToHandler =
    handlers
    |> List.map(handler => (getId(handler), handler))
    |> List.to_seq
    |> Hashtbl.of_seq;
};

let stringToId = Hashtbl.find_opt(Internal.stringToId);

let handle = (rpcId, method, args) => {
  rpcId
  |> Hashtbl.find_opt(Internal.idToHandler)
  |> Option.to_result(
       ~none="No handler registered fol: " ++ (rpcId |> string_of_int),
     )
  |> (
    opt =>
      Result.bind(
        opt,
        fun
        | MainThreadHandler({handler, mapper, _}) => {
            handler(method, args) |> Result.map(mapper);
          }
        | _ => {
            let ret: result(Msg.t, string) =
              Error("ExtHost handler was incorrectly registered for rpcId");
            ret;
          },
      )
  );
};
