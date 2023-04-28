/*******************************************************/
/* CUDF solver: cud.c                                  */
/* main of the cudf solver                             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include "cudf.h"
#include "abstract_solver.h"
#include "constraint_generation.h"
#include "criteria.h"
#include "combiner.h"
#include "cudf_reductions.h"
#include <sys/stat.h>
#include <cerrno>
#include <sstream>
#include "mccscudf.h"

// underlying solver declaration
// allows using solvers withour having to include the whole solver classes
extern abstract_solver *new_lp_solver(const char *lpsolver);
/* extern abstract_solver *new_pblib_solver(char *pbsolver); */
//extern abstract_solver *new_ampl_solver(char *amplsolver);
#ifdef USECPLEX 
extern abstract_solver *new_cplex_solver();
#endif
#ifdef USEGUROBI
extern abstract_solver *new_gurobi_solver();
#endif
#ifdef USELPSOLVE 
extern abstract_solver *new_lpsolve_solver();
#endif
#ifdef USEGLPK
extern abstract_solver *new_glpk_solver(bool use_exact);
#endif

#ifdef USECOIN
  #include "osi_solver.h"
  #ifdef USECLP
    #include <coin/OsiClpSolverInterface.hpp>
  #endif
  #ifdef USECBC
    #include <coin/OsiCbcSolverInterface.hpp>
  #endif
  #ifdef USESYM
    #include <coin/OsiSymSolverInterface.hpp>
  #endif
#endif

bool criteria_opt_var = false;
// Basic user defined criteria option handling
int get_criteria_options(char *crit_descr, unsigned int &pos, vector< pair<unsigned int, unsigned int> *> *opts) {

  if (crit_descr[pos] == '[') {
    int nb_read = 0;
    unsigned int start = ++pos;

    for (; pos < strlen(crit_descr); pos++)
      switch(crit_descr[pos]) {
      case '[':
	crit_descr[pos] = '\0';
	PRINT_ERR("ERROR: criteria options: found '[' within criteria options: %s.\n", crit_descr);
	exit(-1);
	break;
      case ']': 
	{
	  unsigned int length = pos - start;
	  if (length == 0) {
	    crit_descr[pos] = '\0';
	    PRINT_ERR("ERROR: criteria options: found empty criteria option: %s.\n", crit_descr);
	    exit(-1);
	  }
	  opts->push_back(new pair<unsigned int, unsigned int>(start, length));
	  nb_read++;
	  pos++; // ignore ending ']'
	  return nb_read;
	}
	break;
      case ',':
	{
	  unsigned int length = pos - start;
	  if (length == 0) {
	    crit_descr[pos] = '\0';
	    PRINT_ERR("ERROR: criteria options: found empty criteria option: %s.\n", crit_descr);
	    exit(-1);
	  }
	  opts->push_back(new pair<unsigned int, unsigned int>(start, length));
	  nb_read++;
	  start = ++pos;
	}
	break;
      }

    PRINT_ERR("ERROR: criteria options: criteria options ended without an ending ']': %s.\n", crit_descr);
    exit(-1);
  } 

  return 0;
}

// Get user defined weight for a criteria
CUDFcoefficient get_criteria_lambda(char *crit_descr, unsigned int &pos, char sign) {
  CUDFcoefficient lambda = 1;
  vector< pair<unsigned int, unsigned int> *> opts;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 1) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    for (unsigned int i = 0; i < length; i++) 
      if ((crit_descr[start+i] < '0') || (crit_descr[start+i] > '9')) {
	crit_descr[start+i+1] = '\0';
	PRINT_ERR("ERROR: criteria options: a lambda value must be an integer int: %s\n", crit_descr);
	exit(-1);
      }

    if (sscanf(crit_descr+start, "%" CUDFint64"d", &lambda) != 1) {
      crit_descr[start+length+1] = '\0';
      PRINT_ERR("ERROR: criteria options: a lambda value is espected here: %s\n", crit_descr);
      exit(-1);
    }
  } else if (n > 1) {
    crit_descr[pos] = '\0';
    PRINT_ERR("ERROR: criteria options: a lambda value is espected here: %s\n", crit_descr);
    exit(-1);
  }

  if (sign == '+') lambda *= -1;

  return lambda;
}

