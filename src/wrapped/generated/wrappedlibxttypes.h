/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.0.1.14) *
 *******************************************************************/
#ifndef __wrappedlibxtTYPES_H_
#define __wrappedlibxtTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void (*vFpuipp_t)(void*, uint64_t, int64_t, void*, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(XtAddEventHandler, vFpuipp_t)

#endif // __wrappedlibxtTYPES_H_
