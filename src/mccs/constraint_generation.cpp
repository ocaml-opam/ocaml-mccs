
/*******************************************************/
/* CUDF solver: constraint_generation.c                */
/* constraint generation for cudf problems             */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// translate a CUDF problem in a MILP problem

#include <constraint_generation.h>

bool generate_desagregate_constraints = false;
bool generate_agregate_constraints = true;

// check if pkg belongs to a list of providers
bool is_in_provl(const CUDFVersionedPackage *pkg, CUDFProviderList *provl) {
  for (CUDFVersionedPackageListIterator ipkg = provl->begin(); ipkg != provl->end(); ipkg++)
    if ((*ipkg) == pkg) return true;
  return false;
}

// check if pkg belongs to a list of removed packages
bool is_in_remove(const CUDFVersionedPackage *pkg, CUDFVersionedPackageList *remove_set) {
  for (CUDFVersionedPackageListIterator ipkg = remove_set->begin(); ipkg != remove_set->end(); ipkg++)
    if ((*ipkg) == pkg) return true;
  return false;
}

// check if pkg, version belongs to a sublist of providers 
bool is_in_versioned_providers(const CUDFVersionedPackage *pkg, const CUDFVersion version,
			       const CUDFVersionedProviderListIterator vpbegin, 
			       const CUDFVersionedProviderListIterator vpend) {
  CUDFVersionedProviderListIterator ivp = vpbegin;

  while (ivp != vpend) {
    if (ivp->first != version)
      for (CUDFProviderListIterator kpkg = ivp->second.begin(); kpkg != ivp->second.end(); kpkg++)
	if ((*kpkg) == pkg) return true;
    ivp++;
  }
  return false;
}

