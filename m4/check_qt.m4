AC_DEFUN([ICL_CHECK_QT],[
AC_ARG_WITH([QT],
        [AS_HELP_STRING([--without-QT],
                        [disable support for QT])],
        [HAVE_QT=FALSE],
        [ICL_NOTIFY_CHECK([Qt            ])
        ICL_WITH_ROOT([QT],[/usr])
        
        HAVE_QT=TRUE
        ICL_PUSH_FLAG_VARS


#       ICL_EXTEND_FLAG_VARS_TMP_FOR(QT,lib/,include/qt4)
#       ICL_EXTEND_FLAG_VARS_TMP_FOR(QT,lib64/qt4,include/qt4)

        AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtCore.pc],[],[HAVE_QT=FALSE])
        AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtOpenGL.pc],[],[HAVE_QT=FALSE])
        AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtGui.pc],[],[HAVE_QT=FALSE])
        AC_CHECK_FILE([$QT_ROOT/lib/pkgconfig/QtXml.pc],[],[HAVE_QT=FALSE])

        QT_PACKAGES="QtCore QtOpenGL QtGui QtXml"
	ICL_EXTEND_FLAG_VARS_TMP_FROM_PC_FOR($QT_PACKAGES)

        # todo check this extra -lGLU
        ICL_DEF_VARS_FROM_PC([QT],[$QT_PACKAGES])
        ICL_QT_LIBS="$ICL_QT_LIBS -lGLU"
        CPPFLAGS="$CPPFLAGS $ICL_QT_CXXFLAGS"
        LIBS="$LIBS -lGLU"
        QT_MOC=`pkg-config --variable=moc_location QtCore`
        AC_SUBST([QT_MOC])

        AC_CHECK_LIB([GLU],[gluLookAt],[],[HAVE_QT=FALSE])
        AC_CHECK_HEADER([GL/glu.h],[],[HAVE_QT=FALSE])

        AC_CHECK_HEADER([QtCore],[],[HAVE_QT=FALSE])
        AC_CHECK_HEADER([QtGui],[],[HAVE_QT=FALSE])
        AC_CHECK_HEADER([QtOpenGL],[],[HAVE_QT=FALSE])
        AC_CHECK_HEADER([QtXml],[],[HAVE_QT=FALSE])
        AC_CHECK_LIB([QtCore],[main],[],[HAVE_QT=FALSE])
        AC_CHECK_LIB([QtGui],[main],[],[HAVE_QT=FALSE])
        AC_CHECK_LIB([QtOpenGL],[main],[],[HAVE_QT=FALSE])
        AC_CHECK_LIB([QtXml],[main],[],[HAVE_QT=FALSE])

#        if test "$HAVE_QT" = "FALSE" ; then
           ICL_POP_FLAG_VARS  
 #       fi
       ])
AM_CONDITIONAL([HAVE_QT_COND],[test x$HAVE_QT = xTRUE])
])