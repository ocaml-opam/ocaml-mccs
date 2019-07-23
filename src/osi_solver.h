
/*******************************************************/
/* CUDF solver: osi_solver.h                           */
/* Concrete class for OSI solvers                      */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// concrete class which implements an interface to COIN-OR Open Solver Interface
// compliant solvers
#ifdef USECOIN

#ifndef _OSI_SOLVER_H
#define _OSI_SOLVER_H

#include <abstract_solver.h>
#include <coin/OsiSolverInterface.hpp>
#include <coin/CoinPackedVector.hpp>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <limits.h>

template<class OsiSolver>
class osi_solver: public abstract_solver  {
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

  OsiSolverInterface * solver;
  CUDFVersionedPackageList *all_versioned_packages;  // list of all versioned packages
  int nb_packages; // number of packages


  // solver creation
  osi_solver() {
    solver = (OsiSolverInterface *)NULL;
    all_versioned_packages = (CUDFVersionedPackageList *)NULL;
    aborted = false;
    solution = (const double *)NULL;
    matrix = (CoinPackedMatrix *)NULL;
    col_lb = col_ub = (double *)NULL;
  }

  ~osi_solver();

  int nb_vars;
  int nb_coeffs;
  vector<CoinPackedVector> objectives;

 private:
  bool aborted;
  const double *solution;
  double * col_lb, * col_ub;
  vector<double> row_lb, row_ub;
  CoinPackedMatrix *matrix;
  CoinPackedVector coefficients; // intermediate line coefficients storage
                                 // (constraint being defined, or current
                                 // objective)

  int setCoef(int rank, CUDFcoefficient value);
};

// external function for solver creation
template<class OsiSolver>
abstract_solver *new_osi_solver() {
  return new osi_solver<OsiSolver>();
}

template<class OsiSolver>
int osi_solver<OsiSolver>::setCoef(int rank, CUDFcoefficient value) {
  try { coefficients.insert (rank, value); } catch (...) {}
  return 0;
}

// solver initialisation
template<class OsiSolver>
int osi_solver<OsiSolver>::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {
  int i = 0;
  nb_packages = all_versioned_packages->size();
  nb_vars = nb_packages + other_vars;

  if (verbosity > 0) CoinError::printErrors_ = true;

  // Coefficient initialization
  // initialize_coeffs(nb_packages + other_vars);

  col_lb = (double *)calloc(nb_vars, sizeof(double));
  col_ub = (double *)calloc(nb_vars, sizeof(double));

  this->all_versioned_packages = all_versioned_packages;

  solver = new OsiSolver;

  matrix = new CoinPackedMatrix(false,100.,0);
  matrix->setDimensions(0, nb_vars);

  return 0;
}

// Does the solver provides integer variables
template<class OsiSolver>
bool osi_solver<OsiSolver>::has_intvars() { return true; }

// Set range of an integer variable
template<class OsiSolver>
int osi_solver<OsiSolver>::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) {
  col_lb[rank] = lower;
  col_ub[rank] = upper;
  return 0;
}

// initialize objective function
template<class OsiSolver>
int osi_solver<OsiSolver>::begin_objectives(void) {
  solver->setObjSense(1); // Problem is a minimization
  return 0;
}

// return the package coefficient of the objective function
template<class OsiSolver>
CUDFcoefficient osi_solver<OsiSolver>::get_obj_coeff(CUDFVersionedPackage *package) {
  return (CUDFcoefficient)coefficients[package->rank];
}

// return the package coefficient of the objective function 
template<class OsiSolver>
CUDFcoefficient osi_solver<OsiSolver>::get_obj_coeff(int rank) {
  return (CUDFcoefficient)coefficients[rank];
}

// set package coefficient to a value
template<class OsiSolver>
int osi_solver<OsiSolver>::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  return setCoef(package->rank, value);
}
// set column coefficient to a value
template<class OsiSolver>
int osi_solver<OsiSolver>::set_obj_coeff(int rank, CUDFcoefficient value) {
  return setCoef(rank, value);
}

// initialize an additional objective function 
template<class OsiSolver>
int osi_solver<OsiSolver>::new_objective(void) {
  coefficients.clear();
  return 0;
}

// add an additional objective function
template<class OsiSolver>
int osi_solver<OsiSolver>::add_objective(void) {
  objectives.push_back(coefficients);
  return 0;
}

// finalize the objective function
template<class OsiSolver>
int osi_solver<OsiSolver>::end_objectives(void) {
  return 0;
}

// write the problem into a file
// int osi_solver<OsiSolver>::writelp(const char *filename) { glp_write_lp(lp, NULL, filename); return 0; }

template<class OsiSolver>
void osi_solver<OsiSolver>::abort(void) {
  this->aborted = true;
  // this->mip_params.tm_lim = 0; TODO
  return;
}

// solve the current lp problem
template<class OsiSolver>
int osi_solver<OsiSolver>::solve(int timeout) {
  int nb_objectives = objectives.size();
  int nb_rows = matrix->getNumRows();
  int save_stdout = 1;

  try {
  if (verbosity == 0) {
    save_stdout = dup(1);
    close(1);
  }

  double * obj_v = objectives[0].denseVector(nb_vars);
  double * row_lb_v = (double *)malloc(nb_rows * sizeof(double));
  double * row_ub_v = (double *)malloc(nb_rows * sizeof(double));
  std::copy(row_lb.begin(), row_lb.end(), row_lb_v);
  std::copy(row_ub.begin(), row_ub.end(), row_ub_v);
  solver->assignProblem(matrix, col_lb, col_ub, obj_v, row_lb_v, row_ub_v);

  int i = 0;
  for (CUDFVersionedPackageListIterator ipkg = all_versioned_packages->begin();
       ipkg != all_versioned_packages->end();
       ipkg++, i++) {
    if (verbosity > 0) solver->setColName(i, (*ipkg)->versioned_name);
    solver->setInteger(i);
    solver->setColBounds(i, 0., 1.);
  }
  for (i = nb_packages; i < nb_vars; i++) {
    if (verbosity > 0) {
      char *name;
      char buffer[20];

      if ((name = (char *)malloc(strlen(buffer)+1)) == (char *)NULL) {
        PRINT_ERR("CUDF error: can not alloc memory for variable name in osi_solver::solve.\n");
        exit(-1);
      }
      strcpy(name, buffer);
      solver->setColName(i, name);
    }
    solver->setInteger(i);
    solver->setColBounds(i, 0., 1.);
  }

  solver->writeLp("coinpbs");

  this->aborted = false;

  solver->initialSolve();

  for (int k = 1; k < nb_objectives && solver->isProvenOptimal(); k++) {

    CUDFcoefficient objval = objective_value();

    if (verbosity > 0) PRINT_OUT(">>> Objective %d value : %" CUDFint64"d\n", k, objval);

    // Set objective k+1 as the actual objective function
    double *obj_v = objectives[k+1].denseVector(nb_vars);
    solver->setObjective(obj_v);

    // Add objective k = objval constraint
    solver->addRow(objectives[k], objval, objval);

    solver->resolve();

  }
  } catch (...) {
    if (verbosity == 0) {
      dup2(save_stdout, 1);
      close(save_stdout);
    }
    throw;
  }
  if (verbosity == 0) {
    dup2(save_stdout, 1);
    close(save_stdout);
  }

  if (solver->isProvenOptimal()) {
    return 1;
  } else if (solver->isProvenPrimalInfeasible() ||
             solver->isProvenDualInfeasible()) {
    return 0;
  } else if (solver->isIterationLimitReached()) {
    if (this->aborted)
      return -3;
    else
      return -2;
  } else {
    return -1;
  }
}

template<class OsiSolver>
int osi_solver<OsiSolver>::solve() {
  return (this->solve(INT_MAX));
}

// get objective function value
template<class OsiSolver>
CUDFcoefficient osi_solver<OsiSolver>::objective_value() {
  return (CUDFcoefficient)CUDFnearbyint(solver->getObjValue());
}

// solution initialisation
template<class OsiSolver>
int osi_solver<OsiSolver>::init_solutions() {
  solution = solver->getColSolution();
  return 0;
}

// return the status of a package within the final configuration
template<class OsiSolver>
CUDFcoefficient osi_solver<OsiSolver>::get_solution(CUDFVersionedPackage *package) {
  return (CUDFcoefficient)CUDFnearbyint(solution[package->rank]);
}

// initialize constraints
template<class OsiSolver>
int osi_solver<OsiSolver>::begin_add_constraints(void) {
  return 0;
}

// begin a new constraint
template<class OsiSolver>
int osi_solver<OsiSolver>::new_constraint(void) {
  coefficients.clear();
  return 0;
}

// get the package coefficient of the current constraint
template<class OsiSolver>
CUDFcoefficient osi_solver<OsiSolver>::get_constraint_coeff(CUDFVersionedPackage *package) {
  return (CUDFcoefficient)coefficients[package->rank];
}

// get the package coefficient of the current constraint
template<class OsiSolver>
CUDFcoefficient osi_solver<OsiSolver>::get_constraint_coeff(int rank) {
  return (CUDFcoefficient)coefficients[rank];
}

// set package coefficient of the current constraint
template<class OsiSolver>
int osi_solver<OsiSolver>::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  return setCoef(package->rank, value);
}

// set column coefficient of the current constraint
template<class OsiSolver>
int osi_solver<OsiSolver>::set_constraint_coeff(int rank, CUDFcoefficient value) {
  return setCoef(rank, value);
}

// add current constraint as a greater or equal constraint
template<class OsiSolver>
int osi_solver<OsiSolver>::add_constraint_geq(CUDFcoefficient bound) {
  if (coefficients.getNumElements() > 0 ) {
    matrix->appendRow(coefficients);
    row_lb.push_back(bound);
    row_ub.push_back(solver->getInfinity());
  }
  return 0;
}

// add current constraint as a less or equal constraint
template<class OsiSolver>
int osi_solver<OsiSolver>::add_constraint_leq(CUDFcoefficient bound) {
  if (coefficients.getNumElements() > 0 ) {
    matrix->appendRow(coefficients);
    row_lb.push_back(-solver->getInfinity());
    row_ub.push_back(bound);
  }
  return 0;
}

// add current constraint as an equality constraint
template<class OsiSolver>
int osi_solver<OsiSolver>::add_constraint_eq(CUDFcoefficient bound) {
  if (coefficients.getNumElements() > 0 ) {
    int i = matrix->getNumRows();
    matrix->appendRow(coefficients);
    row_lb.push_back(bound);
    row_ub.push_back(bound);
  }
  return 0;
}

// finalize constraints
template<class OsiSolver>
int osi_solver<OsiSolver>::end_add_constraints(void) {
  // if (OUTPUT_MODEL) glp_write_lp(lp, NULL, "osipbs.lp");
  return 0;
}

template<class OsiSolver>
osi_solver<OsiSolver>::~osi_solver() {
  if (solver != NULL) delete solver;
}

#endif
#endif // USECOIN
