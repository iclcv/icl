icl_check_external_package(QT "QtCore/QtCore;QtGui/QtGui;QtOpenGL/QtOpenGL" "QtCore;QtGui;QtOpenGL" lib include/qt4 /usr HAVE_QT_COND TRUE)
if(HAVE_QT_COND)
  set(QT_LIBS_l QtCore QtGui QtOpenGL)
endif()
