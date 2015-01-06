IF(QT4_FOUND)
  ADD_SUBDIRECTORY(rsb)
  ADD_SUBDIRECTORY(undistortion)

# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
#BUILD_DEMO(NAME shared-memory
#           SOURCES shared-memory.cpp
#           LIBRARIES ICLIO)


ADD_SUBDIRECTORY(png_write_test)

ENDIF()

