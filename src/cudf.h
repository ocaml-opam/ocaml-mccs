
/*******************************************************/
/* CUDF solver: cudf.h                                 */
/* Handling of CUDF problem files                      */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// main CUDF include file

#ifndef _CUDF_H
#define _CUDF_H

#ifdef __MINGW32__
// libstdc++ requires POSIX printf, as per comments in GCC's
// libstdc++-v3/config/os/mingw32-w64/os_defines.h. It's defined here to ensure
// that it's defined even if a C header is #include'd before the first C++
// header.
#define __USE_MINGW_ANSI_STDIO 1
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>

using namespace std;

#define PRINT_ERR(...) fprintf(stderr, __VA_ARGS__)
#define PRINT_OUT(...) fprintf(stdout, __VA_ARGS__)

// class predeclarations
class CUDFPropertyValue;
class CUDFPackage;
class CUDFVersionedPackage;
class CUDFVirtualPackage;
class CUDFPackage_comparator;
class CUDFProperty;


// type of list of property values and its iterator
typedef vector<CUDFPropertyValue *> CUDFPropertyValueList;
typedef vector<CUDFPropertyValue *>::iterator CUDFPropertyValueListIterator;

// type of the version of a package
typedef unsigned long long CUDFVersion;

// the different types of operations which can be done on a package
enum CUDFPackageOp { op_none, op_eq, op_neq, op_inf, op_sup, op_infeq, op_supeq};


// CUDF class for a vpkg
class CUDFVpkg {
 public:
  CUDFVirtualPackage *virtual_package;  // the package it operates on
  CUDFPackageOp op;                     // kind of operation
  CUDFVersion version;                  // version it operates on (e.g. 5 in p >= 5)
  
  // constructor
  CUDFVpkg(CUDFVirtualPackage *the_virtual_package, CUDFPackageOp the_op, CUDFVersion the_version) { 
    virtual_package = the_virtual_package;
    op = the_op;
    version = the_version;
  };
};

// true and false for a vpkg
extern CUDFVpkg *vpkg_true;
extern CUDFVpkg *vpkg_false;

// A package i.e. either a versioned package or a virtual package
class CUDFPackage {
 public:
  const char *name;           // package name as provided by the CUDF problem file
  int rank;                   // rank of the package, i.e., column number in a simplex matrix or variable id
  const char *versioned_name; // internal name of the package

  bool in_reduced;

  ~CUDFPackage();
};


// type of a CUDF Vpkg list and its iterator type
typedef vector<CUDFVpkg *> CUDFVpkgList;
typedef CUDFVpkgList::iterator CUDFVpkgListIterator;
// type of a CUDF Vpkg formula and its iterator type
typedef vector< CUDFVpkgList *> CUDFVpkgFormula;
typedef CUDFVpkgFormula::iterator CUDFVpkgFormulaIterator;

// The different types of keep operations
enum CUDFKeepOp { keep_none, keep_feature, keep_package, keep_version};

// Versioned package class (used to store a <package, version> couple)
class CUDFVersionedPackage: public CUDFPackage {
public:

  CUDFVersion version;                 // package version
  CUDFVpkgFormula *depends;            // package dependencies 
  CUDFVpkgList *conflicts;             // package conflicts
  CUDFVpkgList *provides;              // features provided by the package
  bool installed;                      // whether the package is installed in the initial configuration
  bool wasinstalled;                   // whether the package was installed
  CUDFKeepOp keep;                     // keep field value
  CUDFPropertyValueList properties;    // set of additional properties of the package

  CUDFVirtualPackage *virtual_package; // pointer to the virtual package the package belongs to

  CUDFVersionedPackage(const char *pkg_name, int my_rank);  // constructor
  ~CUDFVersionedPackage();

  void set_version(CUDFVersion pkg_version); // allows to set the package version (not always known at package creation)
};

// Compares two versioned package (used by ordered set packages likes CUDFVersionedPackageSet)
class CUDFPackage_comparator {
public:
  bool operator()(CUDFVersionedPackage *p1, CUDFVersionedPackage *p2) const {
    if (p1->version < p2->version)
      return true;
    else
      return false;
  }
};

