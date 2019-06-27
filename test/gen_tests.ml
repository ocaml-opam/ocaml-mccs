let backends =
  try
    let be = Sys.getenv "MCCS_BACKENDS" in
    let bel = String.length be in
    let rec aux i =
      if i >= bel then [] else
      try
        let j = String.index_from be i ' ' in
        String.sub be i (j - i) :: aux (j+1)
      with Not_found -> [String.sub be i (String.length be - i)]
    in List.filter (( <> ) "") (aux 0)
  with Not_found -> ["GLPK"]

let test name ?solver ?ignore ?(ref="") () =
  let solver =
    match solver with
    | None -> ""
    | Some solver -> Printf.sprintf " \"%s\"" solver
  in
  let ignore_eol =
    (* This means that an LF check-out on Windows won't cause a problem with
       CRLF output from the test program. Assume GNU diff present on Windows. *)
    if Sys.win32 || Sys.cygwin then
      "--ignore-trailing-space "
    else
      ""
  in
  let () =
    match ignore with
    | None ->
        Printf.printf "
(rule
  (with-stdout-to test-%s.result (run %%{exe:mccs_test.exe} %%{dep:test.cudf}%s)))

(rule
  (with-stdout-to test.%s.reference (cat %%{dep:test%s.output})))" name solver name ref
    | Some ignore ->
        Printf.printf "
(rule
  (with-stdout-to test-%s.raw (run %%{exe:mccs_test.exe} %%{dep:test.cudf}%s)))

(rule
  (with-stdout-to test-%s.result (system \"grep -v %s %%{dep:test-%s.raw}\")))

(rule
  (with-stdout-to test.%s.reference (progn (cat %%{dep:test.%s}) (cat %%{dep:test%s.output}))))" name solver name ignore name name name ref
  in
  Printf.printf "

(alias
 (name runtest)
 (action (system \"diff %s%%{dep:test-%s.result} %%{dep:test.%s.reference}\")))\n" ignore_eol name name

let () =
  print_endline "; This file is generated using `MCCS_BACKENDS=jbuilder build @settests --auto-promote`";
  let () =
    try
      let lp_solver = Sys.getenv "MCCS_LPSOLVER" |> Printf.sprintf "%S" in
      let solver = "lp+" ^ String.sub lp_solver 1 (String.length lp_solver - 2) in
      test "lp" ~solver ();
    with Not_found -> ()
  in
  if List.mem "GLPK" backends
  then test "glpk" ();
  if List.mem "SYMPHONY" backends
  then test "symphony" ~solver:"coin/symphony" ();
  (* These backends aren't stable enough for CI yet...
  if List.mem "CLP" backends
  then test "clp" ~solver:"coin/clp" ~ignore:"Clp0032I" ~ref:".cl" ();
  if List.mem "CBC" backends
  then test "cbc" ~solver:"coin/cbc" ~ignore:"'Coin3007W\\|Clp0032I'" ~ref:".cl" ()
  *)
