# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME crop
           SOURCES crop.cpp
           LIBRARIES ICLMarkers)

