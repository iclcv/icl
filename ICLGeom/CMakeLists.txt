file(GLOB SOURCES src/ICLGeom/*.cpp)
file(GLOB HEADERS src/ICLGeom/*.h)

# Remove dead RSB backends
foreach(_dead RSBPointCloudGrabber RSBPointCloudSender ProtoBufSerializationDevice)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/${_dead}.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/${_dead}.h)
endforeach()

# Many ICLGeom sources require Qt (Scene, SceneObject, visualization, point cloud objects)
if(NOT QT_FOUND)
  # Keep only the core geometry files that don't need Qt
  file(GLOB _QT_SOURCES
    src/ICLGeom/Scene*.cpp src/ICLGeom/Primitive.cpp src/ICLGeom/GridSceneObject.cpp
    src/ICLGeom/ComplexCoordinateFrameSceneObject.cpp src/ICLGeom/Segmentation3D.cpp
    src/ICLGeom/DepthCameraPointCloudGrabber.cpp src/ICLGeom/PointCloud*.cpp
    src/ICLGeom/GenericPointCloud*.cpp src/ICLGeom/OctreeObject.cpp
    src/ICLGeom/RayCastOctreeObject.cpp src/ICLGeom/SQFitter.cpp
    src/ICLGeom/PlotWidget3D.cpp src/ICLGeom/ShaderUtil.cpp
    src/ICLGeom/EuclideanBlobSegmenter.cpp src/ICLGeom/FeatureGraphSegmenter.cpp
    src/ICLGeom/ConfigurableDepthImageSegmenter.cpp src/ICLGeom/PCDFileGrabber.cpp
    src/ICLGeom/Primitive3DFilter.cpp src/ICLGeom/IterativeClosestPoint*.cpp
    src/ICLGeom/PCLPointCloudObject.cpp)
  foreach(_f ${_QT_SOURCES})
    list(REMOVE_ITEM SOURCES ${_f})
  endforeach()
endif()

if(NOT PCL_FOUND OR NOT QT_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/PCLPointCloudObject.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/PCLPointCloudObject.h)
endif()

if(NOT OPENCL_FOUND OR NOT QT_FOUND)
  list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/PointCloudCreatorCL.cpp
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/IterativeClosestPoint.cpp)
  list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/PointCloudCreatorCL.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/IterativeClosestPoint.h
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/IterativeClosestPointCLCode.h)
endif()

# SceneSynchronizer.h is internal (forward-declares ccl types) — never install
list(REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/SceneSynchronizer.h)

# Raytracing (Cycles) — optional, requires manual build in 3rdparty/cycles/
if(NOT BUILD_WITH_CYCLES OR NOT QT_FOUND)
  list(REMOVE_ITEM SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/CyclesRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/SceneSynchronizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/GltfLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/SceneSetup.cpp)
  list(REMOVE_ITEM HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/CyclesRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/SceneSynchronizer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/GltfLoader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICLGeom/SceneSetup.h)
endif()

# Determine primary dependency
if(QT_FOUND)
  set(_DEP ICLQt)
  set(_PKGDEP ICLQt-${SO_VERSION})
else()
  set(_DEP ICLCV)
  set(_PKGDEP ICLCV-${SO_VERSION})
endif()

set(_PCH_HEADERS
  <vector> <string> <map> <algorithm> <functional>
  <iostream> <sstream> <memory> <cmath> <type_traits>
  <ICLUtils/CompatMacros.h>
  <ICLUtils/BasicTypes.h>
  <ICLUtils/Exception.h>
  <ICLUtils/StringUtils.h>
  <ICLUtils/Macros.h>
  <ICLCore/Img.h>
  <ICLCore/ImgBase.h>)
if(QT_FOUND)
  list(APPEND _PCH_HEADERS <QWidget> <QString> <QObject> <QOpenGLWidget>)
endif()

icl_add_module(ICLGeom
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ${_DEP}
  3RDPARTY_LIBS ${ICLGeom_3RDPARTY_LIBRARIES}
  PKGCONFIG_DEPS ${_PKGDEP}
  HAS_DEMOS HAS_APPS
  PCH_HEADERS ${_PCH_HEADERS})

# Suppress Qt #warning about GLEW incompatibility (Scene.cpp includes QOpenGLContext after GLEW)
if(TARGET ICLGeom AND QT_FOUND)
  target_compile_options(ICLGeom PRIVATE -Wno-\#warnings)
endif()

# Apply Cycles linking to ICLGeom when available
if(BUILD_WITH_CYCLES AND QT_FOUND AND TARGET ICLGeom)
  include(ICLFindCycles)
  cycles_target_setup(ICLGeom)
  target_include_directories(ICLGeom PRIVATE ${CGLTF_INCLUDE_DIR} ${STB_INCLUDE_DIR})
endif()
