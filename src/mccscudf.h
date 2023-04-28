
#ifndef _MCCSCUDF_H
#define _MCCSCUDF_H

typedef enum {CPLEX, GUROBI, LPSOLVE, GLPK, LP, CLP, CBC, SYMPHONY} Solver_backend;

typedef struct {
  Solver_backend backend;
  const char * lp_solver;
} Solver;

typedef struct {
  int success; // 1 on success (SAT or UNSAT), 0 on error, -1 on timeout
  const char * error; // filled when success == 0
  CUDFproblem * problem; // might be the original problem, or a copy
  abstract_solver * solution; // May be NULL with success == 1 for unsatisfiable problems
} Solver_return;

Solver_return call_mccs(Solver solver_arg, char *criteria_arg, int timeout, double mip_gap, CUDFproblem* the_problem);
Solver_return call_mccs(Solver solver_arg, char *criteria_arg, int timeout, double mip_gap, CUDFproblem* the_problem, abstract_solver **solver);

int has_backend(Solver_backend backend);

#endif
