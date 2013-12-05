# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME surf-based-object-tracking
          SOURCES surf-based-object-tracking.cpp
          LIBRARIES ICLMarkers)

