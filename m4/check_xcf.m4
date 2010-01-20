AC_DEFUN([ICL_CHECK_XCF],[
AC_ARG_WITH([XCF],
        [AS_HELP_STRING([--without-XCF],
                        [disable support for XCF])],
        [HAVE_XCF=FALSE],
        [ICL_NOTIFY_CHECK([XCF           ])
        ICL_WITH_ROOT([XCF],[/vol/xcf])

        HAVE_XCF=TRUE
        ICL_PUSH_FLAG_VARS
        ICL_EXTEND_FLAG_VARS_TMP_FOR(XCF,lib/,include/)                
        
        AC_LANG([C++])

        XCF_PACKAGES="xcf xmltio Memory"
        AC_CHECK_FILE([$XCF_ROOT/lib/pkgconfig/xcf.pc],[],[HAVE_XCF=FALSE])
        AC_CHECK_FILE([$XCF_ROOT/lib/pkgconfig/xmltio.pc],[],[HAVE_XCF=FALSE])
        AC_CHECK_FILE([$XCF_ROOT/lib/pkgconfig/Memory.pc],[],[HAVE_XCF=FALSE])

        ICL_DEF_VARS_FROM_PC(XCF,$XCF_PACKAGES)

        AC_CHECK_HEADER([xcf/xcf.hpp],[],[HAVE_XCF=FALSE])
        AC_CHECK_HEADER([xmltio/xmltio.hpp],[],[HAVE_XCF=FALSE])
        AC_CHECK_LIB([xcf],[main],[],[HAVE_XCF=FALSE])
        AC_CHECK_LIB([xmltio],[main],[],[HAVE_XCF=FALSE])

        #if test "$HAVE_XCF" = "FALSE" ; then
        ICL_POP_FLAG_VARS
        #fi
        ])
AM_CONDITIONAL([HAVE_XCF_COND],[test x$HAVE_XCF = xTRUE])
])