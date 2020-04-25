type t = string;

module Internal = {
  let id = ref(0);

  let nextId = () => {
    incr(id);
    id^ |> string_of_int;
  };
};

let create = pipeName =>
  if (Sys.win32) {
    Printf.sprintf("\\\\.\\pipe\\%s%s", pipeName, Internal.nextId());
  } else {
    let name = Filename.temp_file("exthost-", "-sock" ++ Internal.nextId());
    Unix.unlink(name);
    name;
  };

let toString = v => v;
