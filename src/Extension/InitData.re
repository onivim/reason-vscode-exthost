open Types;

module Extension = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    identifier: string,
    extensionLocation: Uri.t,
    name: string,
    main: option(string),
    version: string,
    engines: string,
    activationEvents: list(string),
    extensionDependencies: list(string),
    extensionKind: string,
    enableProposedApi: bool,
  };

  let ofManifestAndPath = (manifest: Manifest.t, path: string) => {
    // TODO: Is identifier right?
    identifier: manifest.name,
    extensionLocation: path |> Uri.fromPath,
    name: manifest.name,
    main: manifest.main,
    version: manifest.version,
    engines: manifest.engines,
    activationEvents: manifest.activationEvents,
    extensionDependencies: manifest.extensionDependencies,
    // TODO: Convert correctly
    extensionKind: "ui",
    enableProposedApi: manifest.enableProposedApi,
  };
};

module Environment = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    isExtensionDevelopmentDebug: bool,
    appName: string,
    // TODO
    /*
     appRoot: option(Uri.t),
     appLanguage: string,
     appUriScheme: string,
     appSettingsHome: option(Uri.t),
     globalStorageHome: Uri.t,
     userHome: Uri.t,
     webviewResourceRoot: string,
     webviewCspSource: string,
     useHostProxy: boolean,
     */
  };

  let default = {
    isExtensionDevelopmentDebug: false,
    appName: "reason-vscode-exthost",
  };
};

[@deriving (show, yojson({strict: false}))]
module Remote = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    isRemote: bool,
    // TODO:
    // authority: string,
  };

  let default = {isRemote: false};
};

[@deriving (show, yojson({strict: false}))]
type t = {
  version: string,
  parentPid: int,
  extensions: list(Extension.t),
  resolvedExtensions: list(unit),
  hostExtensions: list(unit),
  environment: Environment.t,
  logLevel: int,
  logsLocation: Uri.t,
  logFile: Uri.t,
  autoStart: bool,
  remote: Remote.t,
};

let create =
    (
      ~version,
      ~parentPid,
      ~logsLocation,
      ~logFile,
      ~environment=Environment.default,
      ~logLevel=0,
      ~autoStart=true,
      ~remote=Remote.default,
      extensions,
    ) => {
  version,
  parentPid,
  logLevel,
  extensions,
  resolvedExtensions: [],
  hostExtensions: [],
  environment,
  logsLocation,
  logFile,
  autoStart,
  remote,
};