// type of an ordered set of versioned packages and its iterator type
typedef set<CUDFVersionedPackage *, CUDFPackage_comparator> CUDFVersionedPackageSet;
typedef CUDFVersionedPackageSet::iterator CUDFVersionedPackageSetIterator;
// type of a list of providers and its iterator type
typedef vector<CUDFVersionedPackage *> CUDFProviderList;
typedef CUDFProviderList::iterator CUDFProviderListIterator;
// type of a list of versioned providers and its iterator type
typedef map<CUDFVersion, CUDFProviderList> CUDFVersionedProviderList;
typedef CUDFVersionedProviderList::iterator CUDFVersionedProviderListIterator;

// Virtual packages or features
class CUDFVirtualPackage: public CUDFPackage {
public:

  CUDFVersionedPackageSet all_versions;              // set of all versions of the package (versioned packages with the same name)
  CUDFVersionedPackage *highest_installed;           // highest version of the installed versioned packages
  CUDFVersion highest_version;                       // highest available version of the versioned packages

  CUDFProviderList providers;                        // list of all the packages (with a different name than the virtual package name)
                                                     //   which provide this feature (without providing a version)
  CUDFVersionedProviderList versioned_providers;     // list of all the packages (with a different name than the virtual package name)
                                                     //   which provide this feature with a version
  CUDFVersion highest_installed_provider_version;    // highest installed version of the providers

  CUDFVirtualPackage(const char *pkg_name, int my_rank);   // constructor
  ~CUDFVirtualPackage();
};


// type of a list of versioned packages and its iterator type
typedef vector<CUDFVersionedPackage *> CUDFVersionedPackageList;
typedef CUDFVersionedPackageList::iterator CUDFVersionedPackageListIterator;
// type of a list of virtual packages and its iterator type
typedef vector<CUDFVirtualPackage *> CUDFVirtualPackageList;
typedef CUDFVirtualPackageList::iterator CUDFVirtualPackageListIterator;

// type of an enums and its iterator type
typedef vector<const char *> CUDFEnums;
typedef vector<const char *>::iterator CUDFEnumsIterator;

// get enum "estr" from "e" enum list
extern const char *get_enum(CUDFEnums *e, const char *estr);


// a property mapping type and its iterator type
typedef map<string, CUDFProperty *> CUDFProperties;
typedef CUDFProperties::iterator  CUDFPropertiesIterator;

// all user defined properties
// extern CUDFProperties properties;

// Types allowed for properties
enum CUDFPropertyType { pt_none,
                        pt_bool, pt_int, pt_nat, pt_posint, pt_enum, pt_string, 
			pt_vpkg, pt_veqpkg, pt_vpkglist, pt_veqpkglist, pt_vpkgformula};


// Class to describe user defined property values
class CUDFPropertyValue {
 public:
  CUDFProperty *property;           // type of the property value
  int intval;                       // use to store property value when its basic type is an int
  const char *strval;               // use to store property value when its basic type is a string
  CUDFVpkg *vpkg;                   // use to store property value when its basic type is a vpkg
  CUDFVpkgList *vpkglist;           // use to store property value when its basic type is a vpkglist
  CUDFVpkgFormula *vpkgformula;     // use to store property value when its basic type is a vpkgformula

  CUDFPropertyValue(CUDFProperty *the_property, int the_value);              // constructor for int valued properties
  CUDFPropertyValue(CUDFProperty *the_property, const char *the_value);      // constructor for string valued properties
  CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkg *the_value);        // constructor for vpkg valued properties
  CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkgList *the_value);    // constructor for vpkglist valued properties
  CUDFPropertyValue(CUDFProperty *the_property, CUDFVpkgFormula *the_value); // constructor for vpkgformula valued properties

  ~CUDFPropertyValue();
};

// Class to describe user defined properties
class CUDFProperty {
 public:
  const char *name;                   // property name

  CUDFPropertyType type_id;           // property type
  CUDFEnums *enuml;                   // allowed enum values for enum type properties

  bool required;                      // whether the property id required or not

  CUDFPropertyValue *default_value;   // default property value

  CUDFProperty(const char *tname, CUDFPropertyType ttype);                           // CUDF property constructors (without and with default value)
  CUDFProperty(const char *tname, CUDFPropertyType ttype, int tdefault);
  CUDFProperty(const char *tname, CUDFPropertyType ttype, const char *tdefault);
  CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFEnums *tenum);
  CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFEnums *tenum, const char *tident);
  CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFVpkg *tdefault);
  CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFVpkgList *tdefault);
  CUDFProperty(const char *tname, CUDFPropertyType ttype, CUDFVpkgFormula *tdefault);

  ~CUDFProperty();

};



// A CUDF problem class
class CUDFproblem {
public:
  // Problem's properties
  CUDFProperties *properties; // list of user property declarations

  // Problem's packages
  CUDFVersionedPackageList *all_packages;               // list of all the versioned packages of the initial configuration
  CUDFVersionedPackageList *installed_packages;         // list of all installed versioned packages in the initial configuration
  CUDFVersionedPackageList *uninstalled_packages;       // list of all uninstalled versioned packages in the inital configuration

  // Problem's virtual packages
  CUDFVirtualPackageList *all_virtual_packages;                        // list of all the virtual packages in the initial configuration

  // Problem's requests
  CUDFVpkgList *install;  // Vpkg list of the packages to install
  CUDFVpkgList *remove;   // Vpkg list of the packages to remove
  CUDFVpkgList *upgrade;  // Vpkg list of the packages to upgrade

  CUDFproblem() {         // constructor
    install = (CUDFVpkgList *)NULL;
    remove = (CUDFVpkgList *)NULL;
    upgrade = (CUDFVpkgList *)NULL;
  }

  ~CUDFproblem();
};

// current CUDF problem
/* extern CUDFproblem *the_problem;
 * 
 * // list of all the versioned packages of the initial configuration  
 * extern CUDFVersionedPackageList all_packages;
 * // list of all installed versioned packages in the initial configuration
 * extern CUDFVersionedPackageList installed_packages;
 * // list of all uninstalled versioned packages in the inital configuration
 * extern CUDFVersionedPackageList uninstalled_packages; */

// // list of all the virtual packages in the initial configuration
// extern CUDFVirtualPackageList all_virtual_packages;


// parse the CUDF problem from input_file
extern int parse_cudf(FILE *input_file);

/* // parse the CUDF problem from a string */
extern int parse_cudf_string(const char *str);

// operations to compare a package version to another one
extern bool op_none_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_eq_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_neq_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_sup_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_supeq_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_inf_comp(CUDFVersion v1, CUDFVersion v2);
extern bool op_infeq_comp(CUDFVersion v1, CUDFVersion v2);

typedef bool (*a_compptr)(CUDFVersion, CUDFVersion);

// get the comparison operation from a package operation
extern a_compptr get_comparator(CUDFPackageOp op);



// Printing out capabilities
extern void print_enum(FILE *output, CUDFEnums *enuml);                 // print out a list of enums
extern void print_properties(FILE *output, CUDFProperties *properties); // print out a property


// print out a versioned package as installed if install parameter is true, uninstalled otherwise
extern void print_versioned_package_with_install(FILE *output, CUDFVersionedPackage *pkg, bool install, bool wasinstalled);
// print out a versioned package 
extern void print_versioned_package(FILE *output, CUDFVersionedPackage *pkg, bool wasinstalled);
// print out a versioned package as installed
extern void print_versioned_package_as_installed(FILE *output, CUDFVersionedPackage *pkg, bool wasinstalled);


// print out a virtual package
extern void print_virtual_package(FILE *output, CUDFVirtualPackage *vpkg);


// print out a problem
extern void print_problem(FILE *output, CUDFproblem *pbs);

// handling verbosity level
extern int verbosity;

#endif
