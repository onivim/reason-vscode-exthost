module Folder = {
  type t = {
    uri: Uri.t,
    name: string,
    id: string,
  };

  let to_yojson: t => Yojson.Safe.t =
    folder => {
      `Assoc([
        ("uri", Uri.to_yojson(folder.uri)),
        ("name", `String(folder.name)),
        ("id", `String(folder.id)),
      ]);
    };
};

type t = {
  id: string,
  name: string,
  folders: list(Folder.t),
};

let empty: t = {id: "No workspace", name: "No workspace", folders: []};

let create = (~folders=[], ~id, name) => {id, name, folders};

let fromUri = (~name, ~id, uri) => {id, name, folders: [{uri, name, id}]};

let fromPath = path => {
  id: path,
  name: path,
  folders: [{uri: Uri.fromPath(path), name: path, id: path}],
};

let to_yojson: t => Yojson.Safe.t =
  ws => {
    let foldersJson =
      ws.folders |> List.map(Folder.to_yojson) |> (v => `List(v));

    `Assoc([
      ("id", `String(ws.id)),
      ("name", `String(ws.name)),
      ("configuration", `Assoc([])),
      ("folders", foldersJson),
    ]);
  };
