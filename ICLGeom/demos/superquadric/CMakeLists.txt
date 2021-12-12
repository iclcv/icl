# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME superquadric
           SOURCES superquadric.cpp
           LIBRARIES ICLMarkers)
