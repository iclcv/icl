message(STATUS "in script post_install.cmake")
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/lib RESULT_VARIABLE ICL_CHMOD_LIB)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig RESULT_VARIABLE ICL_CHMOD_PKGCFG)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/bin RESULT_VARIABLE ICL_CHMOD_BIN)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/doc RESULT_VARIABLE ICL_CHMOD_DOC)


message(STATUS "result of lib: ${ICL_CHMOD_LIB}")

if(ICL_CHMOD_LIB)
  message(STATUS "unable to change permissions of ${CMAKE_INSTALL_PREFIX}/${LIBRARY_OUTPUT_PATH}")
else()
  message(STATUS "changed permissions of ${CMAKE_INSTALL_PREFIX}/${LIBRARY_OUTPUT_PATH}")
endif()


if(ICL_CHMOD_PKGCFG)
  message(STATUS "unable to change permissions of ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
else()
  message(STATUS "changed permissions of ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif()


if(ICL_CHMOD_BIN)
  message(STATUS "unable to change permissions of ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH}")
else()
  message(STATUS "changed permissions of ${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_OUTPUT_PATH}")
endif()


if(ICL_CHMOD_DOC)
  message(STATUS "unable to change permissions of ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}")
else()
  message(STATUS "changed permissions of ${CMAKE_INSTALL_PREFIX}/${DOC_DIR}")
endif()




# [args1...]]
#                  [COMMAND <cmd2> [args2...] [...]]
#                  [WORKING_DIRECTORY <directory>]
#                  [TIMEOUT <seconds>]
#                  [RESULT_VARIABLE <variable>]
#                  [OUTPUT_VARIABLE <variable>]
#                  [ERROR_VARIABLE <variable>]
#                  [INPUT_FILE <file>]
#                  [OUTPUT_FILE <file>]
#                  [ERROR_FILE <file>]
#                  [OUTPUT_QUIET]
#                  [ERROR_QUIET]
#                  [OUTPUT_STRIP_TRAILING_WHITESPACE]
#                  [ERROR_STRIP_TRAILING_WHITESPACE])