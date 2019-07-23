
/*******************************************************/
/* CUDF solver: cudf_tools.h                           */
/* Tools to handle CUDF problems                       */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cudf.h>
#include <cudf_types.h>

// Handling verbosity level
int verbosity = 0;

// List of all the user declared properties
// CUDFProperties properties;

// True and false for Vpkg
CUDFVpkg *vpkg_true = new CUDFVpkg((CUDFVirtualPackage *)NULL, op_none, 0);
CUDFVpkg *vpkg_false = new CUDFVpkg((CUDFVirtualPackage *)NULL, op_none, 0);

CUDFPackage::~CUDFPackage() {
  free((char *)name);
  if (versioned_name != NULL && versioned_name != name)
    free((char *)versioned_name);
}

// Versioned package constructor
// requires package name and unique package ident (i.e. simplex column number)
CUDFVersionedPackage::CUDFVersionedPackage(const char *pkg_name, int my_rank) {

  // Get some piece of memory to store package name
  if (!(name = strdup(pkg_name))) {
    PRINT_ERR("error: cannot alloc name for CUDFVersionedPackage.\n");
    exit(-1);
  }
  
  // Default initialization of variables
  versioned_name = (char *)NULL;
  version = 0;
  installed = false;
  wasinstalled = false;
  keep = keep_none;
  depends = (CUDFVpkgFormula *)NULL;
  conflicts = (CUDFVpkgList *)NULL;
  provides = (CUDFVpkgList *)NULL;

  virtual_package = (CUDFVirtualPackage *)NULL;
  
  rank = my_rank;

  in_reduced = true;
}

CUDFVersionedPackage::~CUDFVersionedPackage() {
  if (depends != NULL) {
    for (vector<vector<CUDFVpkg*>*>::iterator it = depends->begin(); it != depends->end(); it++) {
      for (vector<CUDFVpkg*>::iterator it2 = (*it)->begin(); it2 != (*it)->end(); it2++)
        delete (*it2);
      delete (*it);
    }
    delete depends;
  }
  if (conflicts != NULL) {
    for (vector<CUDFVpkg*>::iterator it = conflicts->begin(); it != conflicts->end(); it++)
      delete (*it);
    delete conflicts;
  }
  if (provides != NULL) {
    for (vector<CUDFVpkg*>::iterator it = provides->begin(); it != provides->end(); it++)
      delete (*it);
    delete provides;
  }
  for (vector<CUDFPropertyValue*>::iterator it = properties.begin(); it != properties.end(); it++)
    delete (*it);
}

// Set the version of a package
// requires package version
// assumes that package name is already available
void CUDFVersionedPackage::set_version(CUDFVersion pkg_version) {
  static char temp[50];

  sprintf(temp, "%" CUDFint64"u", pkg_version);
  if ((versioned_name = (const char *)malloc(strlen(name)+strlen(temp)+2)) == NULL) {
    PRINT_ERR("error: cannot alloc versioned_name for CUDFVersionedPackage.\n");
    exit(-1);
  }
  sprintf((char *)versioned_name, "%s_%s", name, temp);
    
  version = pkg_version;
}

// Virtual package constructor
// requires virtual package name and virtual package rank
CUDFVirtualPackage::CUDFVirtualPackage(const char *pkg_name, int my_rank) {    
  if (!(name = strdup(pkg_name))) {
    PRINT_ERR("error: cannot alloc name for CUDFVirtualPackage.\n");
    exit(-1);
  }
  versioned_name = name;
  
  highest_installed = (CUDFVersionedPackage *)NULL;
  highest_version = 0;
  highest_installed_provider_version = 0;
  
  rank = my_rank;

  in_reduced = true;
}

CUDFVirtualPackage::~CUDFVirtualPackage() {
  ;
}

// User property constructor
// requires property name and type of the property
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = true;
  default_value = (CUDFPropertyValue *)NULL;

}


// User property constructor
// requires property name, type of the property (must be an int subtype) and int default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, int tdefault) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = false;

  // Checking property
  if ((type_id == pt_bool) && (tdefault != 0) && (tdefault != 1)) {
    PRINT_ERR("CUDF error: default value for property %s: bool must be true or false.\n", tname);
    exit(-1);
  }

  if ((type_id == pt_nat) && (tdefault < 0)) {
    PRINT_ERR("CUDF error: default value for property %s: nat must be >= 0.\n", tname);
    exit(-1);
  }

  if ((type_id == pt_posint) && (tdefault <= 0)) {
    PRINT_ERR("CUDF error: default value for property %s: posint must be > 0.\n", tname);
    exit(-1);
  }

  default_value = new CUDFPropertyValue(this, tdefault);
}

