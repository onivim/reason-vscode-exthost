let getNodePath = () => {
        let ic = Unix.open_process_in("node -e 'console.log(process.execPath)'");
        let nodePath = input_line(ic);
        let _ = close_in(ic);
        nodePath |> String.trim;
};
