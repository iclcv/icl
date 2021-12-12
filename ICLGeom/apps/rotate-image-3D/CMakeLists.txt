# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME rotate-image-3D
          SOURCES rotate-image-3D.cpp
          LIBRARIES ICLMarkers)