// preprocess an upgrade request to insure that only one version can be installed
int preprocess_upgrade(CUDFproblem *problem, int &new_var, vector<an_upgrade_set> &upgrades) {
  if (problem->upgrade != (CUDFVpkgList *)NULL) {
    int firstvarrank = problem->all_packages->size();

    for (CUDFVpkgListIterator ipkgop = problem->upgrade->begin(); ipkgop != problem->upgrade->end(); ipkgop++) {
      int nb_new_var = 0;
      CUDFVersionedPackageList remove_set;
      CUDFVersionedProviderList upgrade_set;

      CUDFVirtualPackage *vpackage = (*ipkgop)->virtual_package;

      CUDFVersion highest_version = vpackage->highest_installed_provider_version;

      CUDFVersionedPackageSetIterator iverpkg = vpackage->all_versions.begin();
      CUDFVersionedPackageSetIterator iverpkgend = vpackage->all_versions.end();
      CUDFVersionedProviderListIterator iprov = vpackage->versioned_providers.begin(); 
      CUDFVersionedProviderListIterator iprovend = vpackage->versioned_providers.end(); 


      if (vpackage->highest_installed != (CUDFVersionedPackage *)NULL)
	if (vpackage->highest_installed->version > highest_version)
	  highest_version = vpackage->highest_installed->version;

      // even if not installed, it seems that we have to upgrade ...
      // If nothing is installed ... it cannot be upgraded ...
      //      if (highest_version == 0) continue;

      // Remove all the "all versions" providers
      for (CUDFProviderListIterator jprov = vpackage->providers.begin(); jprov != vpackage->providers.end(); jprov++)
	remove_set.push_back(*jprov);

      // Remove all packages with version lower than hver
      for (; (iverpkg != iverpkgend) && ((*iverpkg)->version < highest_version); iverpkg++)
	remove_set.push_back(*iverpkg);
      // The same for versioned providers
      for (; (iprov != iprovend) && (iprov->first < highest_version); iprov++)
	for (CUDFProviderListIterator kpkg = iprov->second.begin(); kpkg != iprov->second.end(); kpkg++)
	  remove_set.push_back(*kpkg);

      // Upgrade highest_version version packages ... this does not seems to be right ...
      // the highest installed version package has also to go through the filters
      /*
      {
	CUDFProviderList hverpkgs;
	if ((iverpkg != vpackage->all_versions.end()) && 
	    ((*iverpkg)->version == highest_version) &&
	    (! is_in_remove(*iverpkg, &remove_set)) &&
	    (! is_in_versioned_providers(*iverpkg, (*iverpkg)->version, iprov, iprovend))) {
	  hverpkgs.push_back(*iverpkg);
	  iverpkg++;
	}
	if ((iprov != iprovend) &&
	    (iprov->first == highest_version)) {
	  for (CUDFProviderListIterator kpkg = iprov->second.begin(); kpkg != iprov->second.end(); kpkg++)
	    if (! is_in_versioned_providers(*kpkg, iprov->first, iprov, iprovend))
	      if (! is_in_provl(*kpkg, &hverpkgs)) hverpkgs.push_back(*kpkg);
	  iprov++;
	}
	if (hverpkgs.size() > 0) {
	  upgrade_set.insert(CUDFVersionedProviderList::value_type(highest_version, hverpkgs));
	  if (hverpkgs.size() > 1) nb_new_var++;
	}
      }
      */

      a_compptr comp = get_comparator((*ipkgop)->op);

      while ((iverpkg != iverpkgend) || (iprov != iprovend)) {
	CUDFProviderList provl;
	CUDFVersion version = 0;

	if (iverpkg != iverpkgend)
	  version = (*iverpkg)->version;
	if ((iprov != iprovend) && (version < iprov->first))
	  version = iprov->first;

	if ((iverpkg != iverpkgend) && ((*iverpkg)->version == version)) {
	  if (comp(version, (*ipkgop)->version)) {
	    if (! is_in_remove(*iverpkg, &remove_set)) {
	      if (is_in_versioned_providers(*iverpkg, (*iverpkg)->version, iprov, iprovend))
		remove_set.push_back(*iverpkg);
	      else
		provl.push_back(*iverpkg);
	    }
	  } else if (! is_in_remove(*iverpkg, &remove_set))
	      remove_set.push_back(*iverpkg);
	  iverpkg++;
	}

	if ((iprov != iprovend) && (iprov->first == version)) {
	  if (comp(version, (*ipkgop)->version)) {
	    for (CUDFProviderListIterator kpkg = iprov->second.begin(); kpkg != iprov->second.end(); kpkg++)
	      if (! is_in_remove(*kpkg, &remove_set)) {
		if (is_in_versioned_providers(*kpkg, iprov->first, iprov, iprovend) || ((*kpkg)->virtual_package == vpackage))
		  remove_set.push_back(*kpkg);
		else
		  if (! is_in_provl(*kpkg, &provl)) provl.push_back(*kpkg);
	      }
	  } else {
	    for (CUDFProviderListIterator kpkg = iprov->second.begin(); kpkg != iprov->second.end(); kpkg++)
	      if (! is_in_remove(*kpkg, &remove_set))
		remove_set.push_back(*kpkg);
	  }
	  iprov++;
	}

	if (provl.size() > 0) {
	  upgrade_set.insert(CUDFVersionedProviderList::value_type(version, provl));
	  if (provl.size() > 1) nb_new_var++;
	}
      }

      //      if ((remove_set.size() != 0) || (upgrade_set.size() != 0)) {
      if (upgrade_set.size() != 0) {
	upgrades.push_back(an_upgrade_set(nb_new_var, firstvarrank, remove_set, upgrade_set));
	new_var += nb_new_var;
	firstvarrank += nb_new_var;
      } else { // We need somethin to upgrade to ...
	if (verbosity > 0) PRINT_OUT("Cannot upgrade to %s.\n", vpackage->name);
	return -1;
      }
    }
  }
  return 0;
}

