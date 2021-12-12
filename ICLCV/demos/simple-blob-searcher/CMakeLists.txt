# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME simple-blob-searcher
           SOURCES simple-blob-searcher.cpp
           LIBRARIES ICLMarkers)
