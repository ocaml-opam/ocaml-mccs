
/*******************************************************/
/* CUDF solver: glpk_solver.h                          */
/* Concrete class for the GLPK solver                  */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// concrete class which implements an interface to GLPK solver

#ifndef _GLPK_SOLVER_H
#define _GLPK_SOLVER_H

#include <abstract_solver.h>
#include <scoeff_solver.h>
#include "config.h"

class glpk_solver: public abstract_solver, public scoeff_solver<double, 1, 1>  {
 public:
  // Solver initialization
  int init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars);

  // Does the solver use integer variables
  bool has_intvars();
  // Allocate some columns for integer variables
  int set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper);

  // Write the lp on a file
  // int writelp(const char *filename);

  // Solve the problem
  int solve();
  // Solve the problem, with timeout
  int solve(int timeout);
  // Terminate the current solver run
  void abort(void);
  // Get the objective value (final one)
  CUDFcoefficient objective_value();
  // Init solutions (required before calling get_solution)
  int init_solutions();
  // Get the solution for a package
  CUDFcoefficient get_solution(CUDFVersionedPackage *package);

  // Init the objective function definitions
  int begin_objectives(void);
  // Get current objective coefficient of package 
  CUDFcoefficient get_obj_coeff(CUDFVersionedPackage *package);
  // Get current objective coefficient of a column
  CUDFcoefficient get_obj_coeff(int rank);
  // Set current objective coefficient of package 
  int set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value);
  // Set current objective coefficient of column
  int set_obj_coeff(int rank, CUDFcoefficient value);
  // Begin the definition of a new objective
  int new_objective(void);
  // Add current objective to the set of objectives
  int add_objective(void);
  // End objective definitions
  int end_objectives(void);

  // Init constraint definitions
  int begin_add_constraints(void);
  // Begin the definition of a new constraint
  int new_constraint(void);
  // Get current constraint coefficient of a package
  CUDFcoefficient get_constraint_coeff(CUDFVersionedPackage *package);
  // Get current constraint coefficient of a column
  CUDFcoefficient get_constraint_coeff(int rank);
  // Set current constraint coefficient of a package
  int set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value);
  // Set current constraint coefficient of a column
  int set_constraint_coeff(int rank, CUDFcoefficient value);
  // Add current constraint as a more or equal constraint
  int add_constraint_geq(CUDFcoefficient bound);
  // Add current constraint as a less or equal constraint
  int add_constraint_leq(CUDFcoefficient bound);
  // Add current constraint as a equality constraint
  int add_constraint_eq(CUDFcoefficient bound);
  // End constraint definitions
  int end_add_constraints(void);

  glp_prob *lp; // internal solver representation
  CUDFVersionedPackageList *all_versioned_packages;  // list of all versioned packages
  int nb_packages; // number of packages

  CUDFcoefficient *lb, *ub;          // arrays of lower and upper bounds

  // solver creation
  glpk_solver(bool use_exact) {
    lp = (glp_prob *)NULL;
    all_versioned_packages = (CUDFVersionedPackageList *)NULL;
    lb = ub = (CUDFcoefficient *)NULL;
    aborted = false;
  }

  ~glpk_solver();

 private:
  glp_iocp mip_params;
  bool aborted;

};

#endif
