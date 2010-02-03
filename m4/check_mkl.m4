AC_DEFUN([ICL_CHECK_MKL],[
AC_ARG_WITH([MKL],
        [AS_HELP_STRING([--without-MKL],
                        [disable support for MKL])],
        [HAVE_MKL=FALSE],
        [ICL_NOTIFY_CHECK([MKL           ])
        ICL_WITH_ROOT([MKL],[/vol/nivision/MKL/10.2])

        HAVE_MKL=TRUE
        ICL_PUSH_FLAG_VARS

	MKL_LIB_POSTFIX=32
	MKL_LIB_POSTFIX_2=
	if [[ "$BITS" = "64" ]] ; then
	   MKL_LIB_POSTFIX=em64t
	   MKL_LIB_POSTFIX_2=_lp64
	fi
	
        ICL_EXTEND_FLAG_VARS_TMP_FOR([MKL],[lib/$MKL_LIB_POSTFIX],[include])
        
        AC_CHECK_HEADER([mkl_types.h],[],[HAVE_MKL=FALSE],[])
        AC_CHECK_HEADER([mkl_cblas.h],[],[HAVE_MKL=FALSE],[])

        AC_CHECK_LIB([mkl_core],[mkl_serv_allocate],[],[HAVE_MKL=FALSE],[-pthread -lm])
        
        ICL_POP_FLAG_VARS

        if test "$HAVE_MKL" = "TRUE" ; then


        ICL_DEF_VARS(
                [MKL],
                [-L$MKL_ROOT/lib/$MKL_LIB_POSTFIX -lmkl_intel$MKL_LIB_POSTFIX_2 -lmkl_intel_thread -lmkl_core -liomp5 -pthread],
                [-Wl,-rpath -Wl,${MKL_ROOT}/lib/$MKL_LIB_POSTFIX],
                [-I$MKL_ROOT/include],
                [-DHAVE_MKL])

        fi])
AM_CONDITIONAL([HAVE_MKL_COND],[test x$HAVE_MKL = xTRUE])
])