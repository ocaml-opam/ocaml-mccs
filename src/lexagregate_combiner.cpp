
/*******************************************************/
/* CUDF solver: lexagregate_combiner.c                 */
/* Implementation of the lexagregate combiner          */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cstdio>
#include "lexagregate_combiner.h"

// Compute the number of columns required to handle the combiner
int lexagregate_combiner::column_allocation(int first_rank) {
  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++)
    first_rank = (*crit)->set_variable_range(first_rank);
  return first_rank;
}

// Generate the objective function
int lexagregate_combiner::objective_generation() {
  // Allow criteria to set the range of their integer variables
  for (CriteriaListIterator icrit = criteria->begin(); icrit != criteria->end(); icrit++) (*icrit)->initialize_intvars();

  solver->new_objective();
  add_criteria_to_objective(1);
  solver->add_objective();
  return 0;
}

// Ask to criteria to generate their own constraints
int lexagregate_combiner::constraint_generation() {
  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++)
    (*crit)->add_constraints();
  return 0;
}

// Combiner initialization
void lexagregate_combiner::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->solver = solver;
  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++) (*crit)->initialize(problem, solver);
}


// Compute the number of required columns when the combiner is used as a criteria
int lexagregate_combiner::set_variable_range(int first_free_var) { 
  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++) 
    first_free_var = (*crit)->set_variable_range(first_free_var);

  return first_free_var;
}

// Add the combiner to the curent objective function
int lexagregate_combiner::add_criteria_to_objective(CUDFcoefficient lambda) { 
  CUDFcoefficient lambda_comb = lambda*lambda_crit;

  for (CriteriaList::reverse_iterator crit = criteria->rbegin(); crit != criteria->rend(); ++crit) {
    (*crit)->add_criteria_to_objective(lambda_comb);
    lambda_comb *= (*crit)->bound_range() + 1;
  }
  return 0;
}

// Add the combiner objective as a constraint
int lexagregate_combiner::add_criteria_to_constraint(CUDFcoefficient lambda) {
  CUDFcoefficient lambda_comb = lambda*lambda_crit;

  for (CriteriaList::reverse_iterator crit = criteria->rbegin(); crit != criteria->rend(); ++crit) {
    (*crit)->add_criteria_to_constraint(lambda_comb);
    lambda_comb *= (*crit)->bound_range() + 1;
  }
  return 0;
}

// Add the required constraints (from the criteria set)
int lexagregate_combiner::add_constraints() { return constraint_generation(); }

// Compute the range of the combiner/criteria
CUDFcoefficient lexagregate_combiner::bound_range() {
  CUDFcoefficient range = 0;
  CUDFcoefficient lambda = +1;

  for (CriteriaList::reverse_iterator crit = criteria->rbegin(); crit != criteria->rend(); ++crit) {
    lambda *= ((*crit)->bound_range() + 1);
    range += CUDFabs(lambda_crit) * lambda;
  }
  return range;
}

// Compute the upper bound of the combiner/criteria
CUDFcoefficient lexagregate_combiner::upper_bound() {
  CUDFcoefficient ub = 0;
  CUDFcoefficient lambda = +1;

  for (CriteriaList::reverse_iterator crit = criteria->rbegin(); crit != criteria->rend(); ++crit) {
    if (lambda_crit >= 0)
      ub += lambda_crit * lambda * (*crit)->upper_bound();
    else
      ub += lambda_crit * lambda * (*crit)->lower_bound();
    lambda *= (*crit)->bound_range() + 1;
  }
  return ub;
}

// Compute the lower bound of the combiner/criteria
CUDFcoefficient lexagregate_combiner::lower_bound() {
  CUDFcoefficient lb = 0;
  CUDFcoefficient lambda = +1;

  for (CriteriaList::reverse_iterator crit = criteria->rbegin(); crit != criteria->rend(); ++crit) {
    if (lambda_crit >= 0)
      lb += lambda_crit * lambda * (*crit)->lower_bound();
    else
      lb += lambda_crit * lambda * (*crit)->upper_bound();
    lambda *= (*crit)->bound_range() + 1;
  }
  return lb;
}

// Does the combiner/criteria allows problem reduction
bool lexagregate_combiner::can_reduce() { 
  bool result = true;

  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++) 
    result = result && (*crit)->can_reduce(lambda_crit);
  return result;
}

// Does the combiner/criteria allows problem reduction (taking into account lambda multiplier)
bool lexagregate_combiner::can_reduce(CUDFcoefficient lambda) { 
  bool result = true;
  CUDFcoefficient l = lambda * lambda_crit;

  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++) 
    result = result && (*crit)->can_reduce(l);
  return result;
}


// Initialize integer variables
void lexagregate_combiner::initialize_intvars() { 
  for (CriteriaListIterator crit = criteria->begin(); crit != criteria->end(); crit++) 
    (*crit)->initialize_intvars();
}
