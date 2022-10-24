
/*******************************************************/
/* CUDF solver: removed_criteria.h                     */
/* Concrete class for the removed criteria             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifndef _REMOVED_CRITERIA_H_
#define _REMOVED_CRITERIA_H_

#include "abstract_criteria.h"

// A concrete criteria class for the removed criteria
// i.e. number of virtual packages that were installed in the initial
// configuration and that are not installed in the final configuration
class removed_criteria: public abstract_criteria {
 public:
  CUDFproblem *problem;      // a pointer to the problem
  abstract_solver *solver;   // a pointer to the solver

  // list of installed virtual packages
  CUDFVirtualPackageList installed_virtual_packages;

  // column of the first variable used by the criteria
  int first_free_var;

  // Allocate some columns for the criteria
  int set_variable_range(int first_free_var);

  // Add the criteria to the objective
  int add_criteria_to_objective(CUDFcoefficient lambda);
  // Add the criteria to the constraint set
  int add_criteria_to_constraint(CUDFcoefficient lambda);
  // Add constraints required by the criteria
  int add_constraints();

  // lower and upper bound of the criteria
  CUDFcoefficient ub, lb;

  // Compute the criteria range, upper and lower bounds
  CUDFcoefficient bound_range();
  CUDFcoefficient upper_bound();
  CUDFcoefficient lower_bound();

  // Does the criteria allows problem reductions
  bool can_reduce(CUDFcoefficient lambda) { return true; }

  // Criteria initialization
  void initialize(CUDFproblem *problem, abstract_solver *solver);

  // lambda multiplier for the criteria
  CUDFcoefficient lambda_crit ;

  // Criteria initialization
  removed_criteria() { this->lambda_crit = +1; };
  removed_criteria(CUDFcoefficient lambda_crit) { this->lambda_crit = lambda_crit; };
};

#endif


