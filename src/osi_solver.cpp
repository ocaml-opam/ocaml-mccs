
/*******************************************************/
/* CUDF solver: osi_solver.c                           */
/* Interface to OSI solvers                            */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <math.h>
#include <coin/OsiClpSolverInterface.hpp>
#include <coin/CoinPackedVector.hpp>
#include <osi_solver.h>
#include <limits.h>

#define OUTPUT_MODEL 1

// external function for solver creation
abstract_solver *new_osi_solver(bool use_exact) { return new osi_solver(use_exact); }

int setIndex(CoinPackedVector &v, int rank, CUDFcoefficient value) {
  try { v.insert (rank, value); } catch (...) {}
  return 0;
}

int osi_solver::setCoef(int rank, CUDFcoefficient value) {
  return setIndex(coefficients, rank, value);
}

// solver initialisation
int osi_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {
  int i = 0;
  nb_packages = all_versioned_packages->size();
  nb_vars = nb_packages + other_vars;

  if (verbosity > 0) CoinError::printErrors_ = true;

  // Coefficient initialization
  // initialize_coeffs(nb_packages + other_vars);

  this->all_versioned_packages = all_versioned_packages;

  solver = new OsiClpSolverInterface;

  matrix = new CoinPackedMatrix(false,0,0);
  matrix->setDimensions(0, nb_vars);

  return 0;
}

// Does the solver provides integer variables
bool osi_solver::has_intvars() { return true; }

// Set range of an integer variable
int osi_solver::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) {
  col_lb.insert(rank, lower);
  col_ub.insert(rank, upper);
  return 0;
}

// initialize objective function
int osi_solver::begin_objectives(void) {
  solver->setObjSense(1); // Problem is a minimization
  return 0;
}

// return the package coefficient of the objective function
CUDFcoefficient osi_solver::get_obj_coeff(CUDFVersionedPackage *package) {
  return (CUDFcoefficient)coefficients[package->rank];
}

// return the package coefficient of the objective function 
CUDFcoefficient osi_solver::get_obj_coeff(int rank) {
  return (CUDFcoefficient)coefficients[rank];
}

// set package coefficient to a value
int osi_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  return setCoef(package->rank, value);
}
// set column coefficient to a value
int osi_solver::set_obj_coeff(int rank, CUDFcoefficient value) {
  return setCoef(rank, value);
}

// initialize an additional objective function 
int osi_solver::new_objective(void) {
  coefficients.clear();
  return 0;
}

// add an additional objective function
int osi_solver::add_objective(void) {
  CoinPackedVector obj = new CoinPackedVector(coefficients);
  obj = coefficients; // copy constructor doesn't work !?
  objectives.push_back (obj);
  return 0;
}

// finalize the objective function
int osi_solver::end_objectives(void) {
  return 0;
}

// write the problem into a file
// int osi_solver::writelp(char *filename) { glp_write_lp(lp, NULL, filename); return 0; }

void osi_solver::abort(void) {
  this->aborted = true;
  // this->mip_params.tm_lim = 0; TODO
  return;
}

// solve the current lp problem
int osi_solver::solve(int timeout) {
  int nb_objectives = objectives.size();
  int nb_rows = matrix->getNumRows();
  int save_stdout = 1;

  try {
  if (verbosity == 0) {
    save_stdout = dup(1);
    close(1);
  }

  double * col_lb_v = col_lb.denseVector(nb_vars);
  double * col_ub_v = col_ub.denseVector(nb_vars);
  double * obj_v = objectives[0].denseVector(nb_vars);
  double * row_lb_v = row_lb.denseVector(nb_rows);
  double * row_ub_v = row_ub.denseVector(nb_rows);
  solver->assignProblem(matrix, col_lb_v, col_ub_v, obj_v, row_lb_v, row_ub_v);

  int i = 0;
  for (CUDFVersionedPackageListIterator ipkg = all_versioned_packages->begin();
       ipkg != all_versioned_packages->end();
       ipkg++, i++) {
    solver->setInteger(i);
    solver->setColBounds(i, 0., 1.);
    solver->setColName(i, (*ipkg)->versioned_name);
  }
  for (i = nb_packages; i < nb_vars; i++) {
    char *name;
    char buffer[20];

    sprintf(buffer, "x%d", i+1);
    if ((name = (char *)malloc(strlen(buffer)+1)) == (char *)NULL) {
      PRINT_ERR("CUDF error: can not alloc memory for variable name in osi_solver::solve.\n");
      exit(-1);
    }
    strcpy(name, buffer);

    solver->setInteger(i);
    solver->setColBounds(i, 0., 1.);
    solver->setColName(i, name);
  }

  //solver->writeLp("coinpbs");

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

int osi_solver::solve() {
  return (this->solve(INT_MAX));
}

// get objective function value
CUDFcoefficient osi_solver::objective_value() {
  return (CUDFcoefficient)CUDFnearbyint(solver->getObjValue());
}

// solution initialisation
int osi_solver::init_solutions() {
  solution = solver->getColSolution();
  return 0;
}

// return the status of a package within the final configuration
CUDFcoefficient osi_solver::get_solution(CUDFVersionedPackage *package) {
  return (CUDFcoefficient)CUDFnearbyint(solution[package->rank]);
}

// initialize constraints
int osi_solver::begin_add_constraints(void) {
  return 0;
}

// begin a new constraint
int osi_solver::new_constraint(void) {
  coefficients.clear();
  return 0;
}

// get the package coefficient of the current constraint
CUDFcoefficient osi_solver::get_constraint_coeff(CUDFVersionedPackage *package) {
  return (CUDFcoefficient)coefficients[package->rank];
}

// get the package coefficient of the current constraint
CUDFcoefficient osi_solver::get_constraint_coeff(int rank) {
  return (CUDFcoefficient)coefficients[rank];
}

// set package coefficient of the current constraint
int osi_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  return setCoef(package->rank, value);
}

// set column coefficient of the current constraint
int osi_solver::set_constraint_coeff(int rank, CUDFcoefficient value) {
  return setCoef(rank, value);
}

// add current constraint as a greater or equal constraint
int osi_solver::add_constraint_geq(CUDFcoefficient bound) {
  if (coefficients.getNumElements() > 0 ) {

    int i = matrix->getNumRows();
    matrix->appendRow(coefficients);
    row_lb.insert (i, bound);
    row_ub.insert (i, solver->getInfinity());
  }
  return 0;
}

// add current constraint as a less or equal constraint
int osi_solver::add_constraint_leq(CUDFcoefficient bound) {
  if (coefficients.getNumElements() > 0 ) {
    int i = matrix->getNumRows();
    matrix->appendRow(coefficients);
    row_lb.insert (i, -solver->getInfinity());
    row_ub.insert (i, bound);
  }
  return 0;
}

// add current constraint as an equality constraint
int osi_solver::add_constraint_eq(CUDFcoefficient bound) {
  if (coefficients.getNumElements() > 0 ) {
    int i = matrix->getNumRows();
    matrix->appendRow(coefficients);
    row_lb.insert (i, bound);
    row_ub.insert (i, bound);
  }
  return 0;
}

// finalize constraints
int osi_solver::end_add_constraints(void) {
  // if (OUTPUT_MODEL) glp_write_lp(lp, NULL, "osipbs.lp");
  return 0;
}

osi_solver::~osi_solver() {
  if (solver != NULL) delete solver;
}
