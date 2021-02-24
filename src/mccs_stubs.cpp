#ifdef _WIN32
#include <Windows.h>
#endif

#define CAML_NAME_SPACE
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/callback.h>
#include <signal.h>
#include <map>
#include <cudf.h>
#include <abstract_solver.h>
#include <cudf_reductions.h>
#include <mccscudf.h>
#ifdef USEGLPK
#include <glpk_solver.h>
#endif
#ifdef USECOIN
#include <osi_solver.h>
#endif

#if OCAML_VERSION < 41200
#define Val_none Val_int(0)
#define Some_val(v) Field(v, 0)
#endif

value Val_some (value v)
{
  CAMLparam1 (v);
  CAMLlocal1 (some);
  some = caml_alloc_tuple(1);
  Store_field (some, 0, v);
  CAMLreturn (some);
}

value Val_pair (value v1, value v2)
{
  CAMLparam2 (v1, v2);
  CAMLlocal1 (pair);
  pair = caml_alloc_tuple(2);
  Store_field (pair, 0, v1);
  Store_field (pair, 1, v2);
  CAMLreturn (pair);
}

class Virtual_packages
{
  int rank;
  map<string, CUDFVirtualPackage*> * tbl;

public:

  CUDFVirtualPackage * get(const char *pkgname) {
    CUDFVirtualPackage *pkg;
    map<string, CUDFVirtualPackage*>::iterator iterator = tbl->find(pkgname);
    if (iterator == tbl->end()) {
      pkg = new CUDFVirtualPackage(pkgname, rank++);
      (*tbl)[pkgname] = pkg;
    } else {
      pkg = iterator->second;
    }
    return pkg;
  }

  CUDFVirtualPackageList * all() {
    CUDFVirtualPackageList * l = new CUDFVirtualPackageList;
    for (map<string, CUDFVirtualPackage*>::iterator it = tbl->begin(); it != tbl->end(); it++)
      l->push_back(it->second);
    return l;
  }

  Virtual_packages() {
    rank = 0;
    tbl = new map<string, CUDFVirtualPackage*>;
  }

  ~Virtual_packages() {
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
  else caml_failwith("invalid relop");
}

value c2ml_relop (CUDFPackageOp op)
{
  switch (op) {
  case op_eq: return caml_hash_variant("Eq");
  case op_supeq: return caml_hash_variant("Geq");
  case op_sup: return caml_hash_variant("Gt");
  case op_infeq: return caml_hash_variant("Leq");
  case op_inf: return caml_hash_variant("Lt");
  case op_neq: return caml_hash_variant("Neq");
  case op_none: default: caml_failwith("invalid relop");
  }
}

CUDFKeepOp ml2c_keepop(value ml_op)
{
  if (ml_op == caml_hash_variant("Keep_feature")) return keep_feature;
  else if (ml_op == caml_hash_variant("Keep_none")) return keep_none;
  else if (ml_op == caml_hash_variant("Keep_package")) return keep_package;
  else if (ml_op == caml_hash_variant("Keep_version")) return keep_version;
  else caml_failwith("Invalid keep_op");
}

value c2ml_keepop(CUDFKeepOp op)
{
  switch (op) {
  case keep_feature: return caml_hash_variant("Keep_feature");
  case keep_none: return caml_hash_variant("Keep_none");
  case keep_package: return caml_hash_variant("Keep_package");
  case keep_version: return caml_hash_variant("Keep_version");
  default: caml_failwith("Invalid 'keep' operator");
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
  else if (pt == caml_hash_variant("Typedecl"))
    caml_failwith("recursive property type declarations unsupported");
  else
    caml_failwith("invalid property");
}

CUDFVpkg * ml2c_vpkg(Virtual_packages * tbl, value ml_vpkg)
{
  const char * name = String_val(Field(ml_vpkg, 0));
  CUDFVirtualPackage * virt = tbl->get(name);
  value constr_opt = Field(ml_vpkg, 1);
  if (constr_opt == Val_none) return new CUDFVpkg(virt, op_none, 0);
  else {
    value constr = Some_val(constr_opt);
    return new CUDFVpkg(virt, ml2c_relop(Field(constr,0)), Int_val(Field(constr,1)));
  }
}

value c2ml_vpkg(CUDFVpkg * vpkg)
{
  CAMLparam0 ();
  CAMLlocal4(ml_name, ml_cstr, ml_p, ml_s);
  ml_name = caml_copy_string(vpkg->virtual_package->name);
  if (vpkg->op == op_none)
    ml_p = Val_pair(ml_name, Val_none);
  else {
    ml_cstr = Val_pair(c2ml_relop(vpkg->op), Val_int(vpkg->version));
    ml_s = Val_some(ml_cstr);
    ml_p = Val_pair(ml_name, ml_s);
  }
  CAMLreturn(ml_p);
}

CUDFVpkgList * ml2c_vpkglist(Virtual_packages * tbl, value ml_vpkglist)
{
  CUDFVpkgList * lst = new CUDFVpkgList;
  for (value l = ml_vpkglist; l != Val_emptylist; l = Field(l, 1))
    lst->push_back(ml2c_vpkg(tbl, Field(l, 0)));
  return lst;
}

value c2ml_vpkglist(CUDFVpkgList * vpkgl)
{
  CAMLparam0 ();
  CAMLlocal2 (item, r);
  r = Val_emptylist;
  for (vector<CUDFVpkg*>::iterator it = vpkgl->begin(); it != vpkgl->end(); it++) {
    item = c2ml_vpkg(*it);
    r = Val_pair (item, r);
  }
  CAMLreturn (r);
}

CUDFVpkgFormula * ml2c_vpkgformula(Virtual_packages * tbl, value ml_vpkgformula)
{
  CUDFVpkgFormula * form = NULL;
  if (ml_vpkgformula == Val_emptylist) return NULL;
  form = new CUDFVpkgFormula;
  for (value l = ml_vpkgformula; l != Val_emptylist; l = Field(l, 1))
    form->push_back(ml2c_vpkglist(tbl, Field(l, 0)));
  return form;
}

value c2ml_vpkgformula(CUDFVpkgFormula * form)
{
  CAMLparam0 ();
  CAMLlocal2 (item, r);
  r = Val_emptylist;
  if (form != NULL) {
    for (vector<vector<CUDFVpkg*>*>::iterator it = form->begin(); it != form->end(); it++) {
      item = c2ml_vpkglist(*it);
      r = Val_pair (item, r);
    }
  }
  CAMLreturn (r);
}

CUDFPropertyValue * ml2c_property(Virtual_packages * tbl, CUDFProperties * properties, value ml_prop)
{
  const char * prop_name = String_val(Field(ml_prop,0));
  CUDFPropertiesIterator prop_it;
  CUDFProperty * prop;
  value ml_v = Field(ml_prop,1);
  value v = Field(ml_v, 1);

  prop_it = properties->find(prop_name);
  if (prop_it == properties->end())
    caml_failwith("property not found");
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
    caml_failwith("invalid enum case");
  case pt_vpkg: case pt_veqpkg:
    return new CUDFPropertyValue(prop, ml2c_vpkg(tbl, v));
  case pt_vpkgformula:
    return new CUDFPropertyValue(prop, ml2c_vpkgformula(tbl, v));
  case pt_vpkglist: case pt_veqpkglist:
    return new CUDFPropertyValue(prop, ml2c_vpkglist(tbl, v));
  case pt_none:
    caml_failwith("none property");
  default:
    caml_failwith("unrecognised property");
  }
}

value c2ml_property (CUDFPropertyValue * prop)
{
  CAMLparam0 ();
  CAMLlocal2 (ml_prop_name, pval);
  ml_prop_name = caml_copy_string(prop->property->name);

  switch (prop->property->type_id) {
  case pt_bool:   pval = Val_pair(caml_hash_variant("Bool"),   Val_bool(prop->intval)); break;
  case pt_int:    pval = Val_pair(caml_hash_variant("Int"),    Val_int(prop->intval)); break;
  case pt_posint: pval = Val_pair(caml_hash_variant("Posint"), Val_int(prop->intval)); break;
  case pt_nat:    pval = Val_pair(caml_hash_variant("Nat"),    Val_int(prop->intval)); break;
  case pt_string: pval = Val_pair(caml_hash_variant("String"), caml_copy_string(prop->strval)); break;
  case pt_enum: // todo
  case pt_vpkg: case pt_veqpkg: // todo
  case pt_vpkgformula: // todo
  case pt_vpkglist: case pt_veqpkglist: // todo
    caml_failwith("unimplemented cudf property type");
  case pt_none:
    caml_failwith("none property type");
  default:
    caml_failwith("unrecognised property type");
  }

  CAMLreturn (Val_pair (ml_prop_name, pval));
}

void ml2c_propertylist
  (CUDFPropertyValueList &plist, Virtual_packages * tbl, CUDFProperties * properties, value ml_plist)
{
  for (value l = ml_plist; l != Val_emptylist; l = Field(l, 1))
    plist.push_back(ml2c_property(tbl, properties, Field(l, 0)));
}

value c2ml_propertylist (CUDFPropertyValueList * plist)
{
  CAMLparam0 ();
  CAMLlocal2 (item, r);
  r = Val_emptylist;
  for (vector<CUDFPropertyValue*>::iterator it = plist->begin(); it != plist->end(); it++) {
    item = c2ml_property(*it);
    r = Val_pair(item, r);
  }
  CAMLreturn (r);
}

//extern int versioned_package_rank;

CUDFVersionedPackage * ml2c_package(Virtual_packages * tbl, CUDFProperties * properties, int &max_rank, value ml_package)
{
  const char * package = String_val(Field(ml_package, 0));
  unsigned int version = Int_val(Field(ml_package, 1));
  bool installed = Bool_val(Field(ml_package, 5));

  CUDFVirtualPackage * virtual_package = tbl->get(package);
  CUDFVersionedPackage * pkg = new CUDFVersionedPackage(package, max_rank++);
  CUDFVpkgList * provides = ml2c_vpkglist(tbl, Field(ml_package,4));
  pkg->set_version(version);

  virtual_package->all_versions.insert(pkg);
  if (version > virtual_package->highest_version)
    virtual_package->highest_version = version;
  if (installed &&
      (virtual_package->highest_installed == (CUDFVersionedPackage *)NULL ||
       version > virtual_package->highest_installed->version))
    virtual_package->highest_installed = pkg;

  for (CUDFVpkgListIterator ei = provides->begin(); ei != provides->end(); ei++) {
    CUDFVirtualPackage * vprovided = (*ei)->virtual_package;
    switch ((*ei)->op) {
    case op_none: vprovided->providers.push_back(pkg); break;
    case op_eq: {
      CUDFVersionedProviderListIterator ivpkgl;
      if (installed && version > vprovided->highest_installed_provider_version)
        vprovided->highest_installed_provider_version = version;
      ivpkgl = vprovided->versioned_providers.find(version);
      if (ivpkgl == vprovided->versioned_providers.end())
	vprovided->versioned_providers.insert(CUDFVersionedProviderList::value_type(version, CUDFProviderList(1, pkg)));
      else
	ivpkgl->second.push_back(pkg);
      break;
    }
    default: caml_failwith("invalid provides formula");
    }
  }

  pkg->virtual_package = virtual_package;
  pkg->depends = ml2c_vpkgformula(tbl, Field(ml_package,2));
  pkg->conflicts = ml2c_vpkglist(tbl, Field(ml_package,3));
  pkg->provides = provides;
  pkg->installed = installed;
  pkg->wasinstalled = Bool_val(Field(ml_package, 6));
  pkg->keep = ml2c_keepop(Field(ml_package, 7));
  ml2c_propertylist(pkg->properties, tbl, properties, Field(ml_package, 8));

  return pkg;
}

value c2ml_package(CUDFVersionedPackage * pkg)
{
  CAMLparam0 ();
  CAMLlocal1 (ml_package);

  ml_package = caml_alloc_tuple(9);
  Store_field(ml_package, 0, caml_copy_string(pkg->name)); // pkgname
  Store_field(ml_package, 1, Val_int(pkg->version)); // version
  Store_field(ml_package, 2, c2ml_vpkgformula(pkg->depends)); // depends
  Store_field(ml_package, 3, c2ml_vpkglist(pkg->conflicts)); // conflicts
  Store_field(ml_package, 4, c2ml_vpkglist(pkg->provides)); // provides
  Store_field(ml_package, 5, Val_bool(pkg->installed)); // installed
  Store_field(ml_package, 6, Val_bool(pkg->wasinstalled)); // was_installed
  Store_field(ml_package, 7, c2ml_keepop(pkg->keep)); // keep
  Store_field(ml_package, 8, c2ml_propertylist(&(pkg->properties))); // pkg_extra

  CAMLreturn(ml_package);
}

CUDFProperty * ml2c_propertydef(Virtual_packages * tbl, value ml_pdef)
{
  const char * property_name = String_val(Field(ml_pdef,0));
  value def = Field(ml_pdef,1);
  CUDFPropertyType ty = ml2c_propertytype(Field(def,0));
  value arg = Field(def,1);
  CUDFProperty * p = NULL;
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
      const char * dft;
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
          caml_failwith("invalid enum case");
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
      caml_failwith("none property def");
    default:
      caml_failwith("unrecognised property type def");
    }
  }
  if (p == NULL) exit(88);
  return p;
}

