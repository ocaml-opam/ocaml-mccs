
/*******************************************************/
/* CUDF solver: cudf_tools.h                           */
/* Tools to handle CUDF problems                       */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cudf.h>

// Handling verbosity level
int verbosity = 0;

// List of all the user declared properties
CUDFProperties properties;

// True and false for Vpkg
CUDFVpkg *vpkg_true = new CUDFVpkg((CUDFVirtualPackage *)NULL, op_none, 0);
CUDFVpkg *vpkg_false = new CUDFVpkg((CUDFVirtualPackage *)NULL, op_none, 0);

// Versioned package constructor
// requires package name and unique package ident (i.e. simplex column number)
CUDFVersionedPackage::CUDFVersionedPackage(const char *pkg_name, int my_rank) {

  // Get some piece of memory to store package name
  if ((name = (char *)malloc(strlen(pkg_name)+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for CUDFVersionedPackage.\n");
    exit(-1);
  }
  strcpy(name, pkg_name);
  
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

  in_reduced = false;
}

// Set the version of a package
// requires package version
// assumes that package name is already available
void CUDFVersionedPackage::set_version(CUDFVersion pkg_version) {
  static char temp[50];

  sprintf(temp, "%llu", pkg_version);
  if ((versioned_name = (char *)malloc(strlen(name)+strlen(temp)+2)) == NULL) {
    fprintf(stderr, "error: cannot alloc versioned_name for CUDFVersionedPackage.\n");
    exit(-1);
  }
  sprintf(versioned_name, "%s_%s", name, temp);
    
  version = pkg_version;
}

// Virtual package constructor
// requires virtual package name and virtual package rank
CUDFVirtualPackage::CUDFVirtualPackage(const char *pkg_name, int my_rank) {    
  if ((name = (char *)malloc(strlen(pkg_name)+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for CUDFVirtualPackage.\n");
    exit(-1);
  }
  strcpy(name, pkg_name);
  versioned_name = name;
  
  highest_installed = (CUDFVersionedPackage *)NULL;
  highest_version = 0;
  highest_installed_provider_version = 0;
  
  rank = my_rank;

  in_reduced = false;
}

// User property constructor
// requires property name and type of the property
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = true;
  default_value = (CUDFPropertyValue *)NULL;

}


// User property constructor
// requires property name, type of the property (must be an int subtype) and int default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, int tdefault) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = false;

  // Checking property
  if ((type_id == pt_bool) && (tdefault != 0) && (tdefault != 1)) {
    fprintf(stderr, "CUDF error: default value for property %s: bool must be true or false.\n", tname);
    exit(-1);
  }

  if ((type_id == pt_nat) && (tdefault < 0)) {
    fprintf(stderr, "CUDF error: default value for property %s: nat must be >= 0.\n", tname);
    exit(-1);
  }

  if ((type_id == pt_posint) && (tdefault <= 0)) {
    fprintf(stderr, "CUDF error: default value for property %s: posint must be > 0.\n", tname);
    exit(-1);
  }

  default_value = new CUDFPropertyValue(this, tdefault);
}


// User property constructor
// requires property name, type of the property (must be a string subtype) and string default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, char *tdefault) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault); // unsure of that
}

// User property constructor
// requires property name, type of the property (must be a enum type) and enum default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFEnums *tenum) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = true;

  enuml = tenum;
  default_value = (CUDFPropertyValue *)NULL;

}

// User property constructor
// requires property name, type of the property (must be a enum type), the list of allowed enum values and enum default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFEnums *tenum, char *tident) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = true;

  enuml = tenum;

  char *defval = get_enum(tenum, tident);
  if (defval == (char *)NULL) {
    fprintf(stderr, "CUDF error: property %s default value can not be %s.\n", tname, tident);
    exit(-1);
  } else
    default_value = new CUDFPropertyValue(this, defval);

}

// User property constructor
// requires property name, type of the property (must be a vpkg type) and vpkg default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFVpkg *tdefault) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault);
}


// User property constructor
// requires property name, type of the property (must be a vpkglist type) and vpkglist default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFVpkgList *tdefault) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
  type_id = ttype;
  required = false;
  default_value = new CUDFPropertyValue(this, tdefault);
}


