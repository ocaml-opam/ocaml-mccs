#directory "+compiler-libs";;
#load "ocamlcommon.cma";;

match Sys.argv.(1) with
| "cflags" ->
    if Config.ccomp_type = "cc" then
      print_string "(-Wno-maybe-uninitialized -Wno-parentheses)"
    else
      print_string "()"
| _ ->
    Printf.eprintf "Unrecognised context instruction: %s\n" Sys.argv.(1);
    exit 1
