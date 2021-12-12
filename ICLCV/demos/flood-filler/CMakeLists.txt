# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME flood-filler
           SOURCES flood-filler.cpp
           LIBRARIES ICLMarkers)
