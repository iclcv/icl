# --- Debian Package Information

# Get Ubuntu release
find_program(LSB_RELEASE lsb_release)
execute_process(COMMAND ${LSB_RELEASE} -cs
    OUTPUT_VARIABLE LSB_RELEASE_CODENAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get architecture
find_program(DPKG_CMD dpkg)
execute_process(COMMAND "${DPKG_CMD}" --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

IF(${LSB_RELEASE_CODENAME} STREQUAL "trusty")
  SET(DEB_DEPENDENCIES_CORE "libjpeg8, libpng12-0, ocl-icd-libopencl1, libopenni0, libopencv-dev, libmagick++5, qtbase5-dev, qtmultimedia5-dev, libfreenect0.2, libpcl-all, libzmq1, libglew-dev, libusb-dev, libudev-dev, libavcodec-dev, libavresample1, libavdevice-dev")
  SET(DEB_DEPENDENCIES_DEV "libjpeg-dev, libpng-dev, opencl-dev, libeigen3-dev, libopenni-dev, libopencv-dev, libmagick++-dev, qtbase5-dev, qtmultimedia5-dev, libfreenect-dev, libpcl-all, libzmq-dev, libglew-dev, libusb-dev, libudev-dev, libavcodec-dev, libavresample-dev")
ELSEIF(${LSB_RELEASE_CODENAME} STREQUAL "xenial")
  SET(DEB_DEPENDENCIES "")
ELSE()
  MESSAGE(FATAL_ERROR "Cannot generate DEB packages for '${LSB_RELEASE_CODENAME}'")
ENDIF()

IF(BUILD_WITH_IPP)
  SET(DEB_DEPENDENCIES "${DEV_DEPENDENCIES}")
  SET(VERSION_SUFFIX "prop")
ENDIF(BUILD_WITH_IPP)

# Configuration of CPACK build generator
SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "icl")
SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

# Debian meta information
SET(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}${VERSION_SUFFIX}+${LSB_RELEASE_CODENAME}")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "alneuman@techfak.uni-bielefeld.de")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "ICL is a novel C++ computer-vision library developed in the neuroinformatics group of the university of Bielefeld and in CITEC. It unifies both, performance and user friendliness. ICL provides a large set of simple-to-use classes and functions to facilitate development of complex computer vision applications.")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

# Combining components into groups
set(CPACK_DEB_COMPONENT_INSTALL ON)
SET(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
SET(CPACK_COMPONENT_APPLICATIONS_GROUP tools)
SET(CPACK_COMPONENT_DEMOS_GROUP dev)
SET(CPACK_COMPONENT_DEVELOPMENT_GROUP dev)
SET(CPACK_COMPONENT_DOCUMENTATION_GROUP doc)
SET(CPACK_COMPONENT_EXAMPLES_GROUP dev)
SET(CPACK_COMPONENT_LIBRARIES_GROUP core)

# Make each group into a DEB package, set dependencies for core
# and add icl-core as a dependency if necessary
SET(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
SET(DEP_DEPENDENCY_ICL_CORE "icl-core (= ${CPACK_PACKAGE_VERSION})")
SET(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS ${DEB_DEPENDENCIES})
SET(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS ${DEP_DEPENDENCY_ICL_CORE})
SET(CPACK_DEBIAN_DOC_PACKAGE_SUGGESTS ${DEP_DEPENDENCY_ICL_CORE})
SET(CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS ${DEP_DEPENDENCY_ICL_CORE})
set(CPACK_COMPONENTS_ALL applications demos development examples libraries)

# this HAS to be the last call. Otherwise CPACK variables are not exported
INCLUDE(CPack)
