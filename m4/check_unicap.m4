AC_DEFUN([ICL_CHECK_UNICAP],[
AC_ARG_WITH([UNICAP],
        [AS_HELP_STRING([--without-UNICAP],
                        [disable support for UNICAP])],
        [HAVE_UNICAP=FALSE],
        [ICL_NOTIFY_CHECK([Unicap        ])
        ICL_WITH_ROOT([UNICAP],[/usr])
        HAVE_UNICAP=TRUE

        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(UNICAP,lib,include/unicap)                
        
        AC_CHECK_HEADER([unicap.h],[],[HAVE_UNICAP=FALSE])
        AC_CHECK_LIB([unicap],[unicap_check_version],[],[HAVE_UNICAP=FALSE])

        ICL_POP_FLAG_VARS

        if test "$HAVE_UNICAP" = "TRUE" ; then
           ICL_DEF_VARS_FROM_PC([UNICAP],[libunicap])
           ICL_UNICAP_CXXCPP="$ICL_UNICAP_CXXCPP -DUNICAP_FLAGS_NOT_AS_ENUM"
           ICL_UNICAP_CXXFLAGS="$ICL_UNICAP_CXXFLAGS -DUNICAP_FLAGS_NOT_AS_ENUM"
        fi])
AM_CONDITIONAL([HAVE_UNICAP_COND],[test x$HAVE_UNICAP = xTRUE])
])