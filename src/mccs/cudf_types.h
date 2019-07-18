
/*******************************************************/
/* CUDF solver: cudf_types.h                           */
/* Common types to handle CUDF problems                */
/* (c) Claude Michel I3S (UNSA-CNRS) 2009,2010,2011    */
/*******************************************************/

// Defines common types for cudf handling

#ifndef _CUDF_TYPES__
#define _CUDF_TYPES__


typedef long long int CUDFcoefficient;  // type of coefficients
#if !defined(_MSC_VER) || _MSC_VER >= 1900
#define CUDFabs llabs                   // absolute value of a coefficient
#define CUDFnearbyint nearbyint
#else
#define CUDFabs _abs64
#define CUDFnearbyint(e) ((int)floor((e) + 0.5))
#endif

#ifdef _WIN32
// See note in cudf.h about __MINGW_USE_ANSI_STDIO. This check ensures that the
// headers have been set correctly, as mingw printf is required, if it is
// available.
#if defined(__cplusplus) && defined(__MINGW32__) && !defined(__MINGW_PRINTF_FORMAT)
#error "Headers not included correctly - __MINGW_USE_ANSI_STDIO missing"
#endif

#if __USE_MINGW_ANSI_STDIO
#define CUDFint64 "ll"
#else
#define CUDFint64 "I64"
#endif

#if _MSC_VER >= 1800 || defined(__MINGW_PRINTF_FORMAT)
#define CUDFsizet "z"
#else
#define CUDFsizet "I"
#endif
#else

#define CUDFint64 "ll"
#define CUDFsizet "z"

#endif


#endif

#define CUDFflags "%" CUDFint64 "d"
#define CUDFflagsplus "%+" CUDFint64 "d"
