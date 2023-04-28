/*******************************************************/
/* CUDF solver: lp_solver.c                            */
/* Interface to the lp format solvers                  */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "lp_solver.h"

#define CLEAN_FILES 1
#ifdef _WIN32
#define TMP_FILES_PATH temp_files_path
static char temp_files_path[MAX_PATH+1];
#else
#define TMP_FILES_PATH "/tmp/"
#endif

static unsigned long pid = 0;
static unsigned long uid = 0;

// external function for solver creation
abstract_solver *new_lp_solver(const char *lpsolver) { return new lp_solver(lpsolver); }

// solver initialisation
int lp_solver::init_solver(CUDFVersionedPackageList *all_versioned_packages, int other_vars) {

  nb_packages = all_versioned_packages->size();
  this->all_versioned_packages = all_versioned_packages;

  // Coefficient initialization
  initialize_coeffs(nb_packages + other_vars);

  nb_constraints = 0;
  mult = ' ';

  solution = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));
  lb = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));
  ub = (CUDFcoefficient *)malloc(nb_vars*sizeof(CUDFcoefficient));

#ifdef _WIN32
  // GetTempPath's output includes a terminating slash
  if (!GetTempPath(MAX_PATH + 1, temp_files_path)) {
    fprintf(stderr, "lp_solver: unable to determine TEMP directory.\n");
    exit(-1);
  }
#endif

#ifndef _WIN32
  if (!uid) {
    uid = (unsigned long)getuid();
  }
#endif

  if (!pid) {
#ifdef _WIN32
    pid = GetCurrentProcessId();
#else
    pid = (unsigned long)getpid();
#endif
  }

  for (int i = 0; i < nb_vars; i++) { lb[i] = 0; ub[i] = 1; }

  sprintf(ctlpfilename, "%sctlp_%lu_%lu.lp", TMP_FILES_PATH, uid, pid);
  ctlpfile = fopen(ctlpfilename, "w");

  if ((solution == (CUDFcoefficient *)NULL) ||
      (lb == (CUDFcoefficient *)NULL) ||
      (ub == (CUDFcoefficient *)NULL)) {
    fprintf(stderr, "lp_solver: initialize: not enough memory.\n");
    exit(-1);
  } else if (ctlpfile == (FILE *)NULL) {
    fprintf(stderr, "lp_solver: initialize: cannot open %s.\n", ctlpfilename);
    exit(-1);
  } else
    return 0;
}

// write the problem into a file
int lp_solver::writelp(const char *filename) { return 0; }

void lp_solver::set_mip_gap(double mip_gap) {} // TODO ?