CUDFProperty::~CUDFProperty() {
  free((char *)name);
  if (type_id == pt_enum) {
    for (vector<const char*>::iterator it = enuml->begin(); it != enuml->end(); it++)
      free((char*)*it);
    delete enuml;
  }
  delete default_value;
}

// User property constructor
// requires property name, type of the property (must be a string subtype) and string default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, const char *tdefault) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault); // unsure of that
}

// User property constructor
// requires property name, type of the property (must be a enum type) and enum default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFEnums *tenum) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = true;

  enuml = tenum;
  default_value = (CUDFPropertyValue *)NULL;

}

// User property constructor
// requires property name, type of the property (must be a enum type), the list of allowed enum values and enum default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFEnums *tenum, const char *tident) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = true;

  enuml = tenum;

  const char *defval = get_enum(tenum, tident);
  if (defval == NULL) {
    PRINT_ERR("CUDF error: property %s default value can not be %s.\n", tname, tident);
    exit(-1);
  } else
    default_value = new CUDFPropertyValue(this, defval);

}

// User property constructor
// requires property name, type of the property (must be a vpkg type) and vpkg default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFVpkg *tdefault) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault);
}


// User property constructor
// requires property name, type of the property (must be a vpkglist type) and vpkglist default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFVpkgList *tdefault) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault);
}


// User property constructor
// requires property name, type of the property (must be a vpkgformula type) and vpkgformula default value
CUDFProperty::CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFVpkgFormula *tdefault) {
  if (!(name = strdup(tname))) {
    PRINT_ERR("error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault);
}


// User property value constructor
// requires a pointer to the user property (must be an int subtype) and its int value
CUDFPropertyValue::CUDFPropertyValue(CUDFProperty *the_property, int the_value) {
  property = the_property;
  intval = the_value;
}

// User property value constructor
// requires a pointer to the user property (must be a string subtype) and its string value
CUDFPropertyValue::CUDFPropertyValue(CUDFProperty *the_property, const char *the_value) {
  char *the_nvalue = (char *)malloc(strlen(the_value)+1);

  property = the_property;
  strval = the_nvalue;
  strcpy(the_nvalue, the_value);
}

// User property value constructor
// requires a pointer to the user property (must be a vpkg type) and its vpkg value
CUDFPropertyValue::CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkg *the_value) {
  property = the_property;
  vpkg = the_value;
}

// User property value constructor
// requires a pointer to the user property (must be a vpkglist type) and its vpkglist value
CUDFPropertyValue::CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkgList *the_value) {
  property = the_property;
  vpkglist = the_value;
}

// User property value constructor
// requires a pointer to the user property (must be a vpkgformula type) and its vpkgformula value
CUDFPropertyValue::CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkgFormula *the_value) {
  property = the_property;
  vpkgformula = the_value;
}

CUDFPropertyValue::~CUDFPropertyValue() {
  switch (property->type_id) {
  case pt_string: free((char *)strval); break;
  case pt_vpkg: case pt_veqpkg: delete vpkg; break;
  case pt_vpkglist: case pt_veqpkglist: delete vpkglist; break;
  case pt_vpkgformula: delete vpkgformula; break;
  default: ;
  }
}

CUDFproblem::~CUDFproblem() {
  delete all_packages;
  delete installed_packages;
  delete uninstalled_packages;
  delete all_virtual_packages;
  // these are shared between problem / reduced problem
  // delete install;
  // delete remove;
  // delete upgrade;
  // delete properties;
}

// Operators to compare two version
// requires the two version to compare
bool op_none_comp(CUDFVersion v1, CUDFVersion v2) { return true; }
bool op_eq_comp(CUDFVersion v1, CUDFVersion v2)   { return (v1 == v2); }
bool op_neq_comp(CUDFVersion v1, CUDFVersion v2)  { return (v1 != v2); }
bool op_sup_comp(CUDFVersion v1, CUDFVersion v2)   { return (v1 > v2); }
bool op_supeq_comp(CUDFVersion v1, CUDFVersion v2)  { return (v1 >= v2); }
bool op_inf_comp(CUDFVersion v1, CUDFVersion v2)   { return (v1 < v2); }
bool op_infeq_comp(CUDFVersion v1, CUDFVersion v2)  { return (v1 <= v2); }


// Gives back a comparison operator according to its parameter value
a_compptr get_comparator(CUDFPackageOp op) {
  switch (op) {
  case op_none:  return op_none_comp;  break;
  case op_eq:    return op_eq_comp;    break;
  case op_neq:   return op_neq_comp;   break;
  case op_sup:   return op_sup_comp;   break;
  case op_supeq: return op_supeq_comp; break;
  case op_inf:   return op_inf_comp;   break;
  case op_infeq: return op_infeq_comp; break;
  }
  return op_none_comp;
}
