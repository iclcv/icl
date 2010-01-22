AC_DEFUN([ICL_CHECK_IPP],[
AC_ARG_WITH([IPP],
        [AS_HELP_STRING([--without-IPP],
                        [disable support for IPP])],
        [HAVE_IPP=FALSE],
        [ICL_NOTIFY_CHECK([IPP           ])
        ICL_WITH_ROOT([IPP],[/vol/nivision/IPP/6.0])

        HAVE_IPP=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR([IPP],[sharedlib],[include])
        
        AC_CHECK_HEADER([ipp.h],[],[HAVE_IPP=FALSE],[])
        AC_CHECK_LIB([ippcore],[ippGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([ippi],[ippiGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([ipps],[ippsGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([ippsr],[ippsrGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([ippcv],[ippcvGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([ippm],[ippmGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([ippcc],[ippccGetLibVersion],[],[HAVE_IPP=FALSE],[-lpthread])
        AC_CHECK_LIB([guide],[main],[],[HAVE_IPP=FALSE],[-lpthread])
        
        ICL_POP_FLAG_VARS

        if test "$HAVE_IPP" = "TRUE" ; then

        ICL_DEF_VARS(
                [IPP],
                [-L$IPP_ROOT/sharedlib -lippcore -lippi -lipps -lippsr -lippcv -lippm -lippcc -lguide -pthread -liomp5],
                [-Wl,-rpath=${IPP_ROOT}/sharedlib],
                [-I$IPP_ROOT/include],
                [-DHAVE_IPP])
        fi])
AM_CONDITIONAL([HAVE_IPP_COND],[test x$HAVE_IPP = xTRUE])
])