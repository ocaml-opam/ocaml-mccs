(**************************************************************************)
(*                                                                        *)
(*    Copyright 2017-2018 OCamlPro                                        *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

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

type solver_backend = [ `GLPK | `LP of string | `COIN_CLP | `COIN_CBC | `COIN_SYMPHONY ]

let default_solver = `GLPK

exception Timeout

let () = Callback.register_exception "Sys.Break" Sys.Break
let () = Callback.register_exception "Mccs.Timeout" Timeout

external set_verbosity: int -> unit
  = "set_verbosity"

external gen_problem: preamble -> problem
  = "gen_problem"

external add_package_to_problem: problem -> cudf_package -> unit
  = "add_package_to_problem"

external set_problem_request: problem -> request -> unit
  = "set_problem_request"

external call_solver:
  solver_backend -> string -> int -> problem -> Cudf.package list option
  = "call_solver"

external backends_list:
  unit -> solver_backend list
  = "backends_list"

let problem_of_cudf cudf =
  let preamble, universe, request = cudf in
  let pb = gen_problem preamble in
  Cudf.iter_packages (add_package_to_problem pb) universe;
  set_problem_request pb request;
  pb

let resolve_cudf
    ?(verbose=false) ?timeout ?(solver=default_solver)
    criteria (preamble, _, _ as cudf) =
  let timeout = match timeout with
    | None -> 0
    | Some f -> int_of_float (1000. *. f)
  in
  set_verbosity (if verbose then 1 else 0);
  let pb = problem_of_cudf cudf in
  match call_solver solver criteria timeout pb with
  | None -> None
  | Some sol ->
    let univ = Cudf.load_universe sol in
    Some (preamble, univ)

let get_solver_id ?(solver=default_solver) () =
  "mccs+" ^
  match solver with
  | `GLPK -> "glpk"
  | `LP cmd -> Printf.sprintf "lp(%s)" cmd
  | `COIN_CLP -> "coin/clp"
  | `COIN_CBC -> "coin/cbc"
  | `COIN_SYMPHONY -> "coin/symphony"

let solver_id = get_solver_id ()

let supported_backends = backends_list ()
