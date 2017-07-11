
/*******************************************************/
/* CUDF solver: notuptodate_criteria.h                 */
/* Concrete class for the notuptodate criteria         */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifndef _NOTUPTODATE_CRITERIA_H_
#define _NOTUPTODATE_CRITERIA_H_

#include <abstract_criteria.h>

// A concrete class for the notuptodate criteria
// i.e. number of virtual packages that are not installed
// with their highest available version
class notuptodate_criteria: public abstract_criteria {
 public:
  CUDFproblem *problem;      // a pointer to the problem
  abstract_solver *solver;   // a pointer to the solver

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

  // upper bound of the criteria
  CUDFcoefficient ub;

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
  notuptodate_criteria() { this->lambda_crit = +1; };
  notuptodate_criteria(CUDFcoefficient lambda_crit) { this->lambda_crit = lambda_crit; };
};

#endif


