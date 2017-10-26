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
    OUTPUT_VARIABLE ICL_DEB_PACKAGE_ARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# cmake_host_system_information(RESULT ICL_DEB_NUM_CORES
#                               QUERY NUMBER_OF_LOGICAL_CORES)

SET(ICL_DEB_PACKAGE_MAINTAINER "Alexander Neumann <alneuman@techfak.uni-bielefeld.de>")

SET(ICL_DEB_PACKAGE_DESCRIPTION_SHORT "Image Component Library")
SET(ICL_DEB_PACKAGE_DESCRIPTION "ICL is a novel C++ computer-vision library developed in the neuroinformatics group of the university of Bielefeld and in CITEC. It unifies both, performance and user friendliness. ICL provides a large set of simple-to-use classes and functions to facilitate development of complex computer vision applications.")

SET(ICL_DEB_PACKAGE_SECTION "devel")
SET(ICL_DEB_PACKAGE_PRIORITY "optional")

# Dependencies for building ICL
SET(ICL_DEB_BUILD_DEPENDENCIES "cmake3 | cmake, libjpeg-dev, libpng-dev, ocl-icd-opencl-dev, libeigen3-dev, libopenni-dev, libopencv-dev, libmagick++-dev, qtbase5-dev, qtmultimedia5-dev, libqt5opengl5-dev, libfreenect-dev, libpcl-all | libpcl-dev, libzmq-dev, libglew-dev, libusb-dev, libavcodec-dev, libavresample-dev, libavdevice-dev, doxygen, python-sphinx, python-pyparsing")

# If IPP is specified, add it to dependencies and add version suffix
IF(BUILD_WITH_IPP)
  SET(DEB_DEPENDENCIES "${DEV_DEPENDENCIES}")
  SET(VERSION_SUFFIX "prop")
ENDIF(BUILD_WITH_IPP)


# --- Preparing Debian folder for source packge built

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/debian
	 DESTINATION ${PROJECT_SOURCE_DIR}
	 PATTERN "*.in" EXCLUDE)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/debian/control.in
               ${PROJECT_SOURCE_DIR}/debian/control @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/debian/rules.in
               ${PROJECT_SOURCE_DIR}/debian/rules @ONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/LICENSE.LGPL
               ${PROJECT_SOURCE_DIR}/debian/copyright COPYONLY)



# --- Configuration of CPACK build generator to create debian files
# Currently, using debuild for binaru builts seems to be the less complex
# solution.

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "icl")
SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

# Debian meta information
SET(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}${VERSION_SUFFIX}+${LSB_RELEASE_CODENAME}")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "${ICL_DEB_PACKAGE_MAINTAINER}")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${ICL_DEB_PACKAGE_DESCRIPTION}")
SET(CPACK_DEBIAN_PACKAGE_SECTION ${ICL_DEB_PACKAGE_SECTION})
SET(CPACK_DEBIAN_PACKAGE_PRIORITY ${ICL_DEB_PACKAGE_PRIORITY})
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${ICL_DEB_PACKAGE_ARCH}")
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

# Combining components into groups and enable component build
set(CPACK_DEB_COMPONENT_INSTALL ON)
SET(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
SET(CPACK_COMPONENT_APPLICATIONS_GROUP tools)
SET(CPACK_COMPONENT_DEMOS_GROUP dev)
SET(CPACK_COMPONENT_DEVELOPMENT_GROUP dev)
SET(CPACK_COMPONENT_DOCUMENTATION_GROUP doc)
SET(CPACK_COMPONENT_EXAMPLES_GROUP dev)
SET(CPACK_COMPONENT_LIBRARIES_GROUP core)

SET(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
SET(ICL_DEB_CORE_DEPENDENCY "icl-core (= ${CPACK_PACKAGE_VERSION})")

# Dependencies for using ICL
# Unfortunately, shlip does not work with CPACK for ICL
# These package lists have been retrieved from the  binary packages
# built with debuild (ppa versions)
IF(${LSB_RELEASE_CODENAME} STREQUAL "trusty")
SET(ICL_DEB_CORE_DEPENDENCIES "libavcodec54 (>= 6:9.1-1) | libavcodec-extra-54 (>= 6:9.20), libavformat54 (>= 6:9.1-1), libavutil52 (>= 6:9.1-1), libc6 (>= 2.15), libdc1394-22, libfreenect0.2 (>= 1:0.1.2), libgcc1 (>= 1:4.1.1), libgl1-mesa-glx | libgl1, libglew1.10 (>= 1.10.0), libglu1-mesa | libglu1, libjpeg8 (>= 8c), libmagick++5 (>= 8:6.7.7.10), libopencl-1.1-1, libopencl-1.2-1, libopencv-calib3d2.4, libopencv-core2.4, libopencv-features2d2.4, libopencv-highgui2.4, libopencv-imgproc2.4, libopenni0, libpng12-0 (>= 1.2.13-4), libqt5core5a (>= 5.0.2), libqt5gui5 (>= 5.0.2) | libqt5gui5-gles (>= 5.0.2), libqt5opengl5 (>= 5.0.2) | libqt5opengl5-gles (>= 5.0.2), libqt5widgets5 (>= 5.0.2), libstdc++6 (>= 4.6), libswscale2 (>= 6:9.1-1), libusb-0.1-4 (>= 2:0.1.12), libzmq1, ocl-icd-libopencl1 (>= 1.0) | libopencl1, zlib1g (>= 1:1.1.4)")
  SET(ICL_DEB_TOOLS_DEPENDENCIES "${ICL_DEB_CORE_DEPENDENCY} libc6 (>= 2.14), libdc1394-22, libgcc1 (>= 1:4.1.1), libgl1-mesa-glx | libgl1, libqt5core5a (>= 5.0.2), libqt5widgets5 (>= 5.0.2), libstdc++6 (>= 4.4.0)")
ELSEIF(${LSB_RELEASE_CODENAME} STREQUAL "xenial")
  SET(ICL_DEB_CORE_DEPENDENCIES "libavcodec-ffmpeg56 (>= 7:2.4) | libavcodec-ffmpeg-extra56 (>= 7:2.4), libavformat-ffmpeg56 (>= 7:2.4), libavutil-ffmpeg54 (>= 7:2.4), libc6 (>= 2.15), libdc1394-22, libfreenect0.5 (>= 1:0.1.2), libgcc1 (>= 1:4.2), libgl1-mesa-glx | libgl1, libglew1.13 (>= 1.12.0), libglu1-mesa | libglu1, libjpeg8 (>= 8c), libmagick++-6.q16-5v5 (>= 8:6.8.9.9-7ubuntu5.9), libopencv-calib3d2.4v5, libopencv-core2.4v5, libopencv-features2d2.4v5, libopencv-highgui2.4v5, libopencv-imgproc2.4v5, libopenni0, libpng12-0 (>= 1.2.13-4), libqt5core5a (>= 5.5.0), libqt5gui5 (>= 5.3.0) | libqt5gui5-gles (>= 5.3.0), libqt5multimedia5 (>= 5.5.0), libqt5opengl5 (>= 5.0.2) | libqt5opengl5-gles (>= 5.0.2), libqt5widgets5 (>= 5.0.2), libstdc++6 (>= 5.2), libswscale-ffmpeg3 (>= 7:2.4), libusb-0.1-4 (>= 2:0.1.12), libzmq1, ocl-icd-libopencl1 | libopencl1, ocl-icd-libopencl1 (>= 1.0) | libopencl-1.1-1, ocl-icd-libopencl1 (>= 1.0) | libopencl-1.2-1, zlib1g (>= 1:1.1.4)")
  SET(ICL_DEB_TOOLS_DEPENDENCIES "${ICL_DEB_CORE_DEPENDENCY} libc6 (>= 2.14), libdc1394-22, libgcc1 (>= 1:3.0), libgl1-mesa-glx | libgl1, libqt5core5a (>= 5.3.0), libqt5widgets5 (>= 5.0.2), libstdc++6 (>= 5.2)")
ELSE()
  MESSAGE(FATAL_ERROR "Cannot generate DEB packages for '${LSB_RELEASE_CODENAME}'")
ENDIF()

# Merge each group into a debian package, set dependencies for core
# and add icl-core as a dependency if necessary
SET(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS ${ICL_DEB_CORE_DEPENDENCIES}) # should be automatically set
SET(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS ${ICL_DEB_CORE_DEPENDENCY})
SET(CPACK_DEBIAN_DOC_PACKAGE_SUGGESTS ${ICL_DEB_CORE_DEPENDENCY})
SET(CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS ${ICL_DEB_TOOLS_DEPENDENCIES})
set(CPACK_COMPONENTS_ALL applications demos development examples libraries)

# this HAS to be the last call. Otherwise CPACK variables are not exported
INCLUDE(CPack)


