let (preamble, universe, request) as cudf =
  match Cudf_parser.load_from_file Sys.argv.(1) with
  | Some a, b, Some c -> a, b, c
  | None, b, Some c -> Cudf.default_preamble, b, c
  | _ -> assert false

(* let () =
 *   Printf.printf "## REQUEST ##\n";
 *   Cudf_printer.pp_cudf stdout (preamble, Cudf.load_universe (Mccs.get_problem_packages (Mccs.problem_of_cudf cudf)), request);
 *   Printf.printf "####\n\n%!" *)

let criteria =
  if Array.length Sys.argv <= 2 then "-removed,-changed"
  else Sys.argv.(2)

let solve () =
  Mccs.resolve_cudf ~verbose:true criteria cudf

let () =
  try
  match solve () with
  | None -> print_endline "FAIL"
  | Some sol ->
    Printf.printf "\n## SOLUTION ##\n";
    Cudf_printer.pp_solution stdout sol;
    Printf.printf "####\n%!"
  with Mccs.Timeout -> Printf.eprintf "Timeout!\n%!"
     | Sys.Break -> Printf.eprintf "User pressed CTRL+C!\n%!"
