(library
    (name ExtHostTest)
    (public_name vscode-exthost-test)
    (flags (:standard (-w -39)))
    (ocamlopt_flags -linkall -g)
    (libraries rely.lib vscode-exthost)
)
