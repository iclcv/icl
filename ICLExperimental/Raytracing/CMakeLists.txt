INCLUDE(ICLHelperMacros)

# ---- Cycles configuration ----
set(CYCLES_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/cycles)
set(CYCLES_BUILD ${CYCLES_ROOT}/build)
set(CYCLES_INSTALL ${CYCLES_ROOT}/install)
set(CYCLES_LIB ${CYCLES_ROOT}/lib/macos_arm64)

# Check that Cycles is built
if(NOT EXISTS ${CYCLES_BUILD}/lib/libcycles_session.a)
  message(STATUS "Cycles not built — skipping ICLRaytracing")
  message(STATUS "  Build with: cd 3rdparty/cycles/build && cmake .. && cmake --build . -j16 --target install")
  return()
endif()

message(STATUS "Cycles found at ${CYCLES_ROOT}")

# ---- Cycles compile definitions (must match Cycles build) ----
set(CYCLES_DEFINITIONS
  -DCCL_NAMESPACE_BEGIN=namespace\ ccl\ {
  -DCCL_NAMESPACE_END=}
  -DEMBREE_MAJOR_VERSION=4
  -DOIDN_STATIC_LIB
  -DWITH_EMBREE
  -DWITH_METAL
  -DWITH_NANOVDB
  -DWITH_OCIO
  -DWITH_OPENCOLORIO
  -DWITH_OPENIMAGEDENOISE
  -DWITH_OPENSUBDIV
  -DWITH_OPENVDB
  -DWITH_OSL
  -DWITH_PUGIXML
  -DWITH_SYSTEM_PUGIXML
  -DWITH_SSE2NEON
  -DWITH_TBB
  -DWITH_USD
  -DWITH_PYTHON
  -D__TBB_show_deprecation_message_atomic_H
  -D__TBB_show_deprecation_message_task_H
)

# ---- Cycles include paths ----
set(CYCLES_INCLUDE_DIRS
  ${CYCLES_ROOT}/src
  ${CYCLES_ROOT}/third_party/atomic
  ${CYCLES_ROOT}/third_party/cuew/include
  ${CYCLES_ROOT}/third_party/hipew/include
  ${CYCLES_ROOT}/third_party/sky/include
)

set(CYCLES_SYSTEM_INCLUDE_DIRS
  ${CYCLES_LIB}/sse2neon
  ${CYCLES_LIB}/openimageio/include
  ${CYCLES_LIB}/tbb/include
  ${CYCLES_LIB}/embree/include
  ${CYCLES_LIB}/openvdb/include
  ${CYCLES_LIB}/opensubdiv/include
  ${CYCLES_LIB}/osl/include
  ${CYCLES_LIB}/imath/include
  ${CYCLES_LIB}/imath/include/Imath
  ${CYCLES_LIB}/pugixml/include
  ${CYCLES_LIB}/opencolorio/include
  ${CYCLES_LIB}/openexr/include
  ${CYCLES_LIB}/openexr/include/OpenEXR
  ${CYCLES_LIB}/openimagedenoise/include
  ${CYCLES_LIB}/usd/include
)

# ---- Cycles static libraries (link order matters) ----
set(CYCLES_STATIC_LIBS
  ${CYCLES_BUILD}/lib/libcycles_session.a
  ${CYCLES_BUILD}/lib/libcycles_scene.a
  ${CYCLES_BUILD}/lib/libcycles_integrator.a
  ${CYCLES_BUILD}/lib/libcycles_device.a
  ${CYCLES_BUILD}/lib/libcycles_kernel_cpu.a
  ${CYCLES_BUILD}/lib/libcycles_kernel_osl.a
  ${CYCLES_BUILD}/lib/libcycles_bvh.a
  ${CYCLES_BUILD}/lib/libcycles_subd.a
  ${CYCLES_BUILD}/lib/libcycles_graph.a
  ${CYCLES_BUILD}/lib/libcycles_util.a
  ${CYCLES_BUILD}/lib/libextern_sky.a
  ${CYCLES_BUILD}/lib/libextern_cuew.a
  ${CYCLES_BUILD}/lib/libextern_hipew.a
  ${CYCLES_LIB}/zstd/lib/libzstd.a
)

# ---- Cycles dynamic libraries ----
set(CYCLES_DYNAMIC_LIBS
  embree4
  OpenImageDenoise
  OpenImageIO
  OpenImageIO_Util
  OpenColorIO
  OpenEXR
  OpenEXRCore
  IlmThread
  Iex
  Imath
  tbb
  oslexec
  oslquery
  oslnoise
  oslcomp
  openvdb
  osdCPU
  osdGPU
  usd_ms
)

