# ICLFindCycles.cmake — Detect and configure Blender Cycles raytracing engine
#
# Cycles must be built manually in 3rdparty/cycles/:
#   cd 3rdparty/cycles && make update
#   cd build && cmake .. -DCMAKE_BUILD_TYPE=Release \
#     -DPYTHON_ROOT_DIR=... -DPYTHON_LIBRARY=... -DPYTHON_INCLUDE_DIR=...
#   cmake --build . -j16 --target install
#
# See 3rdparty/cycles/INSTALL_ICL.md for full instructions.
#
# Sets: BUILD_WITH_CYCLES (ON/OFF), CYCLES_ROOT, CYCLES_BUILD, CYCLES_INSTALL
# Provides: cycles_target_setup(target) function

set(CYCLES_ROOT ${CMAKE_SOURCE_DIR}/3rdparty/cycles)
set(CYCLES_BUILD ${CYCLES_ROOT}/build)
set(CYCLES_INSTALL ${CYCLES_ROOT}/install)
set(CYCLES_LIB ${CYCLES_ROOT}/lib/macos_arm64)

# cgltf.h (single-header glTF parser from Cycles' MaterialX dependency)
set(CGLTF_INCLUDE_DIR "${CYCLES_INSTALL}/../lib/macos_arm64/materialx/include/MaterialXRender/External/Cgltf")

# stb_image.h for texture decoding
set(STB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty")

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

# ---- Python (required by Cycles OSL) ----
find_package(Python3 COMPONENTS Development QUIET)
if(NOT Python3_FOUND)
  find_library(PYTHON_LIB python3.13
    HINTS /opt/homebrew/opt/python@3.13/Frameworks/Python.framework/Versions/3.13/lib)
endif()

# ---- Helper function: apply Cycles linking to a target ----
function(cycles_target_setup TARGET_NAME)
  target_include_directories(${TARGET_NAME} PRIVATE ${CYCLES_INCLUDE_DIRS})
  target_include_directories(${TARGET_NAME} SYSTEM PRIVATE ${CYCLES_SYSTEM_INCLUDE_DIRS})
  target_compile_definitions(${TARGET_NAME} PRIVATE
    ${CYCLES_DEFINITIONS}
    CYCLES_INSTALL_DIR="${CYCLES_INSTALL}"
    ICL_HAVE_CYCLES
  )
  target_link_directories(${TARGET_NAME} PUBLIC ${CYCLES_INSTALL}/lib)
  # Use $<LINK_ONLY:> to prevent Cycles libs from propagating to consumers.
  # All Cycles symbols are resolved into the target's shared library at link time.
  target_link_libraries(${TARGET_NAME}
    $<LINK_ONLY:${CYCLES_STATIC_LIBS}>
    $<LINK_ONLY:${CYCLES_DYNAMIC_LIBS}>)
  if(Python3_FOUND)
    target_link_libraries(${TARGET_NAME} $<LINK_ONLY:Python3::Python>)
  elseif(PYTHON_LIB)
    target_link_libraries(${TARGET_NAME} $<LINK_ONLY:${PYTHON_LIB}>)
  endif()
  if(APPLE)
    target_link_libraries(${TARGET_NAME} $<LINK_ONLY:
      "-framework Foundation" "-framework Metal" "-framework CoreVideo"
      "-framework Cocoa" "-framework OpenGL" "-framework IOKit"
      "-framework Carbon" "-framework Accelerate" "-framework CoreFoundation"
      z>
    )
  endif()
  set_target_properties(${TARGET_NAME} PROPERTIES
    BUILD_RPATH "${CYCLES_INSTALL}/lib"
    INSTALL_RPATH "${CYCLES_INSTALL}/lib"
  )
endfunction()
