INCLUDE(ICLHelperMacros)

# ---- Raytracing library ----
set(RT_SOURCES
  src/Raytracing/GeometryExtractor.cpp
  src/Raytracing/BVH.cpp
  src/Raytracing/RaytracerBackend_Cpu.cpp
  src/Raytracing/SceneRaytracer.cpp
)

set(RT_HEADERS
  src/Raytracing/RaytracerTypes.h
  src/Raytracing/RaytracerBackend.h
  src/Raytracing/GeometryExtractor.h
  src/Raytracing/BVH.h
  src/Raytracing/RaytracerBackend_Cpu.h
  src/Raytracing/SceneRaytracer.h
)

# OpenCL backend (conditional)
if(OPENCL_FOUND)
  list(APPEND RT_SOURCES src/Raytracing/RaytracerBackend_OpenCL.cpp)
  list(APPEND RT_HEADERS src/Raytracing/RaytracerBackend_OpenCL.h)
endif()

# Metal RT backend (Apple only — hardware-accelerated BVH on Apple Silicon)
if(APPLE)
  enable_language(OBJCXX)
  find_library(METAL_FRAMEWORK Metal)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  if(METAL_FRAMEWORK)
    list(APPEND RT_SOURCES
      src/Raytracing/MetalRT.mm
      src/Raytracing/RaytracerBackend_Metal.mm)
    list(APPEND RT_HEADERS
      src/Raytracing/MetalRT.h
      src/Raytracing/RaytracerBackend_Metal.h)
    set_source_files_properties(
      src/Raytracing/MetalRT.mm
      src/Raytracing/RaytracerBackend_Metal.mm
      PROPERTIES COMPILE_FLAGS "-fobjc-arc")
  endif()
endif()

add_library(ICLRaytracing SHARED ${RT_SOURCES} ${RT_HEADERS})
target_include_directories(ICLRaytracing PUBLIC src/)
target_include_directories(ICLRaytracing PRIVATE ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)
target_link_libraries(ICLRaytracing ICLGeom)
target_compile_definitions(ICLRaytracing PRIVATE ICL_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Metal RT linkage
if(APPLE AND METAL_FRAMEWORK)
  target_link_libraries(ICLRaytracing ${METAL_FRAMEWORK} ${FOUNDATION_FRAMEWORK})
  target_compile_definitions(ICLRaytracing PUBLIC ICL_HAVE_METAL=1)
endif()

# OpenMP for parallel ray tracing
find_package(OpenMP QUIET)
if(OpenMP_CXX_FOUND)
  target_link_libraries(ICLRaytracing OpenMP::OpenMP_CXX)
elseif(APPLE)
  # Apple Clang + Homebrew libomp
  execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  find_path(LIBOMP_INCLUDE_DIR omp.h HINTS ${LIBOMP_PREFIX}/include /opt/homebrew/opt/libomp/include)
  find_library(LIBOMP_LIBRARY omp HINTS ${LIBOMP_PREFIX}/lib /opt/homebrew/opt/libomp/lib)
  if(LIBOMP_INCLUDE_DIR AND LIBOMP_LIBRARY)
    message(STATUS "Using Homebrew libomp: ${LIBOMP_LIBRARY}")
    target_compile_options(ICLRaytracing PRIVATE -Xclang -fopenmp)
    target_include_directories(ICLRaytracing PRIVATE ${LIBOMP_INCLUDE_DIR})
    target_link_libraries(ICLRaytracing ${LIBOMP_LIBRARY})
  endif()
endif()

# ---- Old simple demo (standalone, no library dependency) ----
BUILD_DEMO(NAME raytracing
           SOURCES raytracing.cpp
           LIBRARIES ICLQt)

# ---- Scene raytracing demo ----
add_executable(raytracing-scene-demo demos/raytracing-scene-demo.cpp)
target_include_directories(raytracing-scene-demo PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src src/)
target_link_libraries(raytracing-scene-demo ICLRaytracing ICLQt)
set_target_properties(raytracing-scene-demo PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Physics raytracing demo (requires Bullet) ----
if(BULLET_FOUND)
  add_executable(raytracing-physics-demo demos/raytracing-physics-demo.cpp)
  target_include_directories(raytracing-physics-demo PRIVATE
    ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src src/)
  target_link_libraries(raytracing-physics-demo ICLRaytracing ICLPhysics ICLQt)
  set_target_properties(raytracing-physics-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# ---- Benchmark ----
add_executable(benchmark-raytracer demos/benchmark_rt.cpp)
target_include_directories(benchmark-raytracer PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src src/)
target_link_libraries(benchmark-raytracer ICLRaytracing ICLQt)
set_target_properties(benchmark-raytracer PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Headless test ----
add_executable(test-raytracer demos/test_rt.cpp)
target_include_directories(test-raytracer PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src src/)
target_link_libraries(test-raytracer ICLRaytracing ICLQt)
set_target_properties(test-raytracer PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
