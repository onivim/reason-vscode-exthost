/*
 * Workspace.rei
 *
 * Types related to modelling extension workspaces
 *
 */

module Folder: {
  type t = {
    uri: Uri.t,
    name: string,
    id: string,
  };

  let to_yojson: t => Yojson.Safe.t;
};

type t = {
  id: string,
  name: string,
  folders: list(Folder.t),
};

let create: (~folders: list(Folder.t)=?, ~id: string, string) => t;

let fromUri: (~name: string, ~id: string, Uri.t) => t;

let fromPath: string => t;

let to_yojson: t => Yojson.Safe.t;
