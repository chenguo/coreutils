# memcoll_nul.m4 serial 1
dnl Copyright (C) 2002-2003, 2005-2006, 2009-2010 Free Software Foundation,
dnl Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_MEMCOLL_NUL],
[
  AC_LIBOBJ([memcoll_nul])

  dnl Prerequisites of lib/memcoll.c.
  AC_FUNC_STRCOLL
])

