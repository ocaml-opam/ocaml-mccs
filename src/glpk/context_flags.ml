#directory "+compiler-libs";;
#load "ocamlcommon.cma";;

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
      print_string "(-Wno-parentheses -Wno-stringop-overflow)"
    else
      print_string "()"
| _ ->
    Printf.eprintf "Unrecognised context instruction: %s\n" Sys.argv.(1);
    exit 1
