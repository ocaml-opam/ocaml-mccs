
/*******************************************************/
/* CUDF solver: glpk_solver.c                          */
/* Interface to the GLPK solver                        */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

#ifdef USEGLPK

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <cmath>
#include "glpk_solver.h"
#include <climits>

#define OUTPUT_MODEL 0

// external function for solver creation
abstract_solver *new_glpk_solver(bool use_exact) { return new glpk_solver(use_exact); }

// solver initialisation
int glpk_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {
  nb_packages = all_versioned_packages->size();

  // Coefficient initialization
  initialize_coeffs(nb_packages + other_vars);

  this->all_versioned_packages = all_versioned_packages;

  lp = glp_create_prob();
  glp_add_cols(lp, nb_vars);

  if ((lb = (CUDFcoefficient *)malloc((nb_vars+1)*sizeof(CUDFcoefficient))) == (CUDFcoefficient *)NULL) {
    PRINT_ERR("glpk_solver: init_solver: not enough memory for lb.\n");
    exit(-1);
  }

  if ((ub = (CUDFcoefficient *)malloc((nb_vars+1)*sizeof(CUDFcoefficient))) == (CUDFcoefficient *)NULL) {
    PRINT_ERR("glpk_solver: init_solver: not enough memory for ub.\n");
    exit(-1);
  }

  for (int i = 0; i <= nb_vars; i++) { lb[i] = 0; ub[i] = 1; }

  return 0;
}

// Does the solver provides integer variables
bool glpk_solver::has_intvars() { return true; }

// Set range of an integer variable
int glpk_solver::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) { 
  lb[rank+1] = lower;
  ub[rank+1] = upper;
  return 0; 
}

// write the problem into a file
// int glpk_solver::writelp(const char *filename) { glp_write_lp(lp, NULL, filename); return 0; }

void glpk_solver::abort(void) {
  this->aborted = true;
  this->mip_params.tm_lim = 0;
  return;
}

void glpk_solver::set_mip_gap(double mip_gap) {
  this->mip_gap = mip_gap;
}