// Get property name from a user defined criteria
char *get_criteria_property_name(char *crit_descr, unsigned int &pos) {
  vector< pair<unsigned int, unsigned int> *> opts;
  char *property = (char *)NULL;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 1) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    if (crit_descr[start+length-1] == ':') length--;

    if ((property = (char *)malloc((length+1)*sizeof(char))) == (char *)NULL) {
      PRINT_ERR("ERROR: criteria options: not enough memory to store property name.\n");
      exit(-1);
    }
    
    strncpy(property, crit_descr+start, length);
    property[length] = '\0';
  } else {
    crit_descr[pos] = '\0';
    PRINT_ERR("ERROR: criteria options: a property name is required here: %s\n", crit_descr);
    exit(-1);
  }

  return property;
}

bool str_is(unsigned int pos, const char * ref, const char * s, int start) {
  int len = strlen(ref);
  return (int)pos - start == len && strncmp(ref, s + start, len) == 0;
}

// Get name and boolean options from user defined criteria
char *get_criteria_property_name_and_scope(char *crit_descr, unsigned int &pos, Count_scope &scope) {
  vector< pair<unsigned int, unsigned int> *> opts;
  char *property = (char *)NULL;

  int n = get_criteria_options(crit_descr, pos, &opts);

  if (n == 2) {
    unsigned int start = opts[0]->first;
    unsigned int length = opts[0]->second;

    if (crit_descr[start+length-1] == ':') length--;

    if ((property = (char *)malloc((length+1)*sizeof(char))) == (char *)NULL) {
      PRINT_ERR("ERROR: criteria options: not enough memory to store property name.\n");
      exit(-1);
    }
    
    strncpy(property, crit_descr+start, length);
    property[length] = '\0';

    start = opts[1]->first;
    length = opts[1]->second;

    if (str_is(pos-1, "request", crit_descr, start))
      scope = REQUEST;
    else if (str_is(pos-1, "new", crit_descr, start))
      scope = NEW;
    else if (str_is(pos-1, "changed", crit_descr, start) ||
             str_is(pos-1, "true", crit_descr, start))
      scope = CHANGED;
    else if (str_is(pos-1, "solution", crit_descr, start) ||
             str_is(pos-1,"false", crit_descr, start))
      scope = SOLUTION;
    else {
      crit_descr[start+length] = '\0';
      PRINT_ERR("ERROR: criteria options: one of 'request', 'new', 'changed' or 'solution' is required here: '%s'\n", crit_descr+start);
      exit(-1);
    }
  } else {
    crit_descr[pos] = '\0';
    PRINT_ERR("ERROR: criteria options: a property name and a scope (one of 'request', 'new', 'changed' or 'solution') are required here: %s\n", crit_descr);
    exit(-1);
  }

  return property;
}

