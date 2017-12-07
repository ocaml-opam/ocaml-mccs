
/*******************************************************/
/* CUDF solver: cudf_reduction.c                       */
/* Implementation of problem reductions                */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


#include <cudf_reductions.h>
#include <list>

vector<CUDFPropertiesIterator> process_properties; // set of property to process

// Add the virtual package of a vpkglist to the reduced problem
void add_vpkgs_from_vpkglist(list<CUDFVirtualPackage *> &lvpkg, CUDFVpkgList *vpkglist) {
  for (CUDFVpkgListIterator ivpkg = vpkglist->begin(); ivpkg != vpkglist->end(); ivpkg++) {
    CUDFVirtualPackage *vpkg = (*ivpkg)->virtual_package;
    if ((vpkg != (CUDFVirtualPackage *)NULL) && (! vpkg->in_reduced)) lvpkg.push_back(vpkg);
  }
}

// Process a vpkgformula (from a depends) adding the related virtual packages to the new problem
void add_vpkgs_from_vpkgformula(list<CUDFVirtualPackage *> &lvpkg, CUDFVpkgFormula *vpkgf) {
  for (CUDFVpkgFormulaIterator anddeps = vpkgf->begin(); anddeps != vpkgf->end(); anddeps++)
    add_vpkgs_from_vpkglist(lvpkg, *anddeps);
}


// Add a package to the reduced problem
void process_package(CUDFproblem *new_pb, list<CUDFVirtualPackage *> &lvpkg, CUDFVersionedPackage *pkg) {
  if (! pkg->in_reduced) {
    pkg->in_reduced = true;
    new_pb->all_packages->push_back(pkg);
    if (pkg->installed)
      new_pb->installed_packages->push_back(pkg);
    else
      new_pb->uninstalled_packages->push_back(pkg);
    if (! pkg->virtual_package->in_reduced) lvpkg.push_back(pkg->virtual_package);
    if (pkg->depends != (CUDFVpkgFormula *)NULL) add_vpkgs_from_vpkgformula(lvpkg, pkg->depends);
    // process properties 
    for (vector<CUDFPropertiesIterator>::iterator prop = process_properties.begin(); prop != process_properties.end(); prop++)
      for (CUDFPropertyValueListIterator propval = pkg->properties.begin();  propval != pkg->properties.end(); propval++)
	if ((*propval)->property == (*prop)->second)
	  switch((*prop)->second->type_id) {
	  case pt_vpkg: 
	  case pt_veqpkg: 
	    {
	      CUDFVirtualPackage *vpkg = (*propval)->vpkg->virtual_package;
	      if (! vpkg->in_reduced) lvpkg.push_back(vpkg);
	    }
	    break;
	  case pt_vpkglist: 
	  case pt_veqpkglist: 
	    add_vpkgs_from_vpkglist(lvpkg, (*propval)->vpkglist);
	    break;
	  case pt_vpkgformula:
	    add_vpkgs_from_vpkgformula(lvpkg, (*propval)->vpkgformula);
	    break;
	  default:
	    break;
	  }
  }
}

// process the virtual package vpkg adding its packages to the reduced problem
void process_vpackage(CUDFproblem *new_pb, list<CUDFVirtualPackage *> &lvpkg, CUDFVirtualPackage *vpkg) {
  if (! vpkg->in_reduced) {
    vpkg->in_reduced = true;
    new_pb->all_virtual_packages->push_back(vpkg);
    if (vpkg->all_versions.size() > 0) // all versions
      for (CUDFVersionedPackageSetIterator ipkg = vpkg->all_versions.begin(); ipkg != vpkg->all_versions.end(); ipkg++)
	process_package(new_pb, lvpkg, (*ipkg));
    if (vpkg->providers.size() > 0) // providers 
      for (CUDFProviderListIterator ipkg = vpkg->providers.begin(); ipkg != vpkg->providers.end(); ipkg++)
	process_package(new_pb, lvpkg, (*ipkg));
    if (vpkg->versioned_providers.size() > 0) // versioned providers
      for (CUDFVersionedProviderListIterator ipkg = vpkg->versioned_providers.begin(); ipkg != vpkg->versioned_providers.end(); ipkg++)
	for (CUDFProviderListIterator kpkg = ipkg->second.begin(); kpkg != ipkg->second.end(); kpkg++)
	  process_package(new_pb, lvpkg, (*kpkg));
  }
}

