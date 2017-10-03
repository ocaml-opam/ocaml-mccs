
type problem

exception Timeout

val problem_of_cudf: Cudf.cudf -> problem

(** Resolve the given problem. The timeout is in seconds, default is to never time out. *)
val resolve_cudf: ?verbose:bool -> ?timeout:float -> string -> Cudf.cudf -> Cudf.solution option

val solver_id: string