CUDFProperties * ml2c_propertydeflist(Virtual_packages * tbl, value ml_pdeflist)
{
  CUDFProperties * pdeflist = new CUDFProperties;
  for (value l = ml_pdeflist; l != Val_emptylist; l = Field(l, 1)) {
    CUDFProperty * prop = ml2c_propertydef(tbl, Field(l, 0));
    (*pdeflist)[prop->name] = prop;
  }
  return pdeflist;
}

Solver ml2c_solver(value ml_solver)
{
  Solver ret = { GLPK, NULL };
  if (Is_block(ml_solver))
    if (Field (ml_solver, 0) == caml_hash_variant("LP")) {
      ret.backend = LP;
      ret.lp_solver = String_val (Field (ml_solver, 1));
    }
    else caml_failwith("invalid solver backend");
  else if (ml_solver == caml_hash_variant("GLPK"))
    ret.backend = GLPK;
  else if (ml_solver == caml_hash_variant("COIN_CLP"))
    ret.backend = CLP;
  else if (ml_solver == caml_hash_variant("COIN_CBC"))
    ret.backend = CBC;
  else if (ml_solver == caml_hash_variant("COIN_SYMPHONY"))
    ret.backend = SYMPHONY;
  else
    caml_failwith("invalid solver backend");
  return ret;
}

// get an enum from its name in an enum list
const char *get_enum(CUDFEnums *e, const char *estr) {
  for (CUDFEnumsIterator ei = e->begin(); ei != e->end(); ei++)
    if (strcmp((*ei), estr) == 0) return (*ei);
  return (const char *)NULL;
}

typedef struct {
  CUDFproblem * pb_cudf_problem;
  Virtual_packages * pb_virtual_packages;
  int pb_max_versioned_package_rank;
} problem;

#define Problem_pt(v) ((problem *) (Data_custom_val(v)))

#define Delete_contents(t,l) for (vector<t*>::iterator it = l->begin(); it != l->end(); it++) delete(*it)

void finalize_problem(value ml_pb) {
  problem * pb = Problem_pt(ml_pb);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  Delete_contents(CUDFVersionedPackage,cpb->all_packages);
  Delete_contents(CUDFVpkg,cpb->install);
  Delete_contents(CUDFVpkg,cpb->remove);
  Delete_contents(CUDFVpkg,cpb->upgrade);
  Delete_contents(CUDFVirtualPackage,cpb->all_virtual_packages);
  for (map<string, CUDFProperty*>::iterator it = cpb->properties->begin(); it != cpb->properties->end(); it++)
    delete(it->second);
  if (pb->pb_virtual_packages != NULL)
    delete (pb->pb_virtual_packages);
  delete cpb->install;
  delete cpb->remove;
  delete cpb->upgrade;
  delete cpb->properties;
  delete cpb;
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
  cpb->properties = ml2c_propertydeflist(tbl, Field(preamble,1));
  cpb->all_packages = new CUDFVersionedPackageList;
  cpb->installed_packages = new CUDFVersionedPackageList;
  cpb->uninstalled_packages = new CUDFVersionedPackageList;

  pb->pb_cudf_problem = cpb;
  pb->pb_virtual_packages = tbl;
  pb->pb_max_versioned_package_rank = 0;

  CAMLreturn (ml_problem);
}

