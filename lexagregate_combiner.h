
/*******************************************************/
/* CUDF solver: lexagregate_combiner.h                 */
/* a concrete class for a lexicographic order agregate */
/* combiner                                            */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifndef __LEXAGREGATE_COMBINER_H
#define __LEXAGREGATE_COMBINER_H

#include <abstract_combiner.h>

// Concrete class to agregate a set of criteria i.e. handle it as sum \lambda_i c_i
// Note: such an agregate could be seen either as a combiner or a criteria
class lexagregate_combiner: public abstract_combiner, public abstract_criteria {
 public:
  CriteriaList *criteria;   // set of criteria
  abstract_solver *solver;  // used solver

  // ********************************************************
  // Seen as a combiner

  int column_allocation(int first_rank);

  int objective_generation();

  int constraint_generation();

  // ********************************************************
  // Seen as a criteria

  int set_variable_range(int first_free_var);
  int add_criteria_to_objective(CUDFcoefficient lambda);
  int add_criteria_to_constraint(CUDFcoefficient lambda);
  int add_constraints();

  // computing combiner/criteria ranges/bounds
  CUDFcoefficient bound_range();
  CUDFcoefficient upper_bound();
  CUDFcoefficient lower_bound();

  // does this combiner/criteria allows problem reduction
  bool can_reduce();
  bool can_reduce(CUDFcoefficient lambda);

  // initialization
  void initialize(CUDFproblem *problem, abstract_solver *solver);
  void initialize_intvars();

  // lambda coefficient for the current combiner/criteria
  CUDFcoefficient lambda_crit ;

  // lexagregate combiner creation
  lexagregate_combiner(CriteriaList *criteria) { this->lambda_crit = 1; this->criteria = criteria; };
  lexagregate_combiner(CriteriaList *criteria, CUDFcoefficient lambda_crit) { 
    this->criteria = criteria;
    this->lambda_crit = lambda_crit; 
  };
};

#endif