// User property constructor
// requires property name, type of the property (must be a vpkgformula type) and vpkgformula default value
CUDFProperty::CUDFProperty(char *tname, CUDFPropertyType ttype, CUDFVpkgFormula *tdefault) {
  int lgth = strlen(tname);

  if ((name = (char *)malloc(lgth+1)) == NULL) {
    fprintf(stderr, "error: cannot alloc name for property %s.\n", tname);
    exit(-1);
  }
  strcpy(name, tname);
    
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
CUDFPropertyValue::CUDFPropertyValue(CUDFProperty *the_property, char *the_value) {
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

// Print out a vpkg
// requires the file descriptor of the targetted file and a vpkg pointer
void print_vpkg(FILE *output, CUDFVpkg *vpkg) {
  if (vpkg == vpkg_true) 
    fprintf(output, "true!");
  else if (vpkg == vpkg_false) 
    fprintf(output, "false!");
  else {
    fprintf(output, "%s", vpkg->virtual_package->name);
    switch (vpkg->op) {
    case op_none: break;
    case op_eq: fprintf(output, " = %llu", vpkg->version); break;
    case op_neq: fprintf(output, " != %llu", vpkg->version); break;
    case op_inf: fprintf(output, " < %llu", vpkg->version); break;
    case op_infeq: fprintf(output, " <= %llu", vpkg->version); break;
    case op_sup: fprintf(output, " > %llu", vpkg->version); break;
    case op_supeq: fprintf(output, " >= %llu", vpkg->version); break;
    }
  }
}

// Print out a vpkglist
// requires the file descriptor of the targetted file and a vpkglist pointer
void print_vpkglist(FILE *output, vector<CUDFVpkg *> *vpkglist, bool is_or) {
  CUDFVpkgListIterator ipkg;

  if ((vpkglist != (CUDFVpkgList *)NULL) && ((*vpkglist).size() > 0)) {
    ipkg = vpkglist->begin();
    print_vpkg(output, *ipkg);
    ipkg++;
    for (; ipkg != vpkglist->end(); ipkg++) { 
      if (is_or) fprintf(output, " | "); else fprintf(output, ", "); 
      print_vpkg(output, *ipkg); 
    }
  }
}

// Print out a vpkgformula
// requires the file descriptor of the targetted file and a vpkgformula pointer
void print_vpkgformula(FILE *output, CUDFVpkgFormula *vpkgformula) {
  CUDFVpkgFormulaIterator ipkg;

  if (vpkgformula != (CUDFVpkgFormula *)NULL) {
    ipkg = vpkgformula->begin();
    print_vpkglist(output, *ipkg, true);
    ipkg++;
    for (; ipkg != vpkgformula->end(); ipkg++) { fprintf(output, ", "); print_vpkglist(output, *ipkg, true); }
  }
}

// Print out a list of enums
// requires the file descriptor of the targetted file and a pointer to the list of enums
void print_enum(FILE *output, CUDFEnums *enuml) {
  CUDFEnumsIterator i = enuml->begin();
  if (i != enuml->end()) {
    fprintf(output, "enum[%s", *i);
    i++;
    for ( ; i != enuml->end(); i++)
      fprintf(output, ", %s", *i);
    fprintf(output, "]");
  }
}

// Print out a string
// requires the file descriptor of the targetted file and a string pointer
void print_string(FILE *output, char *str) {
  int lgth = strlen(str);
  fprintf(output, "\"");
  for (int i = 0; i < lgth; i++)
    if (str[i] == '"')
      fprintf(output, "\\\"");
    else
      fprintf(output, "%c", str[i]);
  fprintf(output, "\"");
}

// Print out a list of properties
// requires the file descriptor of the targetted file and a pointer to the list of properties
void print_properties(FILE *output, CUDFProperties *properties) {
  if (properties != (CUDFProperties *)NULL) {
    char sep = ' ';
    fprintf(output, "property:");
    for (CUDFPropertiesIterator adecl = properties->begin(); adecl != properties->end(); adecl++) {
      fprintf(output, "%c %s ", sep, adecl->second->name);
      switch (adecl->second->type_id) {
      case pt_bool: 
	if (adecl->second->required)
	  fprintf(output, "bool");
	else {
	  if (adecl->second->default_value->intval == 0)
	    fprintf(output, "bool = [false]");
	  else
	    fprintf(output, "bool = [true]");
	}
	break;
      case pt_int:
	if (adecl->second->required)
	  fprintf(output, "int");
	else
	  fprintf(output, "int = [%d]", adecl->second->default_value->intval);
	break;
      case pt_nat:
	if (adecl->second->required)
	  fprintf(output, "nat");
	else
	  fprintf(output, "nat = [%d]", adecl->second->default_value->intval);
	break;
      case pt_posint:
	if (adecl->second->required)
	  fprintf(output, "posint");
	else
	  fprintf(output, "posint = [%d]", adecl->second->default_value->intval);
	break;
      case pt_string:
	if (adecl->second->required)
	  fprintf(output, "string");
	else {
	  fprintf(output, "string = [");
	  print_string(output,  adecl->second->default_value->strval);
	  fprintf(output, "]");
	}
	break;
      case pt_enum:
	print_enum(output, adecl->second->enuml);
	if (adecl->second->required)
	  fprintf(output, " = [%s]", adecl->second->default_value->strval);
	break;
      case pt_vpkg:
	if (adecl->second->required)
	  fprintf(output, "vpkg");
	else {
	  fprintf(output, "vpkg = [");
	  print_vpkg(output,  adecl->second->default_value->vpkg);
	  fprintf(output, "]");
	}
	break;
      case pt_veqpkg:
	if (adecl->second->required)
	  fprintf(output, "veqpkg");
	else {
	  fprintf(output, "veqpkg = [");
	  print_vpkg(output,  adecl->second->default_value->vpkg);
	  fprintf(output, "]");
	}
	break;
      case pt_vpkglist:
	if (adecl->second->required)
	  fprintf(output, "vpkglist");
	else {
	  fprintf(output, "vpkglist = [");
	  print_vpkglist(output,  adecl->second->default_value->vpkglist, false);
	  fprintf(output, "]");
	}
	break;
      case pt_veqpkglist:
	if (adecl->second->required)
	  fprintf(output, "veqpkglist");
	else {
	  fprintf(output, "veqpkglist = [");
	  print_vpkglist(output,  adecl->second->default_value->vpkglist, false);
	  fprintf(output, "]");
	}
	break;
      case pt_vpkgformula:
	if (adecl->second->required)
	  fprintf(output, "vpkgformula");
	else {
	  fprintf(output, "vpkgformula = [");
	  print_vpkgformula(output,  adecl->second->default_value->vpkgformula);
	  fprintf(output, "]");
	}
	break;
      case pt_none:
	fprintf(output, "NONE!!");
	break;
      }
      sep = ',';
    }
  }
}

// Print out a versioned package (according to its install boolean parameter value)
// requires the file descriptor of the targetted file, a pointer to the versioned package, whether it should
// be printed out as installed or uninstalled, whether it was installed or not
void print_versioned_package_with_install(FILE *output, CUDFVersionedPackage *pkg, bool install, bool wasinstalled) {
  fprintf(output, "package: %s\n", pkg->name);
  fprintf(output, "version: %llu\n", pkg->version);
  if (pkg->depends != (CUDFVpkgFormula *)NULL) {
    fprintf(output, "depends: ");
    print_vpkgformula(output, pkg->depends);
    fprintf(output, "\n");
  }
  if (pkg->provides != (CUDFVpkgList *)NULL) {
    fprintf(output, "provides: ");
    print_vpkglist(output, pkg->provides, false);
    fprintf(output, "\n");
  }
  if (pkg->conflicts != (CUDFVpkgList *)NULL) {
    fprintf(output, "conflicts: ");
    print_vpkglist(output, pkg->conflicts, false);
    fprintf(output, "\n");
  }
  if (install) fprintf(output, "installed: true\n");
  if (wasinstalled) if (pkg->installed) fprintf(output, "was-installed: true\n"); //else fprintf(output, "was-installed: false\n");
  switch(pkg->keep) {
  case keep_none: break;
  case keep_feature: fprintf(output, "keep: feature\n"); break;
  case keep_package: fprintf(output, "keep: package\n"); break;
  case keep_version: fprintf(output, "keep: version\n"); break;
  }
  for (CUDFPropertyValueListIterator ip = pkg->properties.begin(); ip != pkg->properties.end(); ip++)
    switch((*ip)->property->type_id) {
    case pt_bool: 
      if ((*ip)->intval == 0) 
	fprintf(output, "%s false\n", (*ip)->property->name);
      else 
	fprintf(output, "%s true\n", (*ip)->property->name);
      break;
    case pt_int: 
    case pt_nat: 
    case pt_posint: 
      fprintf(output, "%s %d\n", (*ip)->property->name, (*ip)->intval);
      break;
    case pt_enum: 
      fprintf(output, "%s %s\n", (*ip)->property->name, (*ip)->strval);
      break;
    case pt_string:
      fprintf(output, "%s %s\n", (*ip)->property->name, (*ip)->strval);
      break;
    case pt_vpkg:
    case pt_veqpkg:
      fprintf(output, "%s ", (*ip)->property->name);
      print_vpkg(output, (*ip)->vpkg);
      fprintf(output, "\n");
      break;
    case pt_vpkglist:
    case pt_veqpkglist:
      fprintf(output, "%s ", (*ip)->property->name);
      print_vpkglist(output, (*ip)->vpkglist, false);
      fprintf(output, "\n");
      break;
    case pt_vpkgformula:
      fprintf(output, "%s ", (*ip)->property->name);
      print_vpkgformula(output, (*ip)->vpkgformula);
      fprintf(output, "\n");
      break;
    case pt_none:
      fprintf(output, "%s ", (*ip)->property->name);
      fprintf(output, "NONE!!\n");
      break;
    }
  fprintf(output, "\n");
}

// Print out a versioned package
// requires the file descriptor of the targetted file, a pointer to the versioned package and whether it was installed or not
void print_versioned_package(FILE *output, CUDFVersionedPackage *pkg, bool wasinstalled) {
  print_versioned_package_with_install(output, pkg, pkg->installed, wasinstalled);
}

// Print out a versioned package as installed
// requires the file descriptor of the targetted file, a pointer to the versioned package and whether it was installed or not
void print_versioned_package_as_installed(FILE *output, CUDFVersionedPackage *pkg, bool wasinstalled) {
  print_versioned_package_with_install(output, pkg, true, wasinstalled);
}


// Print out a virtual package
// requires the file descriptor of the targetted file and a pointer to the virtual package
void print_virtual_package(FILE *output, CUDFVirtualPackage *vpkg) {
  bool out;
  char sep;

  fprintf(output, "virtual package: %s\n", vpkg->name);

  fprintf(output, "versions:"); sep = ' ';
  if (vpkg->all_versions.size() > 0)
    for (CUDFVersionedPackageSetIterator jpkg = vpkg->all_versions.begin(); jpkg != vpkg->all_versions.end(); jpkg++) {
      fprintf(output, "%c %s = %llu", sep, (*jpkg)->name, (*jpkg)->version);
      sep = ',';
    }
  else
    fprintf(output, " none");
  fprintf(output, "\n");

  fprintf(output, "providers (any version):"); sep = ' ';
  if (vpkg->providers.size() > 0)
    for (CUDFProviderListIterator jpkg = vpkg->providers.begin(); jpkg != vpkg->providers.end(); jpkg++) {
      fprintf(output, "%c %s = %llu", sep, (*jpkg)->name, (*jpkg)->version);
      sep = ',';
    }
  else
    fprintf(output, " none");
  fprintf(output, "\n");

  fprintf(output, "versioned providers:"); out = false;
  for (CUDFVersionedProviderListIterator jpkg = vpkg->versioned_providers.begin(); jpkg != vpkg->versioned_providers.end(); jpkg++) {
    out = true;
    fprintf(output, "\n\t%llu:", jpkg->first);
    sep = ' ';
    for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++) {
      fprintf(output, "%c %s = %llu", sep, (*kpkg)->name, (*kpkg)->version);
      sep = ',';
    }
  }
  if (! out) fprintf(output, " none");
  fprintf(output, "\n");


  fprintf(output, "highest installed provider version: %llu\n", vpkg->highest_installed_provider_version);

  if (vpkg->highest_installed != (CUDFVersionedPackage *)NULL)
    fprintf(output, "highest version installed: %s = %llu\n", vpkg->highest_installed->name, vpkg->highest_installed->version);
  else
    fprintf(output, "highest version installed: none\n");
  fprintf(output, "\n");
}


// Print out a CUDF problem
// requires the file descriptor of the targetted file and a pointer to the CUDF problem
void print_problem(FILE *output, CUDFproblem *pbs) {
  fprintf(output, "request:\n");
  if (pbs != (CUDFproblem *)NULL) {
    if (pbs->install != (CUDFVpkgList *)NULL) {
      fprintf(output, "install: ");
      print_vpkglist(output, pbs->install, false);
      fprintf(output, "\n");
    }
    if (pbs->remove != (CUDFVpkgList *)NULL) {
      fprintf(output, "remove: ");
      print_vpkglist(output, pbs->remove, false);
      fprintf(output, "\n");
    }
    if (pbs->upgrade != (CUDFVpkgList *)NULL) {
      fprintf(output, "upgrade: ");
      print_vpkglist(output, pbs->upgrade, false);
      fprintf(output, "\n");
    }
  }
}
