
/*******************************************************/
/* CUDF solver: lp_solver.h                            */
/* Concrete class for lp format based solvers          */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// concrete class which implements an interface to a lp (cplex format) compliant solver

#ifndef _LP_SOLVER_H
#define _LP_SOLVER_H

#include "abstract_solver.h"
#include "scoeff_solver.h"

class lp_solver: public abstract_solver, public scoeff_solver<CUDFcoefficient, 0, 0> {
 public:
  // Solver initialization
  int init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars);
  // Write the lp on a file
  int writelp(const char *filename);

  void set_mip_gap(double);

  // Solve the problem
  int solve();
  int solve(int timeout);
  // Get the objective value (final one)
  CUDFcoefficient objective_value();
  // Init solutions (required before calling get_solution)
  int init_solutions();
  // Get the solution for a package
  CUDFcoefficient get_solution(CUDFVersionedPackage *package);

  void abort();

  // Does the solver use integer variables
  bool has_intvars();
  // Allocate some columns for integer variables
  int set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper);

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

  CUDFVersionedPackageList *all_versioned_packages;  // list of all versioned packages
  int nb_packages; // number of packages

  CUDFcoefficient *lb;          // array of lower bounds
  CUDFcoefficient *ub;          // array of upper bounds

  int nb_constraints; // number of constraints

  CUDFcoefficient *solution; // array of solution values
  CUDFcoefficient objval;    // objective value

  char ctlpfilename[256];
  char lpfilename[256];
  char lpoutfilename[256];
  FILE *lpfile, *ctlpfile;

  const char *lpsolver; // name of the solver to call

  char mult;

  // solver creation
  lp_solver(const char *lpsolver) {
    this->lpsolver = lpsolver;
    nb_packages = 0;
    all_versioned_packages = (CUDFVersionedPackageList *)NULL;
    nb_constraints = 0;
    solution = (CUDFcoefficient *)NULL;
    lpfile = stdout;
    mult = ' ';
  }
};

#endif
