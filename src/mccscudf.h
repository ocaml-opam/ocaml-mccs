
#ifndef _MCCSCUDF_H
#define _MCCSCUDF_H

typedef enum {CPLEX, GUROBI, LPSOLVE, GLPK} Solver;

typedef struct {
  int success;
  const char * error;
  CUDFproblem * solution;
} Solver_return;

Solver_return call_mccs(Solver solver_arg, char *criteria_arg, CUDFproblem* the_problem, int verbose);

#endif
