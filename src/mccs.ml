type cudf_package = Cudf.package = {
  package : string;
  version : int;
  depends : Cudf_types.vpkgformula;
  conflicts : Cudf_types.vpkglist;
  provides : Cudf_types.veqpkglist;
  installed : bool;
  was_installed : bool;
  keep : [`Keep_version | `Keep_package | `Keep_feature | `Keep_none ];
  pkg_extra : Cudf_types.typed_value Cudf_types.stanza;
}

type preamble = Cudf.preamble = {
  preamble_id : string;
  property : Cudf_types.typedecl;
  univ_checksum: string;
  status_checksum: string;
  req_checksum: string;
}

type request = Cudf.request = {
  request_id : string;
  install : Cudf_types.vpkglist;
  remove : Cudf_types.vpkglist;
  upgrade : Cudf_types.vpkglist;
  req_extra : Cudf_types.typed_value Cudf_types.stanza;
}

type problem

external set_verbosity: int -> unit
  = "set_verbosity"

external gen_problem: preamble -> problem
  = "gen_problem"

external add_package_to_problem: problem -> cudf_package -> unit
  = "add_package_to_problem"

external set_problem_request: problem -> request -> unit
  = "set_problem_request"

external call_solver : string -> problem -> Cudf.package list option
  = "call_solver"

let problem_of_cudf cudf =
  let preamble, universe, request = cudf in
  let pb = gen_problem preamble in
  Cudf.iter_packages (add_package_to_problem pb) universe;
  set_problem_request pb request;
  pb

let resolve_cudf ?(verbose=false) criteria (preamble, _, _ as cudf) =
  set_verbosity (if verbose then 1 else 0);
  let pb = problem_of_cudf cudf in
  match call_solver criteria pb with
  | None -> None
  | Some sol ->
    let univ = Cudf.load_universe sol in
    Some (preamble, univ)
