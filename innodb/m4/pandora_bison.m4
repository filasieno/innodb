dnl Copyright (C) 2010 Monty Taylor
dnl Copyright (C) 2010 Hartmut Holzgraefe
dnl This file is free software; Monty Taylor and Hartmut Holzgraefe
dnl give unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([_PANDORA_SEARCH_BISON],[

  dnl --------------------------------------------------------------------
  dnl  Check for bison
  dnl --------------------------------------------------------------------

  AC_CHECK_PROGS([YACC], ['bison -y'], [:])
  AS_IF([test "x$YACC" = "x:"],[
    pandora_IB_HAVE_bison=no
    YACC='if test -f "$@"; then echo "WARNING: no proper bison binary found, ignoring changes to $<"; exit 0; else echo "ERROR: no proper bison binary found"; exit 1; fi;'
    ],[
    pandora_IB_HAVE_bison=yes
    ])

  AM_CONDITIONAL(IB_HAVE_BISON, [test "x${pandora_IB_HAVE_bison}" = "xyes"])
])

AC_DEFUN([PANDORA_IB_HAVE_BISON],[
  AC_REQUIRE([_PANDORA_SEARCH_BISON])
])

AC_DEFUN([PANDORA_REQUIRE_BISON],[
  AC_REQUIRE([PANDORA_IB_HAVE_BISON])
  AS_IF([test "x${pandora_IB_HAVE_bison}" = "xno" -a "$pandora_building_from_bzr" = "yes"],
      AC_MSG_ERROR(["bison is required for ${PACKAGE} to build from a bzr branch"])
      )
])
