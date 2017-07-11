
/*******************************************************/
/* CUDF solver: cudf_types.h                           */
/* Common types to handle CUDF problems                */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// Defines common types for cudf handling

#ifndef _CUDF_TYPES__
#define _CUDF_TYPES_


typedef long long int CUDFcoefficient;  // type of coefficients
#define CUDFabs llabs                   // absolute value of a coefficient
#define CUDFflags "%lld"                // flags to print a coefficient
#define CUDFflagsplus "%+lld"           // flags to print a coefficient with its sign


#endif
