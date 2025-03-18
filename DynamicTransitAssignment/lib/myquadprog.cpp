//
// MATLAB Compiler: 23.2 (R2023b)
// Date: Fri Mar 22 21:27:37 2024
// Arguments:
// "-B""macro_default""-W""cpplib:myquadprog""-T""link:lib""myquadprog.m""-C"
//

#define EXPORTING_myquadprog 1
#include "myquadprog.h"

static HMCRINSTANCE _mcr_inst = NULL; /* don't use nullptr; this may be either C or C++ */

#if defined( _MSC_VER) || defined(__LCC__) || defined(__MINGW64__)
#ifdef __LCC__
#undef EXTERN_C
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>
#undef interface

static char path_to_dll[_MAX_PATH];

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, void *pv)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (GetModuleFileName(hInstance, path_to_dll, _MAX_PATH) == 0)
            return FALSE;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;
}
#endif
#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

static int mclDefaultPrintHandler(const char *s)
{
    return mclWrite(1 /* stdout */, s, sizeof(char)*strlen(s));
}

#ifdef __cplusplus
} /* End extern C block */
#endif

#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

static int mclDefaultErrorHandler(const char *s)
{
    int written = 0;
    size_t len = 0;
    len = strlen(s);
    written = mclWrite(2 /* stderr */, s, sizeof(char)*len);
    if (len > 0 && s[ len-1 ] != '\n')
        written += mclWrite(2 /* stderr */, "\n", sizeof(char));
    return written;
}

#ifdef __cplusplus
} /* End extern C block */
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_myquadprog_C_API
#define LIB_myquadprog_C_API /* No special import/export declaration */
#endif

LIB_myquadprog_C_API 
bool MW_CALL_CONV myquadprogInitializeWithHandlers(
    mclOutputHandlerFcn error_handler,
    mclOutputHandlerFcn print_handler)
{
    int bResult = 0;
    if (_mcr_inst)
        return true;
    if (!mclmcrInitialize())
        return false;
    if (!GetModuleFileName(GetModuleHandle("myquadprog"), path_to_dll, _MAX_PATH))
        return false;
    bResult = mclInitializeComponentInstanceNonEmbeddedStandalone(&_mcr_inst,
        path_to_dll,
        "myquadprog",
        LibTarget,
        error_handler, 
        print_handler,
        NULL);
    if (!bResult)
    return false;
    return true;
}

LIB_myquadprog_C_API 
bool MW_CALL_CONV myquadprogInitialize(void)
{
    return myquadprogInitializeWithHandlers(mclDefaultErrorHandler, 
                                          mclDefaultPrintHandler);
}

LIB_myquadprog_C_API 
void MW_CALL_CONV myquadprogTerminate(void)
{
    if (_mcr_inst)
        mclTerminateInstance(&_mcr_inst);
}

LIB_myquadprog_C_API 
void MW_CALL_CONV myquadprogPrintStackTrace(void) 
{
    char** stackTrace;
    int stackDepth = mclGetStackTrace(&stackTrace);
    int i;
    for(i=0; i<stackDepth; i++)
    {
        mclWrite(2 /* stderr */, stackTrace[i], sizeof(char)*strlen(stackTrace[i]));
        mclWrite(2 /* stderr */, "\n", sizeof(char)*strlen("\n"));
    }
    mclFreeStackTrace(&stackTrace, stackDepth);
}


LIB_myquadprog_C_API 
bool MW_CALL_CONV mlxMyquadprog(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[])
{
    return mclFeval(_mcr_inst, "myquadprog", nlhs, plhs, nrhs, prhs);
}

LIB_myquadprog_CPP_API 
void MW_CALL_CONV myquadprog(int nargout, mwArray& S, mwArray& y, const mwArray& H, const 
                             mwArray& f, const mwArray& A, const mwArray& b, const 
                             mwArray& Aeq, const mwArray& beq, const mwArray& lb, const 
                             mwArray& ub)
{
    mclcppMlfFeval(_mcr_inst, "myquadprog", nargout, 2, 8, &S, &y, &H, &f, &A, &b, &Aeq, &beq, &lb, &ub);
}