// solve the current lp problem
int glpk_solver::solve(int timeout) {
  int status = 0, nb_objectives = objectives.size();
  int save_stdout = 1;

  try {
  if (verbosity == 0) {
    save_stdout = dup(1);
    fclose(stdout); /* close(1) does not close stdout on macOS since macOS 12.7.1 / 13.6.3 / 14.2 (the bug has been reported) */
  }
  glp_init_iocp(&this->mip_params);
  this->mip_params.gmi_cuts = GLP_ON;
  this->mip_params.mir_cuts = GLP_ON;
  this->mip_params.cov_cuts = GLP_ON;
  this->mip_params.clq_cuts = GLP_ON;
  this->mip_params.presolve = GLP_ON;
  this->mip_params.binarize = GLP_ON;
  this->mip_params.tm_lim = timeout;
  this->mip_params.mip_gap = this->mip_gap;
  this->mip_params.msg_lev = (verbosity > 1) ? GLP_MSG_ON : GLP_MSG_OFF;
  // one of GLP_MSG_OFF GLP_MSG_ERR GLP_MSG_ON GLP_MSG_ALL

  this->aborted = false;

  for (int k = 0; k < nb_objectives; k++) {
    glp_cpx_basis(lp);
  
    if (status == 0) status = glp_intopt(lp, &this->mip_params);

    if (k + 1 < nb_objectives) {
      // Get objective value
      CUDFcoefficient objval = objective_value();

      if (verbosity > 0) PRINT_OUT(">>> Objective %d value : %" CUDFint64"d\n", k, objval);

      // Reset objective i coefficients
      for (int i = 1; i < objectives[k]->nb_coeffs + 1; i++) 
	glp_set_obj_coef(lp, objectives[k]->sindex[i], 0);

      // Set objective i+1 as the actual objective function
      for (int i = 1; i < objectives[k+1]->nb_coeffs + 1; i++) 
	glp_set_obj_coef(lp, objectives[k+1]->sindex[i], objectives[k+1]->coefficients[i]);

      // Add objective i = objval constraint
      int irow = glp_add_rows(lp, 1);
      glp_set_row_bnds(lp, irow, GLP_FX, objval, objval);
      glp_set_mat_row(lp, irow, objectives[k]->nb_coeffs, objectives[k]->sindex, objectives[k]->coefficients);    

      if (OUTPUT_MODEL) glp_write_lp(lp, NULL, "glpkpbs1.lp");
    }
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
  switch (status) {
  case GLP_EMIPGAP:
  case 0: {
    switch (glp_mip_status(lp)) {
    case GLP_FEAS:
    case GLP_OPT: return 1;
    case GLP_NOFEAS: return 0;
    default: return -1;
    }
  }
  case GLP_ENOPFS:
  case GLP_ENODFS:
    return 0;
  case GLP_ETMLIM:
    if (this->aborted)
      return -3;
    else
      return -2;
  default: return -1;
  }
}

int glpk_solver::solve() {
  return (this->solve(INT_MAX));
}

// get objective function value
CUDFcoefficient glpk_solver::objective_value() { return (CUDFcoefficient)CUDFnearbyint(glp_mip_obj_val(lp)); }

// solution initialisation
int glpk_solver::init_solutions() { return 0; }

// return the status of a package within the final configuration
CUDFcoefficient glpk_solver::get_solution(CUDFVersionedPackage *package) { return (CUDFcoefficient)CUDFnearbyint(glp_mip_col_val(lp, package->rank+1)); }

// initialize objective function
int glpk_solver::begin_objectives(void) { 
  glp_set_obj_dir(lp, GLP_MIN);  // Problem is minimization
  return 0; 
}

// return the package coefficient of the objective function 
CUDFcoefficient glpk_solver::get_obj_coeff(CUDFVersionedPackage *package) { return (CUDFcoefficient)get_coeff(package); }

// return the package coefficient of the objective function 
CUDFcoefficient glpk_solver::get_obj_coeff(int rank) { return (CUDFcoefficient)get_coeff(rank); }

// set package coefficient to a value
int glpk_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { set_coeff(package, value); return 0; }
// set column coefficient to a value
int glpk_solver::set_obj_coeff(int rank, CUDFcoefficient value) { set_coeff(rank, value); return 0; }

// initialize an additional objective function 
int glpk_solver::new_objective(void) {
  reset_coeffs();
  return 0;
}

// add an additional objective function
int glpk_solver::add_objective(void) { 
  push_obj(); 
  return 0;
}

// finalize the objective function
int glpk_solver::end_objectives(void) {
  int i = 1;
  for (CUDFVersionedPackageListIterator ipkg = all_versioned_packages->begin(); ipkg != all_versioned_packages->end(); ipkg++) {
    glp_set_col_bnds(lp, i, GLP_DB, 0, 1);  // Set bounds to [0, 1]
    glp_set_col_name(lp, i, (*ipkg)->versioned_name); // Set the colunm name
    glp_set_col_kind(lp, i, GLP_BV); // It is a binary variable ...
    i++;
  }
  for (i = nb_packages+1; i <= nb_vars; i++) {
    char *name;
    char buffer[20];

    sprintf(buffer, "x%d", i);
    if ((name = (char *)malloc(strlen(buffer)+1)) == (char *)NULL) {
      PRINT_ERR("CUDF error: can not alloc memory for variable name in glpk_solver::end_objective.\n");
      exit(-1);
    }
    strcpy(name, buffer);

    if ((lb[i] == 0) && (ub[i] == 1)) {
      glp_set_col_bnds(lp, i, GLP_DB, 0, 1);  // Set bounds to [0, 1]
      glp_set_col_name(lp, i, name); // Set the colunm name
      glp_set_col_kind(lp, i, GLP_BV); // It is a binary variable ...
    } else {
      glp_set_col_bnds(lp, i, GLP_DB, lb[i], ub[i]);  // Set bounds to [0, 1]
      glp_set_col_name(lp, i, name); // Set the colunm name
      glp_set_col_kind(lp, i, GLP_IV); // It is an integer variable ...
    }
  }

  // Set objective 0 as the actual objective function
  for (int k = 1; k < objectives[0]->nb_coeffs + 1; k++) glp_set_obj_coef(lp, objectives[0]->sindex[k], objectives[0]->coefficients[k]);

  return 0;
}

// initialize constraints
int glpk_solver::begin_add_constraints(void) { return 0; }

// begin a new constraint
int glpk_solver::new_constraint(void) { reset_coeffs(); return 0; }

// get the package coefficient of the current constraint
CUDFcoefficient glpk_solver::get_constraint_coeff(CUDFVersionedPackage *package) { return (CUDFcoefficient)get_coeff(package); }

// get the package coefficient of the current constraint
CUDFcoefficient glpk_solver::get_constraint_coeff(int rank) { return (CUDFcoefficient)get_coeff(rank); }

// set package coefficient of the current constraint
int glpk_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) { 
  set_coeff(package, value);
  return 0;
}

// set column coefficient of the current constraint
int glpk_solver::set_constraint_coeff(int rank, CUDFcoefficient value) { 
  set_coeff(rank, value);
  return 0;
}

// add current constraint as a greater or equal constraint
int glpk_solver::add_constraint_geq(CUDFcoefficient bound) {
  if (nb_coeffs > 0 ) {
    int irow = glp_add_rows(lp, 1);
    glp_set_row_bnds(lp, irow, GLP_LO, bound, 0);
    glp_set_mat_row(lp, irow, nb_coeffs, sindex, coefficients);
  }
  return 0;
}

// add current constraint as a less or equal constraint
int glpk_solver::add_constraint_leq(CUDFcoefficient bound) {
  if (nb_coeffs > 0 ) {
    int irow = glp_add_rows(lp, 1);
    glp_set_row_bnds(lp, irow, GLP_UP, 0, bound);
    glp_set_mat_row(lp, irow, nb_coeffs, sindex, coefficients);
  }
  return 0;
}

// add current constraint as an equality constraint
int glpk_solver::add_constraint_eq(CUDFcoefficient bound) {
  if (nb_coeffs > 0 ) {
    int irow = glp_add_rows(lp, 1);
    glp_set_row_bnds(lp, irow, GLP_FX, bound, bound);
    glp_set_mat_row(lp, irow, nb_coeffs, sindex, coefficients);
  }
  return 0;
}

// finalize constraints
int glpk_solver::end_add_constraints(void) { 
  if (OUTPUT_MODEL) glp_write_lp(lp, NULL, "glpkpbs.lp"); 
  return 0; 
}

glpk_solver::~glpk_solver() {
  glp_delete_prob(lp);
  glp_free_env();
}

#endif // USEGLPK
