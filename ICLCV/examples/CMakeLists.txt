# ---- Macro definition ----
MACRO(EXAMPLE NAME)
  SET(BINARY "${NAME}-example")
  LIST(APPEND EXAMPLES ${BINARY})
  ADD_EXECUTABLE(${BINARY} ${ARGN})
  TARGET_LINK_LIBRARIES(${BINARY} ICLCV)
ENDMACRO()

# ---- Examples ----
IF(QT4_FOUND)
  EXAMPLE(quick
    quick.cpp)
ENDIF()

# ---- Install specifications ----
INSTALL(TARGETS ${EXAMPLES}
        RUNTIME DESTINATION share/${INSTALL_PATH_PREFIX}/examples)
