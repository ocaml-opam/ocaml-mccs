let detect_cc =
  let clang = Str.regexp "clang"
  and gcc = Str.regexp "Free Software Foundation" in
  fun () ->
  let ch = Unix.open_process_in (Config.c_compiler ^ " --version") in
  let rec aux () =
    let line = input_line ch in
    try Str.search_forward clang line 0 |> ignore; `Clang
    with Not_found ->
    try Str.search_forward gcc line 0 |> ignore; `Gcc
    with Not_found ->
    aux ()
  in
  let cc = try aux () with End_of_file -> `Unknown in
  Unix.close_process_in ch |> ignore;
  cc

let () =
  match Sys.argv.(1) with
  | "clibs" ->
     if Sys.win32 && Config.ccomp_type = "msvc" then
       print_string "(glpk_4_65.lib)"
     else
       let glpk =
         if Array.length Sys.argv > 2 && Sys.argv.(2) = "static" then
           (* flexlink doesn't (yet) support -l: *)
           if Sys.win32 then
             "libglpk.a"
           else
             "-l:libglpk.a"
         else
           "-lglpk"
       in
       Printf.printf "(-lstdc++ %s)" glpk
  | "cflags" ->
     if Config.ccomp_type = "cc" then
       match detect_cc () with
       | `Gcc -> print_string "(-Wall -Wno-parentheses -Wno-stringop-overflow)"
       | `Clang | `Unknown -> print_string "(-Wall -Wno-parentheses)"
     else
       print_string "()"
  | _ ->
     Printf.eprintf "Unrecognised context instruction: %s\n" Sys.argv.(1);
     exit 1
