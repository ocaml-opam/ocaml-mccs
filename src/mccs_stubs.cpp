
#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <unordered_map>
#include <cudf.h>
// #include <cudf_hash_table.h>
#include <abstract_solver.h>
#include <cudf_reductions.h>
#include <mccscudf.h>

class Virtual_packages
{
  int rank;
  std::unordered_map<std::string, CUDFVirtualPackage*> * tbl;

public:

  CUDFVirtualPackage * get(const char *pkgname) {
    CUDFVirtualPackage *pkg;
    auto iterator = tbl->find(pkgname);
    if (iterator == tbl->end()) {
      // fprintf(stderr, "pkg %s not found, adding to tbl\n", pkgname);
      pkg = new CUDFVirtualPackage(pkgname, rank++);
      (*tbl)[pkgname] = pkg;
    } else {
      // fprintf(stderr, "pkg %s found, getting from tbl\n", pkgname);
      pkg = iterator->second;
    }
    return pkg;
  }

  CUDFVirtualPackageList * all() {
    CUDFVirtualPackageList * l = new CUDFVirtualPackageList;
    for (auto it = tbl->begin(); it != tbl->end(); it++)
      l->push_back(it->second);
    return l;
  }

  Virtual_packages() {
    rank = 0;
    tbl = new std::unordered_map<std::string, CUDFVirtualPackage*>;
  }

  ~Virtual_packages() {
    fprintf (stderr, "DELETING Virtual_packages !!\n");
    for (auto it = tbl->begin(); it != tbl->end(); it++)
      delete (it->second);
    delete tbl;
  }
};

CUDFPackageOp ml2c_relop(value relop)
{
  if (relop == caml_hash_variant("Eq")) return op_eq;
  else if (relop == caml_hash_variant("Geq")) return op_supeq;
  else if (relop == caml_hash_variant("Gt")) return op_sup;
  else if (relop == caml_hash_variant("Leq")) return op_infeq;
  else if (relop == caml_hash_variant("Lt")) return op_inf;
  else if (relop == caml_hash_variant("Neq")) return op_neq;
  else {
    fprintf(stderr, "ERROR: Invalid relop\n");
    return op_none;
  }
}

CUDFKeepOp ml2c_keepop(value op)
{
  if (op == caml_hash_variant("Keep_feature")) return keep_feature;
  else if (op == caml_hash_variant("Keep_none")) return keep_none;
  else if (op == caml_hash_variant("Keep_package")) return keep_package;
  else if (op == caml_hash_variant("Keep_version")) return keep_version;
  else {
    fprintf(stderr, "ERROR: Invalid keep_op\n");
    return keep_none;
  }
}

CUDFPropertyType ml2c_propertytype(value pt)
{
  if (pt == caml_hash_variant("Bool")) return pt_bool;
  else if (pt == caml_hash_variant("Int")) return pt_int;
  else if (pt == caml_hash_variant("Nat")) return pt_nat;
  else if (pt == caml_hash_variant("Posint")) return pt_posint;
  else if (pt == caml_hash_variant("Enum")) return pt_enum;
  else if (pt == caml_hash_variant("Pkgname") ||
           pt == caml_hash_variant("String") ||
           pt == caml_hash_variant("Ident")) return pt_string;
  else if (pt == caml_hash_variant("Vpkg")) return pt_vpkg;
  else if (pt == caml_hash_variant("Veqpkg")) return pt_veqpkg;
  else if (pt == caml_hash_variant("Vpkglist")) return pt_vpkglist;
  else if (pt == caml_hash_variant("Veqpkglist")) return pt_veqpkglist;
  else if (pt == caml_hash_variant("Vpkgformula")) return pt_vpkgformula;
  else if (pt == caml_hash_variant("Typedecl")) {
    fprintf(stderr, "ERROR: recursive property type declarations unsupported\n");
    return pt_none;
  }
  else {
    fprintf(stderr, "ERROR: invalid property\n");
    return pt_none;
  }
}

#define Val_none Val_int(0)
#define Some_val(v)  Field(v,0)

CUDFVpkg * ml2c_vpkg(Virtual_packages &tbl, value ml_vpkg)
{
  char * name = String_val(Field(ml_vpkg, 0));
  CUDFVirtualPackage * virt = tbl.get(name);
  value constr_opt = Field(ml_vpkg, 1);
  if (constr_opt == Val_none) return new CUDFVpkg(virt, op_none, 0);
  else {
    value constr = Some_val(constr_opt);
    return new CUDFVpkg(virt, ml2c_relop(Field(constr,0)), Int_val(Field(constr,1)));
  }
}