// Process a user defined criteria
CriteriaList *process_criteria(char *crit_descr, unsigned int &pos, bool first_level, vector<abstract_criteria *> *criteria_with_property) {
  CriteriaList *criteria = new CriteriaList();

  if (crit_descr[pos] == '[') {
    for (pos += 1; pos < strlen(crit_descr) && crit_descr[pos] != ']';) {
      unsigned int sign, crit_name;

      // check for criteria sense
      switch (crit_descr[pos]) {
      case '+':
      case '-':
	sign = pos++;
	crit_name = pos;
	break;
      default:
	PRINT_ERR("ERROR: criteria options: a criteria description must begin with a sign which gives its sense (- = min, + = max): %s\n",
		crit_descr+pos);
	exit(-1);
	break;
      }

      // look for end of criteria name
      for (; pos < strlen(crit_descr); pos++) {
	char c = crit_descr[pos];
	if ((c == ',') || (c == '[') || (c == ']')) break; 
      }

      // handle criteria
      if (str_is(pos, "removed", crit_descr, crit_name)) {
	criteria->push_back(new removed_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (str_is(pos, "changed", crit_descr, crit_name)) {
	criteria->push_back(new changed_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (str_is(pos, "new", crit_descr, crit_name)) {
	criteria->push_back(new new_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (str_is(pos, "notuptodate", crit_descr, crit_name)) {
	criteria->push_back(new notuptodate_criteria(get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else if (str_is(pos, "count", crit_descr, crit_name)) {
	Count_scope scope = SOLUTION;
	char *property_name = get_criteria_property_name_and_scope(crit_descr, pos, scope);
	if (property_name != (char *)NULL) {
	  abstract_criteria *crit = new count_criteria(property_name, scope, get_criteria_lambda(crit_descr, pos, crit_descr[sign]));
	  criteria_with_property->push_back(crit);
	  criteria->push_back(crit);
	}
      } else if (str_is(pos, "lexagregate", crit_descr, crit_name)) {
	criteria->push_back(new lexagregate_combiner(process_criteria(crit_descr, pos, false, criteria_with_property), 
						     get_criteria_lambda(crit_descr, pos, crit_descr[sign])));
      } else {
	crit_descr[pos] = '\0';
	PRINT_ERR("ERROR: criteria options: this is not a criteria: %s\n", crit_descr+crit_name);
	exit(-1);
      }

      if (crit_descr[pos] == ',') pos++; // skip comma
    }
  } else {
    PRINT_ERR("ERROR: criteria options: a criteria list must begin with a '[': %s\n", crit_descr+pos);
    exit(-1);
  }

  pos++;
  return criteria;
}

// Handling user criteria definitions
CriteriaList *get_criteria(char *crit_descr, bool first_level, vector<abstract_criteria *> *criteria_with_property) {
  unsigned int pos = 0;
  return process_criteria(crit_descr, pos, first_level, criteria_with_property);
}

Solver_return call_mccs(Solver solver_arg, char *criteria_arg, int timeout, double mip_gap, CUDFproblem* the_problem) {
  abstract_solver *solver;
  return call_mccs(solver_arg, criteria_arg, timeout, mip_gap, the_problem, &solver);
}

Solver_return call_mccs(Solver solver_arg, char *criteria_arg, int timeout, double mip_gap, CUDFproblem* the_problem, abstract_solver **solver_ptr) {
  CUDFproblem *problem = the_problem;
  vector<abstract_criteria *> criteria_with_property;
  CriteriaList *criteria = get_criteria(criteria_arg, false, &criteria_with_property);
  abstract_solver *solver = (abstract_solver *)NULL;
  abstract_combiner *combiner = (abstract_combiner *)NULL;
  Solver_return ret = { 0, "", NULL, NULL };
  bool no_solution = false;

  if (criteria->size() == 0) {
    ret.error = "invalid criteria";
    return ret;
  }
  combiner = new lexagregate_combiner(criteria);

  switch (solver_arg.backend) {
  case LP: solver = new_lp_solver(solver_arg.lp_solver); break;
#ifdef USECPLEX
  case CPLEX: solver = new_cplex_solver(); break;
#else
  case CPLEX: ret.error = "This mccs is built without cplex support"; return ret;
#endif
#ifdef USEGUROBI
  case GUROBI: solver = new_gurobi_solver(); break;
#else
  case GUROBI: ret.error = "This mccs is built without gurobi support"; return ret;
#endif
#ifdef USELPSOLVE
  case LPSOLVE: solver = new_lpsolve_solver(); break;
#else
  case LPSOLVE: ret.error = "This mccs is built without lpsolve support"; return ret;
#endif
#ifdef USEGLPK
  case GLPK: solver = new_glpk_solver(false); break;
#else
  case GLPK: ret.error = "This mccs is built without glpk support"; return ret;
#endif
#ifdef USECOIN
#ifdef USECLP
  case CLP: solver = new osi_solver<OsiClpSolverInterface>(); break;
#else
  case CLP: ret.error = "This mccs is built without COIN/CLP support"; return ret;
#endif
#ifdef USECBC
  case CBC: solver = new osi_solver<OsiCbcSolverInterface>(); break;
#else
  case CBC: ret.error = "This mccs is built without COIN/CBC support"; return ret;
#endif
#ifdef USESYM
  case SYMPHONY: solver = new osi_solver<OsiSymSolverInterface>(); break;
#else
  case SYMPHONY: ret.error = "This mccs is built without COIN/SYMPHONY support"; return ret;
#endif
#else
  case CLP: case CBC: case SYMPHONY:
    ret.error = "This mccs is built without COIN support"; return ret;
#endif
  default: ret.error = "Unrecognised solver specified"; return ret;
  }

  *solver_ptr = solver;

  // check criteria properties
  for (vector<abstract_criteria *>::iterator icrit = criteria_with_property.begin(); icrit != criteria_with_property.end(); icrit++)
    (*icrit)->check_property(the_problem);

  if (combiner->can_reduce()) {
    if (verbosity > 0) PRINT_OUT("Can reduce graph.\n");
    problem = compute_reduced_CUDF(the_problem);
  } else {
    if (verbosity > 0) PRINT_OUT("Can NOT reduce graph.\n");
  }
  ret.problem = problem;

  // combiner initialization
  combiner->initialize(problem, solver);
  
  ret.success = 1;
  // generate the constraints, solve the problem and print out the solutions
  if (problem->all_packages->size() == 0) {
    if (verbosity > 0) PRINT_OUT("========\nEmpty problem.\n");
    no_solution = true;
  }
  if (! no_solution && generate_constraints(problem, *solver, *combiner) < 0) {
    if (verbosity > 0) PRINT_OUT("========\nConstraint generation error.\n");
    no_solution = true;
  }
  solver->set_mip_gap(mip_gap);
  if (! no_solution) {
    int s = (timeout > 0) ? solver->solve(timeout) : solver->solve();
    if (s <= 0) {
      no_solution = true;
      switch(s) {
      case 0:
        if (verbosity > 0) PRINT_OUT("========\nNo solution found.\n");
        break;
      case -2:
        ret.success = -1;
        ret.error = "Timeout";
        if (verbosity > 0) PRINT_OUT("========\nSolver timed out.\n");
        break;
      case -3:
        ret.success = -2;
        ret.error = "Solver interrupted by SIGINT";
        if (verbosity > 0) PRINT_OUT("========\nSolver interrupted.\n");
        break;
      default:
        ret.success = 0;
        ret.error = "Mip solver failure";
        if (verbosity > 0) PRINT_OUT("========\nMip solver failed.\n");
      }
    }
  }
  delete combiner;
  for (vector<abstract_criteria*>::iterator it = criteria->begin(); it != criteria->end(); it++) delete(*it);
  delete criteria;
  if (no_solution) return ret;

  solver->init_solutions();

  if (verbosity > 2) {
    double obj = solver->objective_value();
    PRINT_OUT("================================================================\n");
    PRINT_OUT("Objective value: %f\n", obj);

    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++)
      PRINT_OUT("%s = %" CUDFint64"d\n", (*ipkg)->versioned_name, solver->get_solution(*ipkg));
      
    PRINT_OUT("================================================================\n");
    
  }

  ret.success = 1;
  ret.solution = solver;
  return ret;
}

int has_backend (Solver_backend backend) {
  switch(backend) {
  case LP:
    return 1;
  case CPLEX:
#ifdef USECPLEX
    return 1;
#else
    return 0;
#endif
  case GUROBI:
#ifdef USEGUROBI
    return 1;
#else
    return 0;
#endif
  case LPSOLVE:
#ifdef USELPSOLVE
    return 1;
#else
    return 0;
#endif
  case GLPK:
#ifdef USEGLPK
    return 1;
#else
    return 0;
#endif
  case CLP:
#ifdef USECLP
    return 1;
#else
    return 0;
#endif
  case CBC:
#ifdef USECBC
    return 1;
#else
    return 0;
#endif
  case SYMPHONY:
#ifdef USESYM
    return 1;
#else
    return 0;
#endif
  default:
    return 0;
  }
}
