AC_DEFUN([ICL_CHECK_LIBMESASR],[
AC_ARG_WITH([LIBMESASR],
        [AS_HELP_STRING([--without-LIBMESASR],
                        [disable support for libmesasr (for Swiss-Ranger Time-of-Flight cameras)])],
        [HAVE_LIBMESASR=FALSE],
        [ICL_NOTIFY_CHECK([libmesasr     ])
        ICL_WITH_ROOT([LIBMESASR],[/usr])
        
        HAVE_LIBMESASR=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(LIBMESASR,lib,include)                
        
        AC_LANG([C++])
        AC_CHECK_HEADER([libMesaSR.h],[],[HAVE_LIBMESASR=FALSE])
        AC_CHECK_LIB([mesasr],[SR_OpenUSB],[],[HAVE_LIBMESASR=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_LIBMESASR" = "TRUE" ; then
           ICL_DEF_VARS(
                [LIBMESASR],
                [-L$LIBMESASR_ROOT/lib -lmesasr],
                [-Wl,-rpath -Wl,$LIBMESASR_ROOT/lib],
                [-I$LIBMESASR_ROOT/include],
                [-DHAVE_LIBMESASR])
        fi])
AM_CONDITIONAL([HAVE_LIBMESASR_COND],[test x$HAVE_LIBMESASR = xTRUE])
])