let () =
  if Array.length Sys.argv <> 2 then begin
    Printf.eprintf "Need lp file as argument\n";
    exit 2
  end else
    let c = Unix.open_process_out "cbc" in
    Printf.fprintf c "import %s\n\
                      solve\n\
                      solution $\n" Sys.argv.(1);
    flush c;
    match Unix.close_process_out c with
    | WEXITED code
    | WSIGNALED code
    | WSTOPPED code -> exit code
