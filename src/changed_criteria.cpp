
/*******************************************************/
/* CUDF solver: changed_criteria.c                     */
/* Implementation of the changed criteria              */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <changed_criteria.h>

// Criteria initialization
void changed_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->problem = problem;
  this->solver = solver;
  range = ub = lb  = 0;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); ivpkg != problem->all_virtual_packages->end(); ivpkg++) {
    int size = (*ivpkg)->all_versions.size();
    if (size > 0) {
      all_versioned_virtual_packages.push_back((*ivpkg));
      if (size == 1) {
	CUDFVersionedPackage *pkg = *((*ivpkg)->all_versions.begin());
	if (pkg->installed) {
	  if (criteria_opt_var) lb--; else range++;
	} else
	  ub++;
      } else
	range++;
    }
  }
}

// Computing the number of columns required to handle the criteria
int changed_criteria::set_variable_range(int first_free_var) {
  this->first_free_var = first_free_var;
  return first_free_var + range;
}

// Add the criteria to the current objective function
int changed_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = all_versioned_virtual_packages.begin(); 
       ivpkg != all_versioned_virtual_packages.end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() == 1) {
      CUDFVersionedPackage *pkg = *((*ivpkg)->all_versions.begin());
      if (pkg->installed) {
	if (criteria_opt_var) 
	  solver->set_obj_coeff(pkg, - lambda_crit * lambda + solver->get_obj_coeff(pkg));
	else
	  solver->set_obj_coeff(ivpkg_rank++, lambda_crit * lambda);
      } else
	solver->set_obj_coeff(pkg, lambda_crit * lambda + solver->get_obj_coeff(pkg));
    } else
      solver->set_obj_coeff(ivpkg_rank++, lambda_crit * lambda);
  return 0;
}

// Add the criteria to the constraint set
int changed_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = all_versioned_virtual_packages.begin(); 
       ivpkg != all_versioned_virtual_packages.end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() == 1) {
      CUDFVersionedPackage *pkg = *((*ivpkg)->all_versions.begin());
      if (pkg->installed) {
	if (criteria_opt_var) 
	  solver->set_constraint_coeff(pkg, - lambda_crit * lambda + solver->get_obj_coeff(pkg));
	else
	  solver->set_constraint_coeff(ivpkg_rank++, lambda_crit * lambda);
      } else
	solver->set_constraint_coeff(pkg, lambda_crit * lambda + solver->get_constraint_coeff(pkg));
    } else
      solver->set_constraint_coeff(ivpkg_rank++, lambda_crit * lambda);
  return 0;
}

// Add the constraints required by the criteria
int changed_criteria::add_constraints() {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = all_versioned_virtual_packages.begin(); 
       ivpkg != all_versioned_virtual_packages.end(); ivpkg++) {
    int m = 0, n = (*ivpkg)->all_versions.size();
    solver->new_constraint();
    if (n == 1) {
      if (! criteria_opt_var) {
	CUDFVersionedPackage *pkg = *((*ivpkg)->all_versions.begin());
	if (pkg->installed) {
	  solver->set_constraint_coeff(pkg->rank, +1);
	  solver->set_constraint_coeff(ivpkg_rank++, +1);
	  solver->add_constraint_eq(1);
	}
      }
    } else {
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	if ((*vers_pkg)->installed) {
	  solver->set_constraint_coeff((*vers_pkg)->rank, -1);
	  m++;
	} else
	  solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, -1);
      solver->add_constraint_geq(-m);
      solver->new_constraint();
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	if ((*vers_pkg)->installed)
	  solver->set_constraint_coeff((*vers_pkg)->rank, -1);
	else
	  solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, -n);
      solver->add_constraint_leq(-m);
      ivpkg_rank++;
    }
  }
  return 0;
}

// Compute the criteria range
CUDFcoefficient changed_criteria::bound_range() { return CUDFabs(lambda_crit) * (ub - lb + 1); }

// Compute the criteria upper bound
CUDFcoefficient changed_criteria::upper_bound() { 
  if (lambda_crit >= 0) 
    return lambda_crit * ub; 
  else
    return lambda_crit * lb; 
}

// Compute the criteria lower bound
CUDFcoefficient changed_criteria::lower_bound() { 
  if (lambda_crit >= 0) 
    return lambda_crit * lb; 
  else
    return lambda_crit * ub; 
}


