
/*******************************************************/
/* CUDF solver: count_criteria.c                       */
/* Implementation of the count criteria                */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// Count a property quantity

#include "count_criteria.h"

// Check property availability
void count_criteria::check_property(CUDFproblem *problem) {
  CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));

  has_property = false;

  if (prop == problem->properties->end())
    PRINT_OUT("WARNING: cannot find \"%s\" property definition: criteria count not used.\n", property_name);
  else
    switch ((*prop).second->type_id) {
    case pt_int: 
    case pt_nat: 
    case pt_posint:
      has_property = true;
      break;
    default:
      PRINT_OUT("WARNING: Property \"%s\" has wrong type: type must be an int, a nat or a posint. Criteria count not used.\n", property_name);
    }
}

inline bool appears_in_request(CUDFproblem * problem, CUDFVirtualPackage * p) {
  for (CUDFVpkgListIterator it = problem->install->begin(); it != problem->install->end(); it++)
    if ((*it)->virtual_package == p) return true;
  for (CUDFVpkgListIterator it = problem->upgrade->begin(); it != problem->upgrade->end(); it++)
    if ((*it)->virtual_package == p) return true;
  for (CUDFVpkgListIterator it = problem->remove->begin(); it != problem->remove->end(); it++)
    if ((*it)->virtual_package == p) return true;
  return false;
}

inline bool in_scope(Count_scope scope, CUDFproblem * problem, CUDFVersionedPackage * p) {
  switch (scope) {
  case REQUEST:
    return appears_in_request(problem, p->virtual_package);
  case NEW:
    return (p->virtual_package->highest_installed == (CUDFVersionedPackage *)NULL);
  case CHANGED:
    return !(p->installed);
  case SOLUTION:
    return true;
  default:
    return false;
  }
}

// Criteria initialization
void count_criteria::initialize(CUDFproblem *problem, abstract_solver *solver) {
  this->problem = problem;
  this->solver = solver;
  ub = lb = 0;

  if (has_property) {
    CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));
    
    default_value = 0;
    if ((*prop).second->default_value != ((CUDFPropertyValue *)NULL))
      switch ((*prop).second->default_value->property->type_id) {
      case pt_int: 
      case pt_nat: 
      case pt_posint:
	default_value = lambda_crit * (*prop).second->default_value->intval;
	break;
      default:
	break;
      }

    if (verbosity > 2) PRINT_OUT("count criteria default value for %s = %" CUDFint64"d\n", property_name, default_value);

    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      if (!in_scope(scope, problem, *ipkg))
        continue;
      bool got_property = false;
      for (CUDFPropertyValueListIterator propval = (*ipkg)->properties.begin();  propval != (*ipkg)->properties.end(); propval++)
	if ((*propval)->property == (*prop).second) {
	  CUDFcoefficient value = lambda_crit * (*propval)->intval;
	  if (value < 0) lb += value; else ub += value;
	  got_property = true;
	  break;
	}
      if (! got_property) { if (default_value < 0) lb += default_value; else ub += default_value; }
    }
  }

}

// Computing the number of columns required to handle the criteria
int count_criteria::set_variable_range(int first_free_var) { return first_free_var; }


// Add the criteria to the current objective function
int count_criteria::add_criteria_to_objective(CUDFcoefficient lambda) {
  if (has_property) {
    CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));

    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      if (!in_scope (scope, problem, *ipkg))
        continue;
      bool got_property = false;
      for (CUDFPropertyValueListIterator propval = (*ipkg)->properties.begin();  propval != (*ipkg)->properties.end(); propval++)
	if ((*propval)->property == (*prop).second) {
	  CUDFcoefficient value = lambda_crit * (*propval)->intval;
	  solver->set_obj_coeff(*ipkg, lambda * value + solver->get_obj_coeff(*ipkg));
	  got_property = true;
	  break;
	}
      if ((! got_property) && (default_value != 0)) solver->set_obj_coeff(*ipkg, lambda * default_value + solver->get_obj_coeff(*ipkg));
    }
  }
  return 0;
}

// Add the criteria to the constraint set
int count_criteria::add_criteria_to_constraint(CUDFcoefficient lambda) {
  if (has_property) {
    CUDFPropertiesIterator prop =  problem->properties->find(string(property_name));

    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      if (!in_scope(scope, problem, *ipkg))
        continue;
      bool got_property = false;
      for (CUDFPropertyValueListIterator propval = (*ipkg)->properties.begin();  propval != (*ipkg)->properties.end(); propval++)
	if ((*propval)->property == (*prop).second) {
	  CUDFcoefficient value = lambda_crit * (*propval)->intval;
	  solver->set_constraint_coeff(*ipkg, lambda * value + solver->get_constraint_coeff(*ipkg));
	  got_property = true;
	  break;
	}
      if ((! got_property) && (default_value != 0)) 
	solver->set_constraint_coeff(*ipkg, lambda * default_value + solver->get_constraint_coeff(*ipkg));
    }
  }
  return 0;
}

// Add the constraints required by the criteria
int count_criteria::add_constraints() { return 0; }

// Compute the criteria range
CUDFcoefficient count_criteria::bound_range() { return (ub - lb + 1); }

// Compute the criteria upper bound
CUDFcoefficient count_criteria::upper_bound() { return ub; }

// Compute the criteria lower bound
CUDFcoefficient count_criteria::lower_bound() { return lb; }


