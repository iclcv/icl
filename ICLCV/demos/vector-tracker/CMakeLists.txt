# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME vector-tracker
           SOURCES vector-tracker.cpp
           LIBRARIES ICLMarkers)