// Do compute a reduced version of the problem
CUDFproblem *compute_reduced_CUDF(CUDFproblem *problem) {
  list<CUDFVirtualPackage *> lvpkg;
  CUDFproblem *new_pb = new CUDFproblem();

  if (verbosity > 0)
    PRINT_OUT("Initial size: %" CUDFsizet"u packages (%" CUDFsizet"u installed, %" CUDFsizet"u uninstalled), %" CUDFsizet"u virtual packages\n",
	   problem->all_packages->size(), problem->installed_packages->size(), problem->uninstalled_packages->size(), 
	   problem->all_virtual_packages->size());

  new_pb->properties = problem->properties;
    new_pb->all_packages = new CUDFVersionedPackageList();
    new_pb->installed_packages = new CUDFVersionedPackageList();;
    new_pb->uninstalled_packages = new CUDFVersionedPackageList();;
    new_pb->all_virtual_packages = new CUDFVirtualPackageList();
    new_pb->install = problem->install;
    new_pb->remove = problem->remove;
    new_pb->upgrade = problem->upgrade;

    for (CUDFVersionedPackageListIterator ipkg = problem->all_packages->begin(); ipkg != problem->all_packages->end(); ipkg++)
      (*ipkg)->in_reduced = false;

    for (CUDFVirtualPackageListIterator ipkg = problem->all_virtual_packages->begin(); ipkg != problem->all_virtual_packages->end(); ipkg++)
      (*ipkg)->in_reduced = false;

    // process all virtual packages of installed versioned packages
    for (CUDFVersionedPackageListIterator ipkg = problem->installed_packages->begin(); ipkg != problem->installed_packages->end(); ipkg++)
      process_vpackage(new_pb, lvpkg, (*ipkg)->virtual_package);

    // add virtual packages from install request
    if (problem->install != (CUDFVpkgList *)NULL) add_vpkgs_from_vpkglist(lvpkg, problem->install);

    // add virtual packages from upgrade request
    if (problem->upgrade != (CUDFVpkgList *)NULL) add_vpkgs_from_vpkglist(lvpkg, problem->upgrade);

    // add virtual packages from remove request (the remove might act only on a subset of the version ...)
    // the problem only arise when the related packages are not installed in the initial configuration
    if (problem->remove != (CUDFVpkgList *)NULL) add_vpkgs_from_vpkglist(lvpkg, problem->remove);

    // process pending virtual packages
    for (list<CUDFVirtualPackage *>::iterator ivpkg = lvpkg.begin(); ivpkg != lvpkg.end(); ivpkg++)
      process_vpackage(new_pb, lvpkg, (*ivpkg));

    if (verbosity > 0)
      PRINT_OUT("Final size: %" CUDFsizet"u packages (%" CUDFsizet"u installed, %" CUDFsizet"u uninstalled), %" CUDFsizet"u virtual packages\n",
	     new_pb->all_packages->size(), new_pb->installed_packages->size(), new_pb->uninstalled_packages->size(), 
	     new_pb->all_virtual_packages->size());

    // Recompute ranks
    { 
      int rank = 0;
      for (CUDFVersionedPackageListIterator ipkg = new_pb->all_packages->begin(); ipkg != new_pb->all_packages->end(); ipkg++, rank++)
	(*ipkg)->rank = rank;
      rank = 0;
      for (CUDFVirtualPackageListIterator ivpkg = new_pb->all_virtual_packages->begin(); 
	   ivpkg != new_pb->all_virtual_packages->end(); ivpkg++, rank++)
	(*ivpkg)->rank = rank;
    }


  return new_pb;
}