// solve the current problem
int lp_solver::solve() {
  int status = 0;
  int rank, iobjval;
  char command[2048];
  FILE *fsol = (FILE *)NULL;
  CUDFcoefficient objvals[20];
  unsigned int nb_objectives = objectives.size();

  sprintf(lpfilename, "%slppbs_%lu_%lu.lp", TMP_FILES_PATH, uid, pid);
  sprintf(lpoutfilename, "%slppbs_%lu_%lu.out", TMP_FILES_PATH, uid, pid);

  for (unsigned int iobj = 0; iobj < nb_objectives; iobj++) {
    if (objectives[iobj]->nb_coeffs == 0) continue;

    if ((lpfile = fopen(lpfilename, "w")) == (FILE *)NULL) {
      fprintf(stderr, "lp_solver: cannot open %s.\n", lpfilename);
      exit(-1);
    }

    fprintf(lpfile, "Minimize\n obj:");
    for (int i = 0, nbc = 0; i < objectives[iobj]->nb_coeffs; i++, nbc++) {
      if (nbc == 20) { // scip does not like too long lines
        nbc = 0;
        fprintf(lpfile, "\n  ");
      }
      fprintf(lpfile, " " CUDFflagsplus "%cx%d", objectives[iobj]->coefficients[i], mult, objectives[iobj]->sindex[i]);
    }
    fprintf(lpfile, "\n");

    fprintf(lpfile, "Subject To\n");
    for (unsigned int i = 0; i < iobj; i++) {
      if (objectives[i]->nb_coeffs > 0) {
        for (int j = 0, nbc = 0; j < objectives[i]->nb_coeffs; j++, nbc++) {
          if (nbc == 20) { // scip does not like too long lines
            nbc = 0;
            fprintf(lpfile, "\n  ");
          }
          fprintf(lpfile, " " CUDFflagsplus "%cx%d", objectives[i]->coefficients[j], mult, objectives[i]->sindex[j]);
        }
        fprintf(lpfile, " = " CUDFflags "\n", objvals[i]);
      }
    }

    fclose(lpfile);

    if (verbosity < 2)
#ifdef _WIN32
      sprintf(command, "cat %s >> %s && %s %s > %s 2> nul",
#else
      sprintf(command, "cat %s >> %s; %s %s > %s 2> /dev/null",
#endif
              ctlpfilename, lpfilename, lpsolver, lpfilename, lpoutfilename);
    else
      sprintf(command, "cat %s >> %s && %s %s | tee %s",
              ctlpfilename, lpfilename, lpsolver, lpfilename, lpoutfilename);

    if (system(command) == -1) {
      fprintf(stderr, "mccs: error while calling solver '%s'.\n", lpsolver);
      exit(-1);
    }

    if ((fsol = fopen(lpoutfilename, "r")) == (FILE *)NULL) {
      fprintf(stderr, "Cannot open solution file \"%s\".\n", lpoutfilename);
      exit(-1);
    }

    status = -1;
    while ((status == -1) && (! feof(fsol)) && (fgets(command, 1000, fsol) != NULL))
      switch (command[0]) {
      case 'p': // scip ?
        if (strncmp(command, "primal solution:", 16) == 0) {
          if (fgets(command, 1000, fsol) != NULL)  // read ===========
            if (fgets(command, 1000, fsol) != NULL)  // read empty line
              if (fgets(command, 1000, fsol) != NULL) { // read objective value or no solution
                if (strncmp(command, "objective value:", 16) == 0) {
                  status = 1;
                  if (sscanf(command+16, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval;

                  // Reading scip solution
                  if (iobj + 1 == nb_objectives) { // read solutions

                    for (int i = 0; i < nb_packages; i++) solution[i] = 0; // Set solution values to 0

                    while ((! feof(fsol)) && (fgets(command, 1000, fsol) != NULL))
                      if (command[0] == 'x') {
                        if (sscanf(command+1, "%d", &rank) > 0)
                          solution[rank] = 1;
                      } else // end of solution reached
                        break;
                  }
                } else if (strncmp(command, "no solution available", 21) == 0) {
                  status = 0;
                }
              } else {
                fprintf(stderr, "mccs: error while reading solution file.\n");
                exit(-1);
              }
            else {
              fprintf(stderr, "mccs: error while reading solution file.\n");
              exit(-1);
            }
          else {
            fprintf(stderr, "mccs: error while reading solution file.\n");
            exit(-1);
          }
        }
        break;
      case 'C':  // COIN or CPLEX ?
        if ((strncmp(command, "Coin:Infeasible - objective value", 33) == 0) ||
            (strncmp(command, "CPLEX> MIP - Integer infeasible.", 32) == 0))
          status = 0;
        else if (strncmp(command, "Coin:Optimal - objective value", 30) == 0) {
          status = 1;
          if (sscanf(command+30, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval;

          // Reading COIN solution
          if (iobj + 1 == nb_objectives) { // read solutions

            for (int i = 0; i < nb_packages; i++) solution[i] = 0; // Set solution values to 0

            while ((! feof(fsol)) && (fgets(command, 1000, fsol) != NULL))
              if ((command[0] == ' ') && (command[8] == 'x')) {
                if (sscanf(command+9, "%d", &rank) > 0)
                  if (((command[30] == ' ') && (command[31] == '1')) ||
                      ((command[31] == ' ') && (command[32] == '1')) ||
                      ((command[32] == ' ') && (command[33] == '1')) ||
                      ((command[33] == ' ') && (command[34] == '1'))) solution[rank] = 1;
              } else // end of solution reached
                break;
          }
        } else if (strncmp(command, "CPLEX> MIP - Integer optimal solution:  Objective = ", 52) == 0) {
          status = 1;
          if (sscanf(command+52, "%d", &iobjval) > 0) objval = objvals[iobj] = iobjval;

          // Reading CPLEX solution
          if (iobj + 1 == nb_objectives) { // read solutions

            for (int i = 0; i < nb_packages; i++) solution[i] = 0; // Set solution values to 0

            if (fgets(command, 1000, fsol) != NULL) { //  Forget two next lines
              if (fgets(command, 1000, fsol) != NULL) {
                while ((! feof(fsol)) && (fgets(command, 1000, fsol) != NULL))
                  if (command[0] == 'x') {
                    if (sscanf(command+1, "%d", &rank) > 0) solution[rank] = 1;
                  } else // end of solution reached
                    break;
              } else {
                fprintf(stderr, "mccs: error while reading solution file.\n");
                exit(-1);
              }
            } else {
              fprintf(stderr, "mccs: error while reading solution file.\n");
              exit(-1);
            }
          }
        }
        break;
      }
    fclose(fsol);
    // If we are here with a status = -1, then we were enable to read the solution (or the infeasability)
    if (status == -1) {
      fprintf(stderr, "ERROR: Cannot read solution from lp solver.\n");
      exit(-1);
    }
  } // end for objectives

  if (CLEAN_FILES) {
    remove(ctlpfilename);
    remove(lpfilename);
    remove(lpoutfilename);
  }

  return status;
}

int lp_solver::solve(int timeout) {
  // warning: timeout unimplemented
  return solve();
}
// get objective function value
CUDFcoefficient lp_solver::objective_value() { return objval; }

void lp_solver::abort() {
  // SIGINT is normally sent directly to the child (solver) process while it
  // runs using system()
  return;
}

// solution initialisation
int lp_solver::init_solutions() { return 0; }

// lp solvers have integer variables
bool lp_solver::has_intvars() { return true; }

// set integer variable range (must be used before end_objective)
int lp_solver::set_intvar_range(int rank, CUDFcoefficient lower, CUDFcoefficient upper) {
  lb[rank] = lower;
  ub[rank] = upper;
  return 0;
}

// return the status of a package within the final configuration
CUDFcoefficient lp_solver::get_solution(CUDFVersionedPackage *package) { return solution[package->rank]; }

// initialize objective function
int lp_solver::begin_objectives(void) { return 0; }

// return the package coefficient of the objective function
CUDFcoefficient lp_solver::get_obj_coeff(CUDFVersionedPackage *package) { return get_coeff(package); }

// return the package coefficient of the objective function
CUDFcoefficient lp_solver::get_obj_coeff(int rank) { return get_coeff(rank); }

// set the package coefficient of the objective function
int lp_solver::set_obj_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  set_coeff(package, value);
  return 0;
}

// set the column coefficient of the objective function
int lp_solver::set_obj_coeff(int rank, CUDFcoefficient value) {
  set_coeff(rank, value);
  return 0;
}

int lp_solver::new_objective(void) {
  reset_coeffs();
  return 0;
}

// add current objective to the set of objectives
int lp_solver::add_objective(void) {
  push_obj();
  return 0;
}

// finalize the objective function
int lp_solver::end_objectives(void) { return 0; }

// initialize constraints
int lp_solver::begin_add_constraints(void) { return 0; }

// begin a new constraint
int lp_solver::new_constraint(void) {
  reset_coeffs();
  return 0;
}

// get the package coefficient of the current constraint
CUDFcoefficient lp_solver::get_constraint_coeff(CUDFVersionedPackage *package) { return get_coeff(package); }

// get the package coefficient of the current constraint
CUDFcoefficient lp_solver::get_constraint_coeff(int rank) { return get_coeff(rank); }

// set package coefficient of the current constraint
int lp_solver::set_constraint_coeff(CUDFVersionedPackage *package, CUDFcoefficient value) {
  set_coeff(package, value);
  return 0;
}

// set column coefficient of the current constraint
int lp_solver::set_constraint_coeff(int rank, CUDFcoefficient value) {
  set_coeff(rank, value);
  return 0;
}

// add current constraint as a greater equal constraint
int lp_solver::add_constraint_geq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++)
      fprintf(ctlpfile, " " CUDFflagsplus "%cx%d", coefficients[i], mult, sindex[i]);
    if (bound == 0) fprintf(ctlpfile, " >= 0\n"); else fprintf(ctlpfile, " >= " CUDFflags "\n", bound);
    nb_constraints++;
  }
  return 0;
}

// add current constraint as a less or equal constraint
int lp_solver::add_constraint_leq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++)
      fprintf(ctlpfile, " " CUDFflagsplus "%cx%d", coefficients[i], mult, sindex[i]);
    if (bound == 0) fprintf(ctlpfile, " <= 0\n"); else fprintf(ctlpfile, " <= " CUDFflags "\n", bound);
    nb_constraints++;
  }
  return 0;
}