CUDFVpkgList * ml2c_vpkglist(Virtual_packages &tbl, value ml_vpkglist)
{
  CUDFVpkgList * lst = new CUDFVpkgList;
  for (value l = ml_vpkglist; l != Val_emptylist; l = Field(l, 1))
    lst->push_back(ml2c_vpkg(tbl, Field(l, 0)));
  return lst;
}

CUDFVpkgFormula * ml2c_vpkgformula(Virtual_packages &tbl, value ml_vpkgformula)
{
  CUDFVpkgFormula * form = NULL;
  if (ml_vpkgformula == Val_emptylist) return NULL;
  form = new CUDFVpkgFormula;
  for (value l = ml_vpkgformula; l != Val_emptylist; l = Field(l, 1))
    form->push_back(ml2c_vpkglist(tbl, Field(l, 0)));
  return form;
}

CUDFPropertyValue * ml2c_property(Virtual_packages &tbl, CUDFProperties * properties, value ml_prop)
{
  char * base_prop_name = String_val(Field(ml_prop,0));
  char property_name[strlen(base_prop_name)+2] = "";
  CUDFPropertiesIterator prop_it;
  CUDFProperty * prop;
  value ml_v = Field(ml_prop,1);
  value v = Field(ml_v, 1);
  strcpy (property_name, base_prop_name);
  strcat (property_name, ":");

  prop_it = properties->find(property_name);
  if (prop_it == properties->end()) {
    fprintf(stderr, "ERROR: property %s not found\n", property_name);
    abort();
  }
  prop = prop_it->second;

  switch(ml2c_propertytype(Field(ml_v, 0))) {
  case pt_bool:
    return new CUDFPropertyValue(prop, Bool_val(v));
  case pt_int: case pt_posint: case pt_nat:
    return new CUDFPropertyValue(prop, Int_val(v));
  case pt_string:
    return new CUDFPropertyValue(prop, String_val(v));
  case pt_enum:
    for (CUDFEnumsIterator ei = prop->enuml->begin(); ei != prop->enuml->end(); ei++)
      if (strcmp((*ei), String_val(v)) == 0)
        return new CUDFPropertyValue(prop, *ei);
    fprintf(stderr, "ERROR: invalid enum case\n");
    exit(88);
  case pt_vpkg: case pt_veqpkg:
    return new CUDFPropertyValue(prop, ml2c_vpkg(tbl, v));
  case pt_vpkgformula:
    return new CUDFPropertyValue(prop, ml2c_vpkgformula(tbl, v));
  case pt_vpkglist: case pt_veqpkglist:
    return new CUDFPropertyValue(prop, ml2c_vpkglist(tbl, v));
  case pt_none:
    fprintf(stderr, "ERROR: none property\n");
    exit(88);
  default:
    fprintf(stderr, "ERROR: unrecognised property\n");
    exit(88);
  }
}

CUDFPropertyValueList * ml2c_propertylist(Virtual_packages &tbl, CUDFProperties * properties, value ml_plist)
{
  CUDFPropertyValueList * plist = new CUDFPropertyValueList;
  for (value l = ml_plist; l != Val_emptylist; l = Field(l, 1))
    plist->push_back(ml2c_property(tbl, properties, Field(l, 0)));
  return plist;
}

extern int versioned_package_rank;

CUDFVersionedPackage * ml2c_package(Virtual_packages &tbl, CUDFProperties * properties, int &max_rank, value ml_package)
{
  char * package = String_val(Field(ml_package, 0));
  unsigned int version = Int_val(Field(ml_package, 1));
  bool installed = Bool_val(Field(ml_package, 5));

  CUDFVirtualPackage * virtual_package = tbl.get(package);
  CUDFVersionedPackage * pkg = new CUDFVersionedPackage(package, max_rank++);
  virtual_package->all_versions.insert(pkg);
  if (version > virtual_package->highest_version)
    virtual_package->highest_version = version;
  if (installed && virtual_package->highest_installed == (CUDFVersionedPackage *)NULL)
    virtual_package->highest_installed = pkg;

  pkg->virtual_package = virtual_package;
  pkg->version = version;
  pkg->depends = ml2c_vpkgformula(tbl, Field(ml_package,2));
  pkg->conflicts =  ml2c_vpkglist(tbl, Field(ml_package,3));
  pkg->provides = ml2c_vpkglist(tbl, Field(ml_package,4));
  pkg->installed = installed;
  pkg->wasinstalled = Bool_val(Field(ml_package, 6));
  pkg->keep = ml2c_keepop(Field(ml_package, 7));
  pkg->properties = *ml2c_propertylist(tbl, properties, Field(ml_package, 8));

  return pkg;
}

