
/*******************************************************/
/* CUDF solver: constraint_generation.h                */
/* constraint generation for cudf problems             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// 

#ifndef _CONSTRAINT_GENERATION_H_
#define _CONSTRAINT_GENERATION_H_

#include "cudf.h"
#include "abstract_solver.h"
#include "criteria.h"
#include "combiner.h"
#include "cudf_reductions.h"

extern bool generate_desagregate_constraints;
extern bool generate_agregate_constraints;

// class used to manage set of upgrade requests
class an_upgrade_set {
 public:
  int nb_new_var;
  int first_var_rank;
  CUDFVersionedPackageList remove_set;
  CUDFVersionedProviderList upgrade_set;

  an_upgrade_set(int nbv, int fvr, CUDFVersionedPackageList rs, CUDFVersionedProviderList us) {
    nb_new_var = nbv;
    first_var_rank = fvr;
    remove_set = rs;
    upgrade_set = us;
  }
};

extern int new_var;

// set of requested upgrades
extern vector<an_upgrade_set> upgrades;

// constructor for int valued properties
extern bool is_in_provl(const CUDFVersionedPackage *pkg, CUDFProviderList *provl);

// check if pkg belongs to a list of removed packages
extern bool is_in_remove(const CUDFVersionedPackage *pkg, CUDFVersionedPackageList *remove_set);

// check if pkg, version belongs to a sublist of providers 
extern bool is_in_versioned_providers(const CUDFVersionedPackage *pkg, const CUDFVersion version,
				      const CUDFVersionedProviderListIterator vpbegin, 
				      const CUDFVersionedProviderListIterator vpend);


// preprocess an upgrade request to insure version unicity
extern int preprocess_upgrade();

// available criteria
#define CRIT_DEFAULT 1
#define CRIT_CONSERVATIVE 2
#define CRIT_FRESHER 3
#define CRIT_SFRESHER 4
#define CRIT_FFRESHER 5
#define CRIT_TFRESHER 5
#define CRIT_PARANOID 6
#define CRIT_TRENDY   7
#define CRIT_TRENDY2  8
#define CRIT_LEXPARANOID  9
#define CRIT_LEXTRENDY   10
#define CRIT_LEXTRENDY2  11

// main function for constraint generation (translate a CUDF problem into MILP problem for a given solver and a given criteria)
extern int generate_constraints(CUDFproblem *problem, abstract_solver &solver, abstract_combiner &combiner);

#endif

