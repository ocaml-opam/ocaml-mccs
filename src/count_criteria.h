
/*******************************************************/
/* CUDF solver: count_criteria.h                       */
/* Concrete class for the count criteria               */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// Count a property quantity

#ifndef _COUNT_CRITERIA_H_
#define _COUNT_CRITERIA_H_

#include <abstract_criteria.h>

typedef enum {REQUEST,NEW,CHANGED,SOLUTION} Count_scope;

// A concrete class for the count criteria
// i.e. a criteria that count the value of a property
// of installed virtual packages
class count_criteria: public abstract_criteria {
 public:
  CUDFproblem *problem;      // a pointer to the problem
  abstract_solver *solver;   // a pointer to the solver

  const char *property_name; // name of the property
  bool has_property;         // is the property available ?
  // list of all versioned packages which have the property
  CUDFVersionedPackageList *versioned_pkg_with_property;
  CUDFcoefficient default_value; // default property value

  // lower and upper bound of the criteria
  CUDFcoefficient lb, ub;

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

  // Check property
  void check_property(CUDFproblem *problem);

  // lambda multiplier for the criteria
  CUDFcoefficient lambda_crit ;

  Count_scope scope;

  // Criteria initialization
  count_criteria(const char *property_name) {
    this->property_name = property_name;
    this->lambda_crit = +1; 
  };

  // Criteria initialization
  count_criteria(const char *property_name, Count_scope scope, CUDFcoefficient lambda_crit) {
    this->property_name = property_name;
    this->lambda_crit = lambda_crit; 
    this->scope = scope;
  };
};

#endif