CUDFProperty * ml2c_propertydef(Virtual_packages &tbl, value ml_pdef)
{
  char * base_prop_name = String_val(Field(ml_pdef,0));
  char property_name[strlen(base_prop_name)+2] = "";
  value def = Field(ml_pdef,1);
  CUDFPropertyType ty = ml2c_propertytype(Field(def,0));
  value arg = Field(def,1);
  CUDFProperty * p = NULL;
  strcpy(property_name, base_prop_name);
  strcat(property_name, ":");
  if (arg == Val_none)
    p = new CUDFProperty(property_name, ty);
  else {
    switch(ty) {
    case pt_bool:
      p = new CUDFProperty(property_name, ty, Bool_val(Some_val(arg))); break;
    case pt_int: case pt_posint: case pt_nat:
      p = new CUDFProperty(property_name, ty, Int_val(Some_val(arg))); break;
    case pt_string:
      p = new CUDFProperty(property_name, ty, String_val(Some_val(arg))); break;
    case pt_enum: {
      CUDFEnums * enuml = new CUDFEnums;
      char * dft;
      for (value l = Field(arg, 0); l != Val_emptylist; l = Field(l, 1))
        enuml->push_back(String_val(Field(l,0)));
      if (Field(arg, 1) == Val_none)
        p = new CUDFProperty(property_name, ty, enuml);
      else {
        dft = String_val(Some_val(Field(arg, 1)));
        for (CUDFEnumsIterator ei = enuml->begin(); ei != enuml->end(); ei++)
          if (strcmp((*ei), dft) == 0)
            p = new CUDFProperty(property_name, ty, enuml, *ei);
        if (p == NULL)
          fprintf(stderr, "ERROR: invalid enum case\n");
      }
      break;
    }
    case pt_vpkg: case pt_veqpkg:
      p = new CUDFProperty(property_name, ty, ml2c_vpkg(tbl, Some_val(arg))); break;
    case pt_vpkgformula:
      p = new CUDFProperty(property_name, ty, ml2c_vpkgformula(tbl, Some_val(arg))); break;
    case pt_vpkglist: case pt_veqpkglist:
      p = new CUDFProperty(property_name, ty, ml2c_vpkglist(tbl, Some_val(arg))); break;
    case pt_none:
      fprintf(stderr, "ERROR: none property def\n"); break;
    default:
      fprintf(stderr, "ERROR: unrecognised property type def\n"); break;
    }
  }
  if (p == NULL) exit(88);
  return p;
}

CUDFProperties * ml2c_propertydeflist(Virtual_packages &tbl, value ml_pdeflist)
{
  CUDFProperties * pdeflist = new CUDFProperties;
  for (value l = ml_pdeflist; l != Val_emptylist; l = Field(l, 1)) {
    CUDFProperty * prop = ml2c_propertydef(tbl, Field(l, 0));
    (*pdeflist)[prop->name] = prop;
  }
  return pdeflist;
}

// get an enum from its name in an enum list
char *get_enum(CUDFEnums *e, char *estr) {
  for (CUDFEnumsIterator ei = e->begin(); ei != e->end(); ei++)
    if (strcmp((*ei), estr) == 0) return (*ei);
  return (char *)NULL;
}

typedef struct {
  CUDFproblem * pb_cudf_problem;
  Virtual_packages pb_virtual_packages;
  int pb_max_versioned_package_rank;
} problem;

#define Problem_pt(v) ((problem *) (Data_custom_val(v)))

void finalize_problem(value ml_pb) {
  problem * pb = Problem_pt(ml_pb);
  delete pb->pb_cudf_problem;
  //pb->pb_virtual_packages_tbl.clear;
  //pb->pb_all_virtual_packages.clear;
  return;
}

