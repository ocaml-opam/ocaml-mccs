(**************************************************************************)
(*                                                                        *)
(*    Copyright 2017-2018 OCamlPro                                        *)
(*                                                                        *)
(*  All rights reserved. This file is distributed under the terms of the  *)
(*  GNU Lesser General Public License version 2.1, with the special       *)
(*  exception on linking described in the file LICENSE.                   *)
(*                                                                        *)
(**************************************************************************)

type problem

exception Timeout

val problem_of_cudf: Cudf.cudf -> problem

type solver_backend = [ `GLPK | `LP of string | `COIN_CLP | `COIN_CBC | `COIN_SYMPHONY ]

(** Resolve the given problem. The timeout is in seconds, default is to never
    time out. *)
val resolve_cudf:
  ?verbose:bool -> ?timeout:float -> ?solver:solver_backend ->
  string -> Cudf.cudf -> Cudf.solution option

(** Deprecated, corresponds to the default solver backend selection only *)
val solver_id: string

val get_solver_id: ?solver:solver_backend -> unit -> string

val supported_backends: solver_backend list
