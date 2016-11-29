///////////////////////////////////////////////////////////////
//
// kdef: Vienna kernel definitions
//
//
//   25.08.2010 19:43 - created
//

#ifndef _KDEF_H_INC_
#define _KDEF_H_INC_


// paste tiny unicode helper
#define UNICODE
#include "tunic.h"


#ifdef __cplusplus
#define EXTERN_C            extern "C"          // simple definition
#define EXTERN_C_BEGIN      extern "C" {        // block definition
#define EXTERN_C_END        }
#else //__cplusplus
#define EXTERN_C            
#define EXTERN_C_BEGIN      
#define EXTERN_C_END        
#endif //__cplusplus


#define INLINE  inline

#define KPI     __stdcall       // kernel program interface
#define KPIV    __cdecl         // k.p.i. (vararg)
//DISABLED: use .def file instead
//#define EXPORT  __declspec(dllexport)
//#define IMPORT  __declspec(dllimport)
#define NAKED   __declspec(naked)   // for pure-asm, no-args, asm-ret functions
#define ASM     __asm               // do not use ASM{ret} without NAKED [or use ASM{retn}]


// internal types (abstraction; do not use directly)
#define SCHAR       signed char
#define SSHORT      signed short int
#define SLONG       signed long int
#define SQUAD       signed __int64      //ms-specific
#define UCHAR       unsigned char
#define USHORT      unsigned short int
#define ULONG       unsigned long int
#define UQUAD       unsigned __int64    //ms-specific
// public types
#define BYTE        UCHAR
#define WORD        USHORT
#define DWORD       ULONG
#define QWORD       UQUAD
#define CHAR        SCHAR
#define SHORT       SSHORT
//DISABLED: use INT instead
//#define LONG        SLONG
#define INT         SLONG
#define LLONG       SQUAD   // better name?
#define BOOL        ULONG

// null
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif //__cplusplus

// boolean
#define FALSE   0
#define TRUE    1
// check boolean conditions
#if ((FALSE == FALSE) != TRUE)
#error boolean is not supported
#endif

// function arguments attributes (human only)
#define IN
#define OUT
#define INOUT
#define OPTIONAL    // can be NULL


#endif //_KDEF_H_INC_
