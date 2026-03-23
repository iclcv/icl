# icl-benchmarks: links all ICL modules to collect all registered benchmarks

set(_BENCH_SOURCES icl-benchmarks.cpp)
set(_BENCH_LIBS ICLUtils ICLMath ICLCore ICLFilter)

# Add benchmark sources and libs for higher-level modules if available
if(TARGET ICLIO)
  list(APPEND _BENCH_LIBS ICLIO)
endif()
if(TARGET ICLCV)
  list(APPEND _BENCH_SOURCES bench-cv.cpp)
  list(APPEND _BENCH_LIBS ICLCV)
endif()
if(TARGET ICLQt)
  list(APPEND _BENCH_LIBS ICLQt)
endif()
if(TARGET ICLGeom)
  list(APPEND _BENCH_LIBS ICLGeom)
endif()
if(TARGET ICLMarkers)
  list(APPEND _BENCH_LIBS ICLMarkers)
endif()
if(TARGET ICLPhysics)
  list(APPEND _BENCH_LIBS ICLPhysics)
endif()

# Core and filter benchmarks are always available
list(APPEND _BENCH_SOURCES bench-core.cpp bench-filter.cpp)

add_executable(icl-benchmarks ${_BENCH_SOURCES})
target_include_directories(icl-benchmarks PRIVATE ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)
target_link_libraries(icl-benchmarks ${_BENCH_LIBS})

install(TARGETS icl-benchmarks
  COMPONENT tools
  RUNTIME DESTINATION bin)
