//
// MATLAB Compiler: 23.2 (R2023b)
// Date: Fri Mar 22 21:27:37 2024
// Arguments:
// "-B""macro_default""-W""cpplib:myquadprog""-T""link:lib""myquadprog.m""-C"
//

#ifndef myquadprog_h
#define myquadprog_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_myquadprog_C_API 
#define LIB_myquadprog_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_myquadprog_C_API 
bool MW_CALL_CONV myquadprogInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_myquadprog_C_API 
bool MW_CALL_CONV myquadprogInitialize(void);
extern LIB_myquadprog_C_API 
void MW_CALL_CONV myquadprogTerminate(void);

extern LIB_myquadprog_C_API 
void MW_CALL_CONV myquadprogPrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_myquadprog_C_API 
bool MW_CALL_CONV mlxMyquadprog(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#ifdef __cplusplus
}
#endif


/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__MINGW64__)

#ifdef EXPORTING_myquadprog
#define PUBLIC_myquadprog_CPP_API __declspec(dllexport)
#else
#define PUBLIC_myquadprog_CPP_API __declspec(dllimport)
#endif

#define LIB_myquadprog_CPP_API PUBLIC_myquadprog_CPP_API

#else

#if !defined(LIB_myquadprog_CPP_API)
#if defined(LIB_myquadprog_C_API)
#define LIB_myquadprog_CPP_API LIB_myquadprog_C_API
#else
#define LIB_myquadprog_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_myquadprog_CPP_API void MW_CALL_CONV myquadprog(int nargout, mwArray& S, mwArray& y, const mwArray& H, const mwArray& f, const mwArray& A, const mwArray& b, const mwArray& Aeq, const mwArray& beq, const mwArray& lb, const mwArray& ub);

/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */
#endif

#endif
