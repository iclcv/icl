# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME hough-line
           SOURCES hough-line.cpp
           LIBRARIES ICLMarkers)

