
icl_check_external_package(QT "QtCore.framework/Headers/QtCore;QtGui.framework/Headers/QtGui;QtOpenGL.framework/Headers/QtOpenGL" "QtCore.framework/QtCore;QtGui.framework/QtGui;QtOpenGL.framework/QtOpenGL" . . /Library/Frameworks/ HAVE_QT_COND TRUE)
if(HAVE_QT_COND)
  #special definition on OS X
  add_definitions( -DQT_SHARED)
  #special linker falgs on OS X
  set(QT_ICL_FLAGS " -F/Library/Frameworks -framework QtCore -F/Library/Frameworks -framework QtGui -F/Library/Frameworks -framework QtOpenGL")
  set(CMAKE_EXE_LINKER_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} ${QT_ICL_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}  ${QT_ICL_FLAGS}")
  link_directories(/Library/Frameworks/QtOpenGL.framework/Versions/4)
  link_directories(/Library/Frameworks/QtCore.framework/Versions/4)
endif()
