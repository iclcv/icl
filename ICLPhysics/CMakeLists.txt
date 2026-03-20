file(GLOB SOURCES src/ICLPhysics/*.cpp)
file(GLOB HEADERS src/ICLPhysics/*.h)

icl_add_module(ICLPhysics
  SOURCES ${SOURCES}
  HEADERS ${HEADERS}
  ICL_DEPS ICLMarkers
  3RDPARTY_LIBS ${ICLPhysics_3RDPARTY_LIBRARIES}
  PKGCONFIG_DEPS ICLGeom-${SO_VERSION}
  HAS_DEMOS HAS_APPS)

# Bullet needs its include dir added separately (may have non-standard layout)
if(BULLET_INCLUDE_DIRS)
  target_include_directories(ICLPhysics PRIVATE "${BULLET_ROOT_DIR}/${BULLET_INCLUDE_DIRS}")
endif()
