message(STATUS "executing ICL/scripts/post_install.cmake")
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/lib RESULT_VARIABLE ICL_CHMOD_LIB)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig RESULT_VARIABLE ICL_CHMOD_PKGCFG)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/bin RESULT_VARIABLE ICL_CHMOD_BIN)
execute_process(COMMAND chmod -f g+w ${CMAKE_INSTALL_PREFIX}/doc RESULT_VARIABLE ICL_CHMOD_DOC)

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

#todo: make the script to adapt all installed files!