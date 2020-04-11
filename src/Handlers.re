type t = {
  id: int,
  name: string,
  handler: (string, Yojson.Safe.t) => result(Message.t, string),
};

let main = name => {
  id: (-1),
  name,
  handler: (method, _json) =>
    Error(Printf.sprintf("No handler registered for %s:%s\n", name, method)),
};

let ext = name => {
  id: (-1),
  name,
  handler: (method, _json) =>
    Error(Printf.sprintf("Unexpected request %s:%s\n", name, method)),
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
    main("MainThreadAuthentication"),
    main("MainThreadClipboard"),
    main("MainThreadCommands"),
    main("MainThreadComments"),
    main("MainThreadConfiguration"),
    main("MainThreadConsole"),
    main("MainThreadDebugService"),
    main("MainThreadDecorations"),
    main("MainThreadDiagnostics"),
    main("MainThreadDialogs"),
    main("MainThreadDocuments"),
    main("MainThreadDocumentContentProviders"),
    main("MainThreadTextEditors"),
    main("MainThreadEditorInsets"),
    main("MainThreadErrors"),
    main("MainThreadTreeViews"),
    main("MainThreadDownloadService"),
    main("MainThreadKeytar"),
    main("MainThreadLanguageFeatures"),
    main("MainThreadLanguages"),
    main("MainThreadLog"),
    main("MainThreadMessageService"),
    main("MainThreadOutputService"),
    main("MainThreadProgress"),
    main("MainThreadQuickOpen"),
    main("MainThreadStatusBar"),
    main("MainThreadStorage"),
    main("MainThreadTelemetry"),
    main("MainThreadTerminalService"),
    main("MainThreadWebviews"),
    main("MainThreadUrls"),
    main("MainThreadWorkspace"),
    main("MainThreadFileSystem"),
    main("MainThreadExtensionService"),
    main("MainThreadSCM"),
    main("MainThreadSearch"),
    main("MainThreadTask"),
    main("MainThreadWindow"),
    main("MainThreadLabelService"),
    main("MainThreadNotebook"),
    main("MainThreadTheming"),
    main("MainThreadTunnelService"),
    main("MainThreadTimeline"),
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
  |> List.mapi((idx, v) => {...v, id: idx + 1});

module Internal = {
  let stringToId =
    handlers
    |> List.map(({id, name, _}) => (name, id))
    |> List.to_seq
    |> Hashtbl.of_seq;
};

let stringToId = Hashtbl.find_opt(Internal.stringToId);