// Generate MILP objective function(s) and constraints for a given solver
// and a given criteria combination
int generate_constraints(CUDFproblem *problem, abstract_solver &solver, abstract_combiner &combiner) { 
  int new_var = 0;

  // set of requested upgrades
  vector<an_upgrade_set> upgrades;
  int nb_packages = problem->all_packages->size();
  int nb_vpackages = problem->all_virtual_packages->size();
  int nb_vars;
  vector<bool> keep_package_handled(nb_vpackages, false);
  CUDFVirtualPackageList installed_vpkgs_mv;
  CUDFVirtualPackageList installed_vpkgs_uv;

  if (nb_packages == 0) { // we lack a problem then ...
    PRINT_ERR("generate_constraints: no declared package !\n");
    exit(-1);
  }

  //----------------------------------------------------------------------------------------------------
  // Objective function


  if (preprocess_upgrade(problem, new_var, upgrades) != 0) return -1; // Trying to upgrade non existent package

  nb_vars = nb_packages + new_var;
  nb_vars = combiner.column_allocation(nb_vars);

  solver.init_solver(problem->all_packages, nb_vars - nb_packages);

  solver.begin_objectives();
  combiner.objective_generation();
  solver.end_objectives();

  //----------------------------------------------------------------------------------------------------
  // Constraints generation

  solver.begin_add_constraints();

  combiner.constraint_generation();

  // Install 
  if (problem->install != (CUDFVpkgList *)NULL) {
    for (CUDFVpkgListIterator ipkgop = problem->install->begin(); ipkgop != problem->install->end(); ipkgop++) {
      CUDFVirtualPackage *vpackage = (*ipkgop)->virtual_package;
      a_compptr comp = get_comparator((*ipkgop)->op);
      bool has_pkg = false;

      solver.new_constraint();
      if (vpackage->all_versions.size() > 0) // Install P = install one version of P
	for (CUDFVersionedPackageSetIterator ipkg = vpackage->all_versions.begin(); ipkg != vpackage->all_versions.end(); ipkg++)
	  if (use_pkg(*ipkg) && (comp((*ipkg)->version, (*ipkgop)->version))) {
	    has_pkg = true;
	    solver.set_constraint_coeff(*ipkg, +1);
	  }
      if (vpackage->providers.size() > 0) // or install one of the providers of P
	for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
	  if (use_pkg(*jpkg) && (solver.get_constraint_coeff(*jpkg) == 0)) {
	    has_pkg = true;
	    solver.set_constraint_coeff(*jpkg, +1);
	  }
      // or install one of the providers with the right version
      for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
	   jpkg != vpackage->versioned_providers.end(); jpkg++)
	if (comp(jpkg->first, (*ipkgop)->version))
	  for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++)
	    if (use_pkg(*kpkg) && (solver.get_constraint_coeff(*kpkg) == 0)) {
	      has_pkg = true;
	      solver.set_constraint_coeff(*kpkg, +1);
	    }
      if (has_pkg)
	solver.add_constraint_geq(+1);
      else
	return -1; // Cannot install this pkg
    }
  }

  // Remove
  if (problem->remove != (CUDFVpkgList *)NULL) {
    for (CUDFVpkgListIterator ipkgop = problem->remove->begin(); ipkgop != problem->remove->end(); ipkgop++) {
      CUDFVirtualPackage *vpackage = (*ipkgop)->virtual_package;
      a_compptr comp = get_comparator((*ipkgop)->op);
      bool has_pkg = false;

      if (generate_agregate_constraints) {
	solver.new_constraint();
	if (vpackage->all_versions.size() > 0) // Remove all the versions of the package
	  for (CUDFVersionedPackageSetIterator ipkg = vpackage->all_versions.begin(); ipkg != vpackage->all_versions.end(); ipkg++)
	    if (use_pkg(*ipkg) && (comp((*ipkg)->version, ((*ipkgop)->version)))) { has_pkg = true; solver.set_constraint_coeff(*ipkg, +1); }
	if (vpackage->providers.size() > 0) // as well as all the providers
	  for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
	    if (use_pkg(*jpkg) && (solver.get_constraint_coeff(*jpkg) == 0)) { has_pkg = true; solver.set_constraint_coeff(*jpkg, +1); }
	// as well as all the versioned providers with the right version
	for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
	     jpkg != vpackage->versioned_providers.end(); jpkg++)
	  if (comp(jpkg->first, (*ipkgop)->version))
	    for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++)
	      if (use_pkg(*kpkg) && (solver.get_constraint_coeff(*kpkg) == 0)) { has_pkg = true; solver.set_constraint_coeff(*kpkg, +1); }
	if (has_pkg)
	  solver.add_constraint_eq(0);
      }

      if (generate_desagregate_constraints) {
	if (vpackage->all_versions.size() > 0) // Remove all the versions of the package
	  for (CUDFVersionedPackageSetIterator ipkg = vpackage->all_versions.begin(); ipkg != vpackage->all_versions.end(); ipkg++)
	    if (use_pkg(*ipkg) && (comp((*ipkg)->version, ((*ipkgop)->version)))) { 
	      solver.new_constraint(); 
	      solver.set_constraint_coeff(*ipkg, +1); 
	      solver.add_constraint_eq(0);
	    }
	if (vpackage->providers.size() > 0) // as well as all the providers
	  for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
	    if (use_pkg(*jpkg)) { 
	      solver.new_constraint(); 
	      solver.set_constraint_coeff(*jpkg, +1); 
	      solver.add_constraint_eq(0);
	    }
	// as well as all the versioned providers with the right version
	for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
	     jpkg != vpackage->versioned_providers.end(); jpkg++)
	  if (comp(jpkg->first, (*ipkgop)->version))
	    for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++)
	      if (use_pkg(*kpkg)) { 
		solver.new_constraint(); 
		solver.set_constraint_coeff(*kpkg, +1); 
		solver.add_constraint_eq(0);
	      }
      }
    }
  }


  // Upgrade : There can be only one VERSION of (already_installed with higher version, + accepted by criteria and > higher version) installed, 
  // all other removed
  // WARNING: Providers not handled here ...
  if (upgrades.size() > 0) {
    for (vector<an_upgrade_set>::iterator iup = upgrades.begin(); iup != upgrades.end(); iup++) {

      // Remove packages belonging to upgrades remove set
      if (generate_agregate_constraints) {
	solver.new_constraint();
	for (CUDFVersionedPackageListIterator ipkg = (*iup).remove_set.begin(); ipkg != (*iup).remove_set.end(); ipkg++) {
	  if (solver.get_constraint_coeff(*ipkg) == 0) solver.set_constraint_coeff(*ipkg, +1);
	  //PRINT_OUT("upgrade => remove %s\n", (*ipkg)->name);
	}
	solver.add_constraint_eq(0);
      }
      if (generate_desagregate_constraints) {
	for (CUDFVersionedPackageListIterator ipkg = (*iup).remove_set.begin(); ipkg != (*iup).remove_set.end(); ipkg++) {
	  solver.new_constraint();
	  solver.set_constraint_coeff(*ipkg, +1);
	  solver.add_constraint_eq(0);
	}
      }

      // Force the installation of the upgraded packages
      int var_rank = (*iup).first_var_rank;
      bool has_mvp = false;
      solver.new_constraint();
      for (CUDFVersionedProviderListIterator ivpl = (*iup).upgrade_set.begin(); ivpl != (*iup).upgrade_set.end(); ivpl++)
	if (ivpl->second.size() == 1)
	  solver.set_constraint_coeff(ivpl->second.front(), +1);
	else {
	  has_mvp = true;
	  solver.set_constraint_coeff(var_rank++, +1);
	}
      solver.add_constraint_eq(1);
      if (has_mvp) {
	int var_rank = (*iup).first_var_rank;
	int size;
	for (CUDFVersionedProviderListIterator ivpl = (*iup).upgrade_set.begin(); ivpl != (*iup).upgrade_set.end(); ivpl++)
	  if ((size = ivpl->second.size()) > 1) {
	    solver.new_constraint();
	    for (CUDFProviderListIterator kpkg = ivpl->second.begin(); kpkg != ivpl->second.end(); kpkg++)
	      solver.set_constraint_coeff(*kpkg, +1);
	    solver.set_constraint_coeff(var_rank, -1);
	    solver.add_constraint_geq(0);
	    solver.new_constraint();
	    for (CUDFProviderListIterator kpkg = ivpl->second.begin(); kpkg != ivpl->second.end(); kpkg++)
	      solver.set_constraint_coeff(*kpkg, -1);
	    solver.set_constraint_coeff(var_rank, size);
	    solver.add_constraint_geq(0);
	    var_rank++;
	}
      }
    }
  }

  // Handling packages constraints
  if (problem->all_packages->size() > 0)
    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++) {
      // Depends
      if ((*ipkg)->depends != (CUDFVpkgFormula *)NULL) {
	// Conjunction of dependencies
	for (CUDFVpkgFormulaIterator anddeps = (*ipkg)->depends->begin(); anddeps != (*ipkg)->depends->end(); anddeps++) {
	  bool self_depend = false;
	  bool has_coeff = false;
	  solver.new_constraint();
	  // Disjunction of dependencies
	  for (CUDFVpkgListIterator ordeps = (*anddeps)->begin(); ordeps != (*anddeps)->end(); ordeps++) {
	    CUDFVirtualPackage *vpackage = (*ordeps)->virtual_package;
	    a_compptr comp = get_comparator((*ordeps)->op);
	    // It depends from all the right versions of the package
	    if (vpackage->all_versions.size() > 0) {
	      for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++)
		if (comp((*jpkg)->version, (*ordeps)->version)) {
		  if ((*jpkg) == (*ipkg)) { // Then, the dependency is always checked
		    self_depend = true;
		    has_coeff = false;
		    break;
		  } else if (use_pkg(*jpkg)) {
		    has_coeff = true;
		    solver.set_constraint_coeff(*jpkg, +1);
		  }
		}
	    }
	    // as well as from all the providers 
	    if ((! self_depend) && (vpackage->providers.size() > 0)) { 
	      for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
		if ((*jpkg) == (*ipkg)) { // Then, the dependency is always checked
		  self_depend = true;
		  has_coeff = false;
		  break;
		} else if (use_pkg(*jpkg)) {
		  has_coeff = true;
		  solver.set_constraint_coeff(*jpkg, +1);
		}
	    }
	    // as well as from all the versioned providers with the right version
	    if (! self_depend) {
	      for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
		   jpkg != vpackage->versioned_providers.end(); jpkg++)
		if (self_depend)
		  break;
		else if (comp(jpkg->first, (*ordeps)->version)) {
		  for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++) 
		    if ((*kpkg) == (*ipkg)) { // Then, the dependency is always checked
		      self_depend = true;
		      has_coeff = false;
		      break;
		    } else if (use_pkg(*kpkg)) {
		      has_coeff = true;
		      solver.set_constraint_coeff(*kpkg, +1);
		    }
                }
            }
	  }
	  if (has_coeff) {
	    solver.set_constraint_coeff(*ipkg, -1);
	    solver.add_constraint_geq(0);
	  } else if (!self_depend) { // The package depends on a not available package => force it to 0
	    solver.set_constraint_coeff(*ipkg, 1);
	    solver.add_constraint_eq(0);
	  }
	}
      }
      
      // Conflicts
      if ((*ipkg)->conflicts != (CUDFVpkgList *)NULL) {
	if (generate_agregate_constraints) {
	  int nb_coeff = 0;
	  
	  solver.new_constraint();
	
	  for (CUDFVpkgListIterator ipkgop = (*ipkg)->conflicts->begin();  ipkgop != (*ipkg)->conflicts->end(); ipkgop++) {
	    CUDFVirtualPackage *vpackage = (*ipkgop)->virtual_package;
	    a_compptr comp = get_comparator((*ipkgop)->op);
	    if (vpackage->all_versions.size() > 0) {
	      // It conflicts with all the right versions of the package
	      for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++) 
		if (use_pkg(*jpkg) && ((*jpkg) != (*ipkg)) && (comp((*jpkg)->version, (*ipkgop)->version))&& (solver.get_constraint_coeff(*jpkg) == 0)) {
		  solver.set_constraint_coeff(*jpkg, -1);
		  nb_coeff++;
		}
	    }
	    // as well as with all the providers
	    if (vpackage->providers.size() > 0) {
	      for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
		if (use_pkg(*jpkg) && ((*jpkg) != (*ipkg)) && (solver.get_constraint_coeff(*jpkg) == 0)) {
		  solver.set_constraint_coeff(*jpkg, -1);
		  nb_coeff++;
		}
	    }
	    // as well as with all the versioned providers with the right version
	    for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
		 jpkg != vpackage->versioned_providers.end(); jpkg++)
	      if (comp(jpkg->first, (*ipkgop)->version))
		for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++) 
		  if (use_pkg(*kpkg) && ((*kpkg) != (*ipkg)) && (solver.get_constraint_coeff(*kpkg) == 0)) {
		    solver.set_constraint_coeff(*kpkg, -1);
		    nb_coeff++;
		  }
	  }
	  if (nb_coeff > 0) {
	    solver.set_constraint_coeff(*ipkg, -nb_coeff);
	    solver.add_constraint_geq(-nb_coeff);
	  }
	}

	if (generate_desagregate_constraints) {
	  for (CUDFVpkgListIterator ipkgop = (*ipkg)->conflicts->begin();  ipkgop != (*ipkg)->conflicts->end(); ipkgop++) {
	    CUDFVirtualPackage *vpackage = (*ipkgop)->virtual_package;
	    a_compptr comp = get_comparator((*ipkgop)->op);
	    if (vpackage->all_versions.size() > 0) {
	      // It conflicts with all the right versions of the package
	      for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++) 
		if (use_pkg(*jpkg) && ((*jpkg) != (*ipkg)) && (comp((*jpkg)->version, (*ipkgop)->version))) {
		  solver.new_constraint();
		  solver.set_constraint_coeff(*jpkg, 1);
		  solver.set_constraint_coeff(*ipkg, 1);
		  solver.add_constraint_leq(1);
		}
	    }
	    // as well as with all the providers
	    if (vpackage->providers.size() > 0) {
	      for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
		if (use_pkg(*jpkg) && ((*jpkg) != (*ipkg))) {
		  solver.new_constraint();
		  solver.set_constraint_coeff(*jpkg, 1);
		  solver.set_constraint_coeff(*ipkg, 1);
		  solver.add_constraint_leq(1);
		}
	    }
	    // as well as with all the versioned providers with the right version
	    for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
		 jpkg != vpackage->versioned_providers.end(); jpkg++)
	      if (comp(jpkg->first, (*ipkgop)->version))
		for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++) 
		  if (use_pkg(*kpkg) && ((*kpkg) != (*ipkg))) {
		    solver.new_constraint();
		    solver.set_constraint_coeff(*kpkg, 1);
		    solver.set_constraint_coeff(*ipkg, 1);
		    solver.add_constraint_leq(1);
		  }
	  }
	}
      }
      
      // Keep (if and only if installed)
      if ((*ipkg)->installed)
	switch((*ipkg)->keep) {
	case keep_none: break;
	case keep_feature: // Preserve all the provided features
	  if ((*ipkg)->provides != (CUDFVpkgList *)NULL) {
	    for (CUDFVpkgListIterator ipkgop = (*ipkg)->provides->begin(); ipkgop != (*ipkg)->provides->end(); ipkgop++) {
	      CUDFVirtualPackage *vpackage = (*ipkgop)->virtual_package;
	      a_compptr comp = get_comparator((*ipkgop)->op);
	      bool has_coeff = false;

	      solver.new_constraint();
	      if (vpackage->all_versions.size() > 0)
		for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++)
		  if (use_pkg(*jpkg) && (comp((*jpkg)->version, (*ipkgop)->version))) { has_coeff = true; solver.set_constraint_coeff(*jpkg, +1); }
	      if (vpackage->providers.size() > 0)
		for (CUDFProviderListIterator jpkg = vpackage->providers.begin(); jpkg != vpackage->providers.end(); jpkg++)
		  if (use_pkg(*jpkg) && (solver.get_constraint_coeff(*jpkg) == 0)) { has_coeff = true;  solver.set_constraint_coeff(*jpkg, +1); }
	      for (CUDFVersionedProviderListIterator jpkg = vpackage->versioned_providers.begin(); 
		   jpkg != vpackage->versioned_providers.end(); jpkg++)
		if (comp(jpkg->first, (*ipkgop)->version))
		  for (CUDFProviderListIterator kpkg = jpkg->second.begin(); kpkg != jpkg->second.end(); kpkg++) 
		    if (use_pkg(*kpkg) && (solver.get_constraint_coeff(*kpkg) == 0)) { has_coeff = true; solver.set_constraint_coeff(*kpkg, +1); }
	      if (has_coeff)
		solver.add_constraint_geq(+1);
	    }
	  }
	  break;
	case keep_package: // Preserve at least one version of the package
	  if (((*ipkg)->virtual_package->all_versions.size() > 0) && (! keep_package_handled[(*ipkg)->virtual_package->rank+1])) {
	    CUDFVirtualPackage *vpackage = (*ipkg)->virtual_package;
	    // bool has_coeff = false;

	    solver.new_constraint();
	    if (vpackage->all_versions.size() > 0)  // Should not make sense
	      for (CUDFVersionedPackageSetIterator jpkg = vpackage->all_versions.begin(); jpkg != vpackage->all_versions.end(); jpkg++)
		if (use_pkg(*jpkg)) { /*has_coeff = true;*/ solver.set_constraint_coeff(*jpkg, +1); }
	    solver.add_constraint_geq(+1);
	    keep_package_handled[(*ipkg)->virtual_package->rank+1] = true;
	  }
	  break;
	case keep_version: // Preserve the current version
	  solver.new_constraint();
	  solver.set_constraint_coeff(*ipkg, +1);
	  solver.add_constraint_eq(+1);
	  break;
	}
    }

  solver.end_add_constraints();

  return 0;
}

