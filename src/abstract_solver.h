
/*******************************************************/
/* CUDF solver: abstract_solver.h                      */
/* Abstract class for solvers                          */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// Solver abstraction
// all implemented solvers are implemented through a concrete class that inherits and implements an abstract solver.

#ifndef _ABSTRACT_SOLVER_H
#define _ABSTRACT_SOLVER_H

#include <cudf.h>
#include <cudf_types.h>


// provide an abstraction of the underlying solvers
class abstract_solver {
 public:
  // ******************************************************************
  // solver initialisation method (called at the beginning of constraint generation)
  virtual int init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) { return 0; };

  // ******************************************************************
  // does the solver has the capability to handle integer variables
  virtual bool has_intvars() { return false; };

  // set variable type to int and its range to [lower, upper] (must be used before end_objectives)
  virtual int set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) { return 0; }

  // ******************************************************************
  // called before objective definitions
  virtual int begin_objectives(void) { return 0; };

  // get the current coefficient value of the objective function for parameter package
  virtual CUDFcoefficient get_obj_coeff(CUDFVersionedPackage *package) { return 0; };
  // get the current coefficient value of the objective function for variable rank
  virtual CUDFcoefficient get_obj_coeff(int rank) { return 0; };

  // set objective coefficient value according to a package
  virtual int set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { return 0; };
  // set objective coefficient value according to a rank (column number)
  virtual int set_obj_coeff(int rank, CUDFcoefficient value) { return 0; };

  // called before defining a new objective
  virtual int new_objective(void) { return 0; };

  // called to add a new objective
  virtual int add_objective(void) { return 0; };

  // called at the end of objective definitions
  virtual int end_objectives(void) { return 0; };

  // ******************************************************************
  // constraint generation

  // called before the definition of any constraint
  virtual int begin_add_constraints(void) { return 0; };

  // called before the definition of a new constraint
  virtual int new_constraint(void) { return 0; };

  // get current constraint coefficient according to a package
  virtual CUDFcoefficient get_constraint_coeff(CUDFVersionedPackage *package) { return 0; };
  // get current constraint coefficient according to a rank
  virtual CUDFcoefficient get_constraint_coeff(int rank) { return 0; };

  // set constraint coefficient of a package
  virtual int set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { return 0; };
  // set constraint coefficient of a rank (i.e. column number)
  virtual int set_constraint_coeff(int rank, CUDFcoefficient value) { return 0; };

  // define the constraint as greater or equal to a bound (called once all constraints coefficients have been defined)
  virtual int add_constraint_geq(CUDFcoefficient bound) { return 0; };
  // define the constraint as less or equal to a bound (called once all constraints coefficients have been defined)
  virtual int add_constraint_leq(CUDFcoefficient bound) { return 0; };
  // define the constraint as equal to a bound (called once all constraints coefficients have been defined)
  virtual int add_constraint_eq(CUDFcoefficient bound) { return 0; };

  // called once all constraints have been defined
  virtual int end_add_constraints(void) { return 0; };


  // ******************************************************************
  // write the internal representation of the problem to a file
  // virtual int writelp(const char *filename) { return 0; };

  // ******************************************************************
  // solve the problem (must return a value > 0 if a solution has been found, 0
  // if unsatisfiable, < 0 on error. -2 means timeout, -3 means sigint)
  virtual int solve() { return 0; }; 
  virtual int solve(int timeout) { return 0; };

  // ******************************************************************
  // abort should, if possible, do something which causes solve to
  // terminate.
  virtual void abort() = 0;

  // ******************************************************************
  // initialisation of the solutions (called before reading them)
  virtual int init_solutions() { return 0; };

  // get the objective value at the end of solving
  virtual CUDFcoefficient objective_value() { return 0; };

  // get the status of a package in the final configuration
  virtual CUDFcoefficient get_solution(CUDFVersionedPackage *package) { return 0; };
  virtual CUDFcoefficient get_solution(int k) { return 0; };

  // ******************************************************************
  // abstract solver destructor
  virtual ~abstract_solver() {};
  
};


#endif
