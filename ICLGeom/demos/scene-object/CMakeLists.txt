# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME scene-object
           SOURCES scene-object.cpp
           LIBRARIES ICLMarkers)
