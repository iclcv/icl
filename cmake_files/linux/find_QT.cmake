icl_check_external_package(QT "QtCore/QtCore;QtGui/QtGui;QtOpenGL/QtOpenGL" "QtCore;QtGui;QtOpenGL" lib include/qt4 FALSE FALSE)

find_program(QT_MOC_EXECUTABLE moc-qt4 "${ICL_XDEP_QT_PATH}/bin" DOC "location of Qt's meta object compiler")
if(NOT "${QT_MOC_EXECUTABLE}" STREQUAL "QT_MOC_EXECUTABLE-NOTFOUND") 
  message(STATUS "found (binary): moc-qt4")
else()
  message(STATUS "not found (binary): moc-qt4")
  set(HAVE_QT_COND FALSE)
endif()

if(HAVE_QT_COND)
  message(STATUS "QT detected: TRUE")
else()
  message(STATUS "QT detected: FALSE")
endif()

