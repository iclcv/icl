# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME compute-relative-camera-transform
          SOURCES compute-relative-camera-transform.cpp
          LIBRARIES ICLGeom)
