
/*******************************************************/
/* CUDF solver: removed_criteria.c                     */
/* Implementation of the removed criteria              */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <removed_criteria.h>
#include <constraint_generation.h>

// Criteria initialization
void removed_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->problem = problem;
  this->solver = solver;
  ub = lb = 0;
  for (CUDFVirtualPackageListIterator ivpkg = problem->all_virtual_packages->begin(); ivpkg != problem->all_virtual_packages->end(); ivpkg++) 
    if ((*ivpkg)->highest_installed != (CUDFVersionedPackage *)NULL) {
      installed_virtual_packages.push_back(*ivpkg);
      if (criteria_opt_var)
	if ((*ivpkg)->all_versions.size() > 1) ub++; else lb--;
      else
	ub++;
    }
}

// Computing the number of columns required to handle the criteria
int removed_criteria::set_variable_range(int first_free_var) {
  this->first_free_var = first_free_var;
  return first_free_var + ub;
}

// Add the criteria to the current objective function
int removed_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = installed_virtual_packages.begin(); 
       ivpkg != installed_virtual_packages.end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() > 1) 
      solver->set_obj_coeff(ivpkg_rank++, lambda * lambda_crit);
    else if (criteria_opt_var) {
      CUDFVersionedPackage *package = (*((*ivpkg)->all_versions.begin()));
      solver->set_obj_coeff(package, - lambda * lambda_crit + solver->get_obj_coeff(package));
    } else 
      solver->set_obj_coeff(ivpkg_rank++, lambda * lambda_crit);
  return 0;
}

// Add the criteria to the constraint set
int removed_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  int ivpkg_rank = first_free_var;
  for (CUDFVirtualPackageListIterator ivpkg = installed_virtual_packages.begin(); 
       ivpkg != installed_virtual_packages.end(); ivpkg++)
    if ((*ivpkg)->all_versions.size() > 1) 
      solver->set_constraint_coeff(ivpkg_rank++, lambda * lambda_crit);
    else if (criteria_opt_var) {
      CUDFVersionedPackage *package = (*((*ivpkg)->all_versions.begin()));
      solver->set_constraint_coeff(package, - lambda * lambda_crit + solver->get_obj_coeff(package));
    } else 
      solver->set_constraint_coeff(ivpkg_rank++, lambda * lambda_crit);
  return 0;
}

// Add the constraints required by the criteria
int removed_criteria::add_constraints() {
  int ivpkg_rank = first_free_var;

  for (CUDFVirtualPackageListIterator ivpkg = installed_virtual_packages.begin(); 
       ivpkg != installed_virtual_packages.end(); ivpkg++) 
    if ((*ivpkg)->all_versions.size() > 1) {
      solver->new_constraint();
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, +1);
      solver->add_constraint_geq(+1);
      solver->new_constraint();
      for (CUDFVersionedPackageSetIterator vers_pkg = (*ivpkg)->all_versions.begin(); vers_pkg != (*ivpkg)->all_versions.end(); vers_pkg++)
	solver->set_constraint_coeff((*vers_pkg)->rank, +1);
      { 
	int n = (*ivpkg)->all_versions.size();
	solver->set_constraint_coeff(ivpkg_rank, +n);
	solver->add_constraint_leq(+n);
      }
      ivpkg_rank++;
    } else if (! criteria_opt_var) {
      solver->new_constraint();
      solver->set_constraint_coeff((*((*ivpkg)->all_versions.begin()))->rank, +1);
      solver->set_constraint_coeff(ivpkg_rank, +1);
      solver->add_constraint_eq(+1);
      ivpkg_rank++;
    }
  return 0;
}

// Compute the criteria range
CUDFcoefficient removed_criteria::bound_range() { return CUDFabs(lambda_crit) * (ub - lb + 1); }

// Compute the criteria upper bound
CUDFcoefficient removed_criteria::upper_bound() { return lambda_crit * (lambda_crit >= 0 ?ub:lb); }

// Compute the criteria lower bound
CUDFcoefficient removed_criteria::lower_bound() { return lambda_crit * (lambda_crit >= 0 ?lb:ub); }

