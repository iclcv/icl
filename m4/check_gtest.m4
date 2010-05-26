AC_DEFUN([ICL_CHECK_GTEST],[
AC_ARG_WITH([GTEST],
        [AS_HELP_STRING([--without-GTEST],
                        [disable support for Test Framework])],
        [HAVE_GTEST=FALSE],
        [ICL_NOTIFY_CHECK([libgtest-dev  ])
        ICL_WITH_ROOT([GTEST],[/usr])
        
        HAVE_GTEST=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(GTEST,lib,include)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([gtest/gtest.h],[],[HAVE_GTEST=FALSE])
        AC_CHECK_LIB([gtest],[main],[],[HAVE_GTEST=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_GTEST" = "TRUE" ; then
           ICL_DEF_VARS(
                [GTEST],
                [-L$GTEST_ROOT/lib -lgtest],
                [-Wl,-rpath -Wl,$GTEST_ROOT/lib],
                [-I$GTEST_ROOT/include],
                [-DHAVE_GTEST])
        fi])
AM_CONDITIONAL([HAVE_GTEST_COND],[test x$HAVE_GTEST = xTRUE])
])