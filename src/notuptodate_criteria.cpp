
/*******************************************************/
/* CUDF solver: notuptodate_criteria.c                 */
/* Implementation the notuptodate criteria             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include "notuptodate_criteria.h"


// Implementation of the not up to date criteria
// 


// Criteria initialization
void notuptodate_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->problem = problem;
  this->solver = solver;

  ub = 0;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); 
       ivpkg != problem->all_virtual_packages->end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() > 1) ub++;
}

// Computing the number of columns required to handle the criteria
int notuptodate_criteria::set_variable_range(int first_free_var) {
  this->first_free_var = first_free_var;
  return first_free_var + ub;
}

// Add the criteria to the current objective function
int notuptodate_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); 
       ivpkg != problem->all_virtual_packages->end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() > 1) solver->set_obj_coeff(ivpkg_rank++, lambda_crit * lambda);
  return 0;
}

// Add the criteria to the constraint set
int notuptodate_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); 
       ivpkg != problem->all_virtual_packages->end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() > 1) solver->set_constraint_coeff(ivpkg_rank++, lambda_crit * lambda);
  return 0;
}

// Add the constraints required by the criteria
int notuptodate_criteria::add_constraints() {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); 
       ivpkg != problem->all_virtual_packages->end(); ivpkg++) {
    int size = (*ivpkg)->all_versions.size();
    if (size > 1) {
      solver->new_constraint();
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	if ((*vers_pkg)->version == (*ivpkg)->highest_version)
	  solver->set_constraint_coeff((*vers_pkg)->rank, -(size - 1));
	else
	  solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, -size);
      solver->add_constraint_leq(+0);
      solver->new_constraint();
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	if ((*vers_pkg)->version == (*ivpkg)->highest_version)
	  solver->set_constraint_coeff((*vers_pkg)->rank, -(size - 1));
	else
	  solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, -size);
      solver->add_constraint_geq(-size+1);
      ivpkg_rank++;
    }
  }
  return 0;
}

// Compute the criteria range
CUDFcoefficient notuptodate_criteria::bound_range() { return CUDFabs(lambda_crit) * ub; }

// Compute the criteria upper bound
CUDFcoefficient notuptodate_criteria::upper_bound() { 
  if (lambda_crit >= 0)
    return lambda_crit * ub; 
  else
    return 0;
}

// Compute the criteria lower bound
CUDFcoefficient notuptodate_criteria::lower_bound() { 
  if (lambda_crit >= 0)
    return 0;
  else
    return lambda_crit * ub; 
}


