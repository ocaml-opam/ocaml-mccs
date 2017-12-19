
/*******************************************************/
/* CUDF solver: cudf_reduction.h                       */
/* Interface for problem reduction                     */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/


// Problem reduction consists in computing a subproblem
// which will provide the same solution set than the initial problem.
// It can be done if and only if criteria will not imply the installation
// of uninstalled package that are not either involved in the request
// of link to the installed package dependencies

#ifndef __CUDF_REDUCTIONS
#define __CUDF_REDUCTIONS

#include <cudf.h>
#include <cudf_types.h>

extern vector<CUDFPropertiesIterator> process_properties; // set of property to process

// Do reduce the problem
extern CUDFproblem *compute_reduced_CUDF(CUDFproblem *problem);

// boolean function to test whether or not a package or a virtual package belongs to the reduced problem
inline bool use_pkg(CUDFVersionedPackage *pkg) { return (pkg->in_reduced); }
inline bool use_vpkg(CUDFVirtualPackage *vpkg) { return (vpkg->in_reduced); }

#endif

