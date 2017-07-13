
/*******************************************************/
/* CUDF solver: abstract_criteria.h                    */
/* Abstract class for criteria                         */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/



#ifndef _ABSTRACT_CRITERIA_H_
#define _ABSTRACT_CRITERIA_H_

#include <cudf.h>
#include <cudf_types.h>
#include <abstract_solver.h>

// Abstract criteria class
class abstract_criteria {
 public:
  // Method called to allocate some variables (columns) to the criteria
  virtual int set_variable_range(int first_free_var) { return 0; }
  // Method called to add the criteria to the current objective
  virtual int add_criteria_to_objective(CUDFcoefficient lambda) { return 0; };
  // Method called to add the criteria to the constraints
  virtual int add_criteria_to_constraint(CUDFcoefficient lambda) { return 0; };
  // Method called to add criteria related constraints
  virtual int add_constraints() { return 0; };

  // Gives the range of the criteria objective 
  virtual CUDFcoefficient bound_range() { return 0; };
  // Gives the upper bound of the criteria objective
  virtual CUDFcoefficient upper_bound() { return 0; };
  // Gives the lower bound of the criteria objective
  virtual CUDFcoefficient lower_bound() { return 0; };

  // Does this criteria allows problem reduction ?
  virtual bool can_reduce(CUDFcoefficient lambda) { return true; }

  // Method called to let the criteria initializes itself
  virtual void initialize(CUDFproblem *problem, abstract_solver *solver) { };
  // Method called to initialize criteria variables
  virtual void initialize_intvars() { };

  // Method called to let the criteria checks some properties availability
  virtual void check_property(CUDFproblem *problem) { };

  // Criteria destructor
  virtual ~abstract_criteria() { };
};

// Type for a list of criteria
typedef vector<abstract_criteria *> CriteriaList;
typedef CriteriaList::iterator CriteriaListIterator;

// Shall we optimize variable usage or not
extern bool criteria_opt_var;

#endif

