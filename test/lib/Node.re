let spawn = (~additionalEnv=[], ~onExit, args) => {
  let env = Luv.Env.environ() |> Result.get_ok;
  let nodeFullPath = Exthost.Utility.getNodePath();
  print_endline("Using node path: " ++ nodeFullPath);
  Luv.Process.spawn(
    ~on_exit=onExit,
    ~environment=env @ additionalEnv,
    ~redirect=[
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stdin,
        ~from_parent_fd=Luv.Process.stdin,
        (),
      ),
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stdout,
        ~from_parent_fd=Luv.Process.stderr,
        (),
      ),
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stderr,
        ~from_parent_fd=Luv.Process.stderr,
        (),
      ),
    ],
    nodeFullPath,
    [nodeFullPath, ...args],
  )
  |> Result.get_ok;
};