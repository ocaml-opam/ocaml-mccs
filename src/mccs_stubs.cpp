#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <mccscudf.h>

#define Problem_val(v) (* (CUDFproblem **) (Data_custom_val(v)))

void finalize_problem(value pb) {
  delete Problem_val(pb);
}

static struct custom_operations problem_ops = {
  "mccs_cudf_problem",
  &finalize_problem,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default
};

value gen_problem()
{
  CAMLparam0 ();
  value v = caml_alloc_custom(&problem_ops, sizeof(CUDFproblem *), 0, 1);
  Problem_val(v) = new CUDFproblem ();
  CAMLreturn v;
}

CUDFPackageOp ml2c_relop(value relop)
{
  if (relop == hash_variant("Eq")) return op_eq;
  else if (relop == hash_variant("Geq")) return op_supeq;
  else if (relop == hash_variant("Gt")) return op_sup;
  else if (relop == hash_variant("Leq")) return op_infeq;
  else if (relop == hash_variant("Lt")) return op_inf;
  else if (relop == hash_variant("Neq")) return op_neq;
  else {
    fprintf(stderr, "ERROR: Invalid relop");
    return op_none;
  }
}

CUDFKeepOp ml2c_keepop(value op)
{
  if (relop == hash_variant("Keep_feature")) return keep_feature;
  else if (relop == hash_variant("Keep_none")) return keep_none;
  else if (relop == hash_variant("Keep_package")) return keep_package;
  else if (relop == hash_variant("Keep_version")) return keep_version;
  else {
    fprintf(stderr, "ERROR: Invalid keep_op");
    return keep_none;
  }
}

vpkg ml2c_vpkg(value ml_vpkg)
{
  char * name = Field(ml_vpkg, 0);
  CUDFVirtualPackage * virt = get_virtual_package(name);
  value constr_opt = Field(ml_vpkg, 1);
  if (constr_opt == Val_none) return new CUDFVpkg(virt, op_none, 0);
  else {
    value constr = Some_val(constr_opt);
    return new CUDFVpkg(virt, o2c_relop(Field(constr,0)), Int_val(Field(constr,1)));
  }
}

CUDFVpkgList ml2c_vpkglist(value ml_vpkglist)
{
  CUDFVpkgList * lst = new CUDFVpkgList;
  while (ml_vpkglist != Val_emptylist) {
    lst->push_back(ml2c_vpkg(Field(ml_vpkglist, 0)));
    ml_vpkglist = Field(ml_vpkglist, 1);
  }
  return lst;
}

CUDFVpkgFormula ml2c_vpkgformula(value ml_vpkgformula)
{
  CUDFVpkgFormula * form = new CUDFVpkgFormula;
  while (ml_vpkgformula != Val_emptylist) {
    lst->push_back(ml2c_vpkglist(Field(ml_vpkgformula, 0)));
    ml_vpkgformula = Field(ml_vpkgformula, 1);
  }
  return form;
}



CUDFPropertyValue ml2c_property(value ml_prop)
{
  char * property_name = String_val(Field(ml_prop,0));
  CudfProperty prop = properties.find(property_name);
  value v = Field(ml_prop,1);
  int tag = Tag(v);
  assert(prop != properties.end());
  if (tag == hash_variant("Int"))
    return new CUDFPropertyValue(prop, Int_val(Field(
  CUDFPropertyValue 
}


// FIXME: three globals: defined properties, this one and the package hashtable: pass them along
int versioned_package_rank = 0;

CUDFVersionedPackage ml2c_package(value ml_package)
{
  string package = String_val(Field(ml_package, 0));
  int version = Int_val(Field(ml_package, 1));
  bool installed = Bool_val(Field(ml_package, 5));

  CUDFVirtualPackage * virtual_package = get_virtual_package(package);
  CUDFVersionedPackage * pkg = new CUDFVersionedPackage(package, versioned_package_rank++);
  virtual_package->all_versions.insert(pkg);
  if (version > virtual_package->highest_version)
    virtual_package->highest_version = version;
  if (installed && virt_pkg->highest_installed == (CUDFVersionedPackage *)NULL)
    virtual_package->highest_installed = pkg;

  pkg->virtual_package = virtual_package;
  pkg->version = version;
  pkg->depends =  ml2c_vpkgformula(Field(ml_package,2));
  pkg->conflicts =  ml2c_vpkglist(Field(ml_package,3));
  pkg->provides = ml2c_vpkglist(Field(ml_package,4));
  pkg->installed = installed;
  pkg->was_installed = Bool_val(Field(ml_package, 6));
  pkg->keep = ml2c_keepop(Field(ml_package, 7));
  // pkg->properties = ml2c_propertylist Field(ml_package, 8);

  return pkg
}

value add_package(value problem, value ml_package)
{
  CAMLparam2 (problem, cudf_package);

  CUDFVersionedPackage pkg = ml2c_package(ml_package)

  problem->all_packages.push_back(vpkg);
  if (pkg->installed) {
    problem->installed_packages.push_back(pkg);
  } else
    problem->uninstalled_packages.push_back(pkg);

  current_package = new CUDFVersionedPackage($2, versioned_package_rank++);
  the_problem->properties = &properties;
  the_problem->all_packages = &all_packages;
  the_problem->installed_packages = &installed_packages;
  the_problem->uninstalled_packages = &uninstalled_packages;
  the_problem->all_virtual_packages = &all_virtual_packages;
}

extern "C" value call_solver(value verbose, value criteria, value mccs_cudf)
{
  CAMLparam3(verbose, criteria, mccs_cudf);
  
  int argc = Wosize_val(args);
  char * argv[argc];
  for (int i=0; i<argc; i++) argv[i] = String_val(Field(args, i));
  call_mccs(argc, argv);
  CAMLreturn (Val_unit);
}
