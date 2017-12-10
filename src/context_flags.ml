#directory "+compiler-libs";;
#load "ocamlcommon.cma";;

match Sys.argv.(1) with
| "cxxflags" ->
    if Sys.win32 && Config.ccomp_type = "msvc" then
      print_string "(/EHsc)"
    else
      print_string "()"
| "clibs" ->
    if Config.ccomp_type = "cc" then
      print_string "(-lstdc++)"
    else
      print_string "()"
| _ ->
    Printf.eprintf "Unrecognised context instruction: %s\n" Sys.argv.(1);
    exit 1