extern "C" value add_package_to_problem(value ml_problem, value ml_package)
{
  CAMLparam2 (ml_problem, ml_package);
  problem * pb = Problem_pt(ml_problem);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  CUDFVersionedPackage * pkg;
  Virtual_packages * tbl = pb->pb_virtual_packages;

  pkg = ml2c_package(tbl, cpb->properties, pb->pb_max_versioned_package_rank, ml_package);

  //PRINT_ERR("add package: %s\n", pkg->name);

  cpb->all_packages->push_back(pkg);
  if (pkg->installed) {
    cpb->installed_packages->push_back(pkg);
  } else
    cpb->uninstalled_packages->push_back(pkg);

  CAMLreturn (Val_unit);
}

// Must be called once after all packages are added, and before solving
extern "C" value set_problem_request(value ml_problem, value ml_request)
{
  CAMLparam2 (ml_problem, ml_request);
  problem * pb = Problem_pt(ml_problem);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  Virtual_packages * tbl = pb->pb_virtual_packages;

  cpb->install = ml2c_vpkglist(tbl, Field(ml_request, 1));
  cpb->remove = ml2c_vpkglist(tbl, Field(ml_request, 2));
  cpb->upgrade = ml2c_vpkglist(tbl, Field(ml_request, 3));
  cpb->all_virtual_packages = tbl->all();

  delete tbl; // no longer needed
  pb->pb_virtual_packages = NULL;

  if (Val_emptylist != Field(ml_request, 4)) {
    PRINT_ERR("WARNING: extra request field not supported\n");
  }

  CAMLreturn (Val_unit);
}

abstract_solver *mccs_current_solver = NULL;

#ifndef _WIN32

static void sigint_handler(int sig, siginfo_t *si, void * ucontext) {
  if (mccs_current_solver) {
    mccs_current_solver->abort();
  }
}

static struct sigaction ocaml_sigint_action;

void install_sigint_handler() {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = sigint_handler;
  if (sigaction(SIGINT, &sa, &ocaml_sigint_action) == -1) {
    PRINT_ERR("ERROR: cannot install solver signal handler\n");
    exit(99);
  }
  return;
}

void restore_sigint_handler() {
  if (sigaction (SIGINT, &ocaml_sigint_action, NULL) == -1) {
    PRINT_ERR("ERROR: cannot restore solver signal handler\n");
    exit(99);
  }
  return;
}

#else

/*
 * Handling CTRL+C on Windows. Windows allows a handler function to be called on
 * certain events via the SetConsoleCtrlHandler function. This function handles
 * both the addition and removal of handler functions and maintains a stack of
 * handlers (so there is no need to back-up OCaml's handler).
 */

static BOOL WINAPI sigint_handler(DWORD dwCtrlType) {
  if (dwCtrlType == CTRL_C_EVENT) {
    if (mccs_current_solver) {
      mccs_current_solver->abort();
    }
    return TRUE;
  } else {
    return FALSE;
  }
}

static bool mccs_handler_installed = false;