static struct custom_operations problem_ops = {
  (char *)"mccs_cudf_problem",
  &finalize_problem,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default
};

extern "C" value set_verbosity(value v)
{
  CAMLparam1 (v);
  verbosity = Int_val(v);
  CAMLreturn (Val_unit);
}

extern "C" value gen_problem(value preamble)
{
  CAMLparam1 (preamble);
  CAMLlocal1 (ml_problem);
  CUDFproblem * cpb;
  Virtual_packages * tbl = new Virtual_packages;
  problem * pb;

  ml_problem = caml_alloc_custom(&problem_ops, sizeof(problem), 0, 1);
  pb = Problem_pt(ml_problem);

  // initialise cudf problem
  cpb = new CUDFproblem;
  cpb->properties = ml2c_propertydeflist(*tbl, Field(preamble,1));
  cpb->all_packages = new CUDFVersionedPackageList;
  cpb->installed_packages = new CUDFVersionedPackageList;
  cpb->uninstalled_packages = new CUDFVersionedPackageList;

  pb->pb_cudf_problem = cpb;
  pb->pb_virtual_packages = *tbl;
  pb->pb_max_versioned_package_rank = 0;

  CAMLreturn (ml_problem);
}

extern "C" value add_package_to_problem(value ml_problem, value ml_package)
{
  CAMLparam2 (ml_problem, ml_package);
  problem * pb = Problem_pt(ml_problem);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  CUDFVersionedPackage * pkg;
  Virtual_packages &tbl = pb->pb_virtual_packages;

  pkg = ml2c_package(tbl, cpb->properties, pb->pb_max_versioned_package_rank, ml_package);

  //fprintf(stderr, "add package: %s\n", pkg->name);

  cpb->all_packages->push_back(pkg);
  if (pkg->installed) {
    cpb->installed_packages->push_back(pkg);
  } else
    cpb->uninstalled_packages->push_back(pkg);

  CAMLreturn (Val_unit);
}

extern "C" value set_problem_request(value ml_problem, value ml_request)
{
  CAMLparam2 (ml_problem, ml_request);
  problem * pb = Problem_pt(ml_problem);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  Virtual_packages &tbl = pb->pb_virtual_packages;

  cpb->install = ml2c_vpkglist(tbl, Field(ml_request, 1));
  cpb->remove = ml2c_vpkglist(tbl, Field(ml_request, 2));
  cpb->upgrade = ml2c_vpkglist(tbl, Field(ml_request, 3));
  cpb->all_virtual_packages = tbl.all();

  pb->pb_cudf_problem = compute_reduced_CUDF(cpb);
  if (pb->pb_cudf_problem != cpb) delete cpb;

  if (Val_emptylist != Field(ml_request, 3)) {
    fprintf(stderr, "WARNING: extra request field not supported\n");
  }

  CAMLreturn (Val_unit);
}

extern "C" value call_solver(value ml_criteria, value ml_problem, value ml_outfile/*tmp wip*/)
{
  CAMLparam3(ml_criteria, ml_problem, ml_outfile);
  char * outfile = String_val(ml_outfile);
  problem * pb = Problem_pt(ml_problem);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  CUDFVirtualPackageList all_virtual_packages = *(cpb->all_virtual_packages);
  CUDFVersionedPackageList all_packages = *(cpb->all_packages);
  Solver_return ret;
  FILE * out;
  char criteria[strlen(String_val(ml_criteria))+3] = "[";

  strcat(criteria, String_val(ml_criteria));
  strcat(criteria, "]");

  ret = call_mccs(GLPK, criteria, cpb);
  if (ret.success == 0) caml_failwith(ret.error);
  out = fopen(outfile, "w");

  if (ret.solution == NULL) {
    fprintf(out, "FAIL\n");
    fprintf(out, "No solution found.\n");
  }
  else {
    if (cpb->properties->size() > 0) {
      fprintf(out, "preamble: \n");
      print_properties(out, cpb->properties);
      fprintf(out, "\n\n");
    }
    for (CUDFVersionedPackageListIterator ipkg = cpb->all_packages->begin(); ipkg != cpb->all_packages->end(); ipkg++) {
      if (ret.solution->get_solution(*ipkg))
        print_versioned_package_as_installed(out, (*ipkg), true);
    }
  }
  fclose(out);

  CAMLreturn (Val_unit);
}
