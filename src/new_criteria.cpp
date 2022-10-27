
/*******************************************************/
/* CUDF solver: new_criteria.h                         */
/* Implementation the new criteria                     */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include "new_criteria.h"

// Criteria initialization
void new_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->problem = problem;
  this->solver = solver;
  range = 0;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); ivpkg != problem->all_virtual_packages->end(); ivpkg++) {
    int size = (*ivpkg)->all_versions.size();
    if ((size > 0) && ((*ivpkg)->highest_installed == (CUDFVersionedPackage *)NULL)) {
      all_uninstalled_versioned_virtual_packages.push_back((*ivpkg));
      if (size > 1) range++;
    }
  }
}

// Computing the number of columns required to handle the criteria
int new_criteria::set_variable_range(int first_free_var) {
  this->first_free_var = first_free_var;
  return first_free_var + range;
}

// Add the criteria to the current objective function
int new_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = all_uninstalled_versioned_virtual_packages.begin(); 
       ivpkg != all_uninstalled_versioned_virtual_packages.end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() == 1) {
      CUDFVersionedPackage *pkg = *((*ivpkg)->all_versions.begin());
      solver->set_obj_coeff(pkg, lambda_crit * lambda + solver->get_obj_coeff(pkg));
    } else
      solver->set_obj_coeff(ivpkg_rank++, lambda_crit * lambda);
  return 0;
}

// Add the criteria to the constraint set
int new_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = all_uninstalled_versioned_virtual_packages.begin(); 
       ivpkg != all_uninstalled_versioned_virtual_packages.end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() == 1) 
      solver->set_constraint_coeff(*((*ivpkg)->all_versions.begin()), lambda_crit * lambda);
    else
      solver->set_constraint_coeff(ivpkg_rank++, lambda_crit * lambda);
  return 0;
}

// Add the constraints required by the criteria
int new_criteria::add_constraints() {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = all_uninstalled_versioned_virtual_packages.begin(); 
       ivpkg != all_uninstalled_versioned_virtual_packages.end(); ivpkg++) {
    solver->new_constraint();
    if ((*ivpkg)->all_versions.size() > 1) {
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, -1);
      solver->add_constraint_geq(0);
      solver->new_constraint();
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      { 
	int n = (*ivpkg)->all_versions.size();
	solver->set_constraint_coeff(ivpkg_rank, -n);
	solver->add_constraint_leq(0);
      }
      ivpkg_rank++;
    }
  }
  return 0;
}

// Compute the criteria range
CUDFcoefficient new_criteria::bound_range() { return CUDFabs(lambda_crit) * all_uninstalled_versioned_virtual_packages.size() + 1; }

// Compute the criteria upper bound
CUDFcoefficient new_criteria::upper_bound() { 
  if (lambda_crit >= 0)
    return lambda_crit * all_uninstalled_versioned_virtual_packages.size(); 
  else
    return 0;
}

// Compute the criteria lower bound
CUDFcoefficient new_criteria::lower_bound() { 
  if (lambda_crit >= 0)
    return 0;
  else
    return lambda_crit * all_uninstalled_versioned_virtual_packages.size(); 
}


