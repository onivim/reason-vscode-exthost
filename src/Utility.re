let getNodePath = () => {
  let ic =
    Unix.open_process_args_in(
      "node",
      [|"node", "-e", "console.log(process.execPath)"|],
    );
  let nodePath = input_line(ic);
  let _ = close_in(ic);
  nodePath |> String.trim;
};
