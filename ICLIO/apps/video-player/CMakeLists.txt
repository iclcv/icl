# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME video-player
          SOURCES video-player.cpp
          LIBRARIES ICLMarkers)
