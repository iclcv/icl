# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME polynomial-regression
           SOURCES polynomial-regression.cpp
           LIBRARIES ICLMarkers)