void install_sigint_handler() {
  if (!SetConsoleCtrlHandler(&sigint_handler, TRUE)) {
    PRINT_ERR("ERROR: cannot install solver signal handler\n");
    exit(99);
  }
  mccs_handler_installed = true;
  return;
}

void restore_sigint_handler() {
  if (mccs_handler_installed && !SetConsoleCtrlHandler(&sigint_handler, FALSE)) {
    PRINT_ERR("ERROR: cannot restore solver signal handler\n");
    exit(99);
  }
  mccs_handler_installed = false;
  return;
}

#endif

// Allow C-c to interrupt the solver
Solver_return call_mccs_protected(Solver solver, char *criteria, int timeout, CUDFproblem* cpb) {
  Solver_return ret = { 0, "", cpb, NULL };
  try {
    install_sigint_handler();
    ret = call_mccs(solver, criteria, timeout, cpb, &mccs_current_solver);
    mccs_current_solver = NULL;
    restore_sigint_handler();
  } catch (...) {
    restore_sigint_handler();
    ret.error = "Uncaught solver exception";
  }
  return ret;
}

extern "C" value call_solver
(value ml_solver_backend, value ml_criteria, value ml_timeout, value ml_problem)
{
  CAMLparam3(ml_criteria, ml_timeout, ml_problem);
  CAMLlocal2(results, pkg);
  problem * pb = Problem_pt(ml_problem);
  CUDFproblem * cpb = pb->pb_cudf_problem;
  CUDFVirtualPackageList all_virtual_packages = *(cpb->all_virtual_packages);
  CUDFVersionedPackageList all_packages = *(cpb->all_packages);
  Solver_return ret;
  char* criteria = new char[strlen(String_val(ml_criteria))+3];
  Solver solver = ml2c_solver(ml_solver_backend);

  strcpy(criteria, "[");
  strcat(criteria, String_val(ml_criteria));
  strcat(criteria, "]");

  // caml_release_runtime_system ();
  ret = call_mccs_protected(solver, criteria, Int_val(ml_timeout), cpb);
  // caml_acquire_runtime_system ();
  delete[] criteria;
  switch (ret.success) {
  case 0: caml_failwith(ret.error);
  case -1: caml_raise_constant(*caml_named_value("Mccs.Timeout"));
  case -2: caml_raise_constant(*caml_named_value("Sys.Break"));
    // raise (SIGINT); Does not work well from C, better to raise directly
  }

  if (ret.solution == NULL) {
    if (ret.problem != cpb) delete ret.problem;
    fflush(stdout);
    CAMLreturn (Val_none);
  }
  else {
    results = Val_emptylist;
    for (vector<CUDFVersionedPackage*>::iterator ipkg = ret.problem->all_packages->begin();
         ipkg != ret.problem->all_packages->end();
         ipkg++)
      {
        if (ret.solution->get_solution(*ipkg)) {
          (*ipkg)->wasinstalled = (*ipkg)->installed;
          (*ipkg)->installed = 1;
          pkg = c2ml_package(*ipkg);
          results = Val_pair(pkg, results);
        }
      }
    if (ret.problem != cpb) delete ret.problem;
    delete ret.solution;
    fflush(stdout);
    CAMLreturn (Val_some(results));
  }
}

extern "C" value backends_list (value unit)
{
  CAMLparam1 (unit);
  CAMLlocal1 (r);
  r = Val_emptylist;
  if (has_backend(GLPK)) r = Val_pair (caml_hash_variant("GLPK"), r);
  if (has_backend(LP))
    r = Val_pair (Val_pair (caml_hash_variant("LP"), caml_copy_string("")), r);
  if (has_backend(CLP)) r = Val_pair (caml_hash_variant("COIN_CLP"), r);
  if (has_backend(CBC)) r = Val_pair (caml_hash_variant("COIN_CBC"), r);
  if (has_backend(SYMPHONY)) r = Val_pair (caml_hash_variant("COIN_SYMPHONY"), r);
  CAMLreturn (r);
}
