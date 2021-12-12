# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME template-matching
           SOURCES template-matching.cpp
           LIBRARIES ICLMarkers)
