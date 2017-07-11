
/*******************************************************/
/* CUDF solver: new_criteria.h                         */
/* Concrete class for the new criteria                 */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifndef _NEW_CRITERIA_H_
#define _NEW_CRITERIA_H_

#include <abstract_criteria.h>

// A concrete class for the new criteria
// i.e. number of virtual packages that were
// not installed in the initial configuration and
// that are installed in the final one
class new_criteria: public abstract_criteria {
 public:
  CUDFproblem *problem;      // a pointer to the problem
  abstract_solver *solver;   // a pointer to the solver

  // list of all uninstalled virtual packaged with at list a versionned package
  CUDFVirtualPackageList all_uninstalled_versioned_virtual_packages;

  // range of the criteria
  int range;
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

  // Compute the criteria range, upper and lower bounds
  CUDFcoefficient bound_range();
  CUDFcoefficient upper_bound();
  CUDFcoefficient lower_bound();

  // Does the criteria allows problem reductions
  bool can_reduce(CUDFcoefficient lambda) { return ((lambda >= 0) && (lambda_crit >= 0)); }

  // Criteria initialization
  void initialize(CUDFproblem *problem, abstract_solver *solver);

  // lambda multiplier for the criteria
  CUDFcoefficient lambda_crit ;

  // Criteria initialization
  new_criteria() { this->lambda_crit = +1; };
  new_criteria(CUDFcoefficient lambda_crit) { this->lambda_crit = lambda_crit; };
};

#endif


