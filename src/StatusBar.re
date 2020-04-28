
[@deriving show]
type msg =
  | SetEntry({
      id: int,
      text: string,
      alignment: int,
      priority: int,
    });

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | ("$setEntry", `List([
    `Int(id),
    _,
    `String(text),
    _,
    _,
    _,
    `Int(alignment),
    `Int(priority)
  ])) =>
    Ok(SetEntry({id, text, alignment, priority}))
  | _ => Error("Unable to parse method: " ++ method ++ " with args: " ++ Yojson.Safe.to_string(args));
  };
};