// add current constraint as an equality constraint
int lp_solver::add_constraint_eq(CUDFcoefficient bound) {
  if (nb_coeffs > 0) {
    for (int i = 0; i < nb_coeffs; i++)
      fprintf(ctlpfile, " " CUDFflagsplus "%cx%d", coefficients[i], mult, sindex[i]);
    if (bound == 0) fprintf(ctlpfile, " = 0\n"); else fprintf(ctlpfile, " = " CUDFflags "\n", bound);
    nb_constraints++;
  }
  return 0;
}

// finalize constraints
int lp_solver::end_add_constraints(void) {
  int nbcols = 0, nbvars = 0;

  fprintf(ctlpfile, "Bounds\n");
  for (int i = 0; i < nb_vars; i++) {
    fprintf(ctlpfile, " " CUDFflags " <= x%d <= " CUDFflags "\n", lb[i], i, ub[i]);
  }

  fprintf(ctlpfile, "Binaries\n");
  for (int i = 0; i < nb_vars; i++) {
    if ((lb[i] == 0) && (ub[i] == 1)) {
      nbcols++;
      if (nbcols == 10) { fprintf(ctlpfile, "\n"); nbcols = 0; }
      fprintf(ctlpfile, " x%d", i);
    }
  }
  for (int i = 0; i < nb_vars; i++) {
    if ((lb[i] != 0) || (ub[i] != 1)) {
      if (nbvars == 0) fprintf(ctlpfile, "\nGenerals\n");
      nbcols++;
      nbvars++;
      if (nbcols == 10) { fprintf(ctlpfile, "\n"); nbcols = 0; }
      fprintf(ctlpfile, " x%d", i);
    }
  }
  fprintf(ctlpfile, "\nEnd\n");
  fclose(ctlpfile);
  return 0;
}
