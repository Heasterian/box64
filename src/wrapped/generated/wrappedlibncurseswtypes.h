/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.0.1.14) *
 *******************************************************************/
#ifndef __wrappedlibncurseswTYPES_H_
#define __wrappedlibncurseswTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef int64_t (*iFpV_t)(void*, void*);
typedef int64_t (*iFppp_t)(void*, void*, void*);
typedef int64_t (*iFpiipV_t)(void*, int64_t, int64_t, void*, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(printw, iFpV_t) \
	GO(vwprintw, iFppp_t) \
	GO(mvwprintw, iFpiipV_t)

#endif // __wrappedlibncurseswTYPES_H_