link_directories(${CYCLES_INSTALL}/lib)

# ---- Python (required by Cycles OSL) ----
find_package(Python3 COMPONENTS Development QUIET)
if(NOT Python3_FOUND)
  find_library(PYTHON_LIB python3.13
    HINTS /opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/lib)
endif()

# ---- Helper function for Cycles-linked targets ----
function(cycles_target_setup TARGET_NAME)
  target_include_directories(${TARGET_NAME} PRIVATE
    ${CYCLES_INCLUDE_DIRS}
  )
  target_include_directories(${TARGET_NAME} SYSTEM PRIVATE
    ${CYCLES_SYSTEM_INCLUDE_DIRS}
  )
  target_compile_definitions(${TARGET_NAME} PRIVATE
    ${CYCLES_DEFINITIONS}
    CYCLES_INSTALL_DIR="${CYCLES_INSTALL}"
  )
  target_link_libraries(${TARGET_NAME}
    ${CYCLES_STATIC_LIBS}
    ${CYCLES_DYNAMIC_LIBS}
  )
  if(Python3_FOUND)
    target_link_libraries(${TARGET_NAME} Python3::Python)
  elseif(PYTHON_LIB)
    target_link_libraries(${TARGET_NAME} ${PYTHON_LIB})
  endif()
  if(APPLE)
    target_link_libraries(${TARGET_NAME}
      "-framework Foundation"
      "-framework Metal"
      "-framework CoreVideo"
      "-framework Cocoa"
      "-framework OpenGL"
      "-framework IOKit"
      "-framework Carbon"
      "-framework Accelerate"
      "-framework CoreFoundation"
      z
    )
  endif()
  set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    BUILD_RPATH "${CYCLES_INSTALL}/lib"
    INSTALL_RPATH "${CYCLES_INSTALL}/lib"
  )
endfunction()

# ---- ICLRaytracing library (Cycles-powered) ----
set(RT_SOURCES
  src/Raytracing/CyclesRenderer.cpp
  src/Raytracing/SceneSynchronizer.cpp
  src/Raytracing/GltfLoader.cpp
)
set(RT_HEADERS
  src/Raytracing/CyclesRenderer.h
  src/Raytracing/SceneSynchronizer.h
  src/Raytracing/GltfLoader.h
)

# cgltf.h location (single-header glTF parser from Cycles' MaterialX dependency)
set(CGLTF_INCLUDE_DIR "${CYCLES_INSTALL}/../lib/macos_arm64/materialx/include/MaterialXRender/External/Cgltf")

add_library(ICLRaytracing SHARED ${RT_SOURCES} ${RT_HEADERS})
target_include_directories(ICLRaytracing PUBLIC src/)
target_include_directories(ICLRaytracing PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src
  ${CGLTF_INCLUDE_DIR}
)
target_link_libraries(ICLRaytracing ICLGeom)
cycles_target_setup(ICLRaytracing)

# ---- CyclesRenderer test (Phase 3) ----
add_executable(cycles-renderer-test demos/cycles-renderer-test.cpp)
target_include_directories(cycles-renderer-test PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src
  src/
)
target_link_libraries(cycles-renderer-test ICLRaytracing ICLIO)
set_target_properties(cycles-renderer-test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Physics demo (requires Bullet + Qt) ----
if(BULLET_FOUND)
  add_executable(cycles-physics-demo demos/cycles-physics-demo.cpp)
  target_include_directories(cycles-physics-demo PRIVATE
    ${ICL_MODULE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/src
    src/
  )
  target_link_libraries(cycles-physics-demo ICLRaytracing ICLPhysics ICLQt)
  set_target_properties(cycles-physics-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# ---- Scene viewer (OBJ loader + Cycles) ----
add_executable(cycles-scene-viewer demos/cycles-scene-viewer.cpp)
target_include_directories(cycles-scene-viewer PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src
  src/
)
target_link_libraries(cycles-scene-viewer ICLRaytracing ICLQt)
set_target_properties(cycles-scene-viewer PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Cycles link test (Phase 1) ----
add_executable(cycles-link-test demos/cycles-link-test.cpp)
target_include_directories(cycles-link-test PRIVATE
  ${ICL_MODULE_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/src
)
target_link_libraries(cycles-link-test ICLIO)
cycles_target_setup(cycles-link-test)
