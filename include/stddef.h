 /*  stddef.h

    Definitions for common types, and NULL

    Copyright (c) 1987, 1991 by Borland International
    All Rights Reserved.
*/

#ifndef __STDDEF_H
#define __STDDEF_H

#if !defined( __DEFS_H )
#if !defined( __DEFS_H )
#define __DEFS_H

#if __STDC__
#  define _Cdecl
#else
#  define _Cdecl  cdecl
#endif

#ifndef __PAS__
#  define _CType _Cdecl
#else
#  define _CType pascal
#endif

#if !__STDC__
#  if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#    define _CLASSTYPE  near
#  elif defined(__COMPACT__) || defined(__LARGE__)
#    define _CLASSTYPE  far
#  else
#    define _CLASSTYPE  huge
#  endif
#endif

#define _FAR
#define _FARFUNC

#if defined( __cplusplus )
#  define _PTRDEF(name) typedef name * P##name;
#  define _REFDEF(name) typedef name & R##name;
#  define _REFPTRDEF(name) typedef name * & RP##name;
#  define _PTRCONSTDEF(name) typedef const name * PC##name;
#  define _REFCONSTDEF(name) typedef const name & RC##name;
#  define _CLASSDEF(name) class _CLASSTYPE name; \
    _PTRDEF(name) \
    _REFDEF(name) \
    _REFPTRDEF(name) \
    _PTRCONSTDEF(name) \
    _REFCONSTDEF(name)
#endif

#endif  /* __DEFS_H */
#endif

#ifndef NULL
#ifndef NULL
#  if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#      define NULL    0
#  else
#      define NULL    0L
#  endif
#endif
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
#if     defined(__LARGE__) || defined(__HUGE__) || defined(__COMPACT__)
typedef long    ptrdiff_t;
#else
typedef int     ptrdiff_t;
#endif
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#define offsetof( s_name, m_name )  (size_t)&(((s_name *)0)->m_name)

#ifndef _WCHAR_T
#define _WCHAR_T
typedef char wchar_t;
#endif

#endif  /* __STDDEF_H */