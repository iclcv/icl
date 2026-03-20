# ---- Macro definition ----
MACRO(EXAMPLE NAME)
  SET(BINARY "${NAME}-example")
  LIST(APPEND EXAMPLES ${BINARY})
  ADD_EXECUTABLE(${BINARY} ${ARGN})
  target_include_directories(${BINARY} PRIVATE ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)
  TARGET_LINK_LIBRARIES(${BINARY} ICLMath)
ENDMACRO()

# ---- Examples ----
EXAMPLE(levenberg-marquardt
        levenberg-marquardt.cpp)

# ---- Install specifications ----
INSTALL(TARGETS ${EXAMPLES}
        RUNTIME DESTINATION share/${INSTALL_PATH_PREFIX}/examples)
