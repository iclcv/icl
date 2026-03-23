# icl-tests: links all ICL modules to collect all registered tests

set(_TEST_SOURCES icl-tests.cpp)
set(_TEST_LIBS ICLUtils ICLMath ICLCore ICLFilter)

if(TARGET ICLIO)
  list(APPEND _TEST_LIBS ICLIO)
endif()
if(TARGET ICLCV)
  list(APPEND _TEST_LIBS ICLCV)
endif()
if(TARGET ICLQt)
  list(APPEND _TEST_LIBS ICLQt)
endif()
if(TARGET ICLGeom)
  list(APPEND _TEST_LIBS ICLGeom)
endif()
if(TARGET ICLMarkers)
  list(APPEND _TEST_LIBS ICLMarkers)
endif()
if(TARGET ICLPhysics)
  list(APPEND _TEST_LIBS ICLPhysics)
endif()

# Test source files per module (add as modules get tests)
list(APPEND _TEST_SOURCES test-utils.cpp)

add_executable(icl-tests ${_TEST_SOURCES})
target_include_directories(icl-tests PRIVATE ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)
target_link_libraries(icl-tests ${_TEST_LIBS})

install(TARGETS icl-tests
  COMPONENT tools
  RUNTIME DESTINATION bin)

# Register with ctest so `ctest` or `cmake --build . --target test` works
add_test(NAME icl-tests COMMAND icl-tests)
