type cudf_package = Cudf.package =  {
  package : string;
  version : int;
  depends : Cudf_types.vpkgformula;
  conflicts : Cudf_types.vpkglist;
  provides : Cudf_types.veqpkglist;
  installed : bool;
  was_installed : bool;
  keep : Cudf_types.enum_keep;
  pkg_extra : Cudf_types.typed_value Cudf_types.stanza;
}

type problem

external gen_problem: unit -> problem = "gen_problem"

external add_package: problem -> cudf_package -> unit = "add_package"



external call_solver : string array -> unit = "call_solver"
