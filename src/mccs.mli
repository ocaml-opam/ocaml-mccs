
type problem

val problem_of_cudf: Cudf.cudf -> problem

val resolve_cudf: ?verbose:bool -> string -> Cudf.cudf -> Cudf.solution option
