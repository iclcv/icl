cmake_host_system_information(RESULT ICL_DEB_NUM_CORES
                              QUERY NUMBER_OF_LOGICAL_CORES)

SET(CPACK_GENERATOR ${BUILD_REDIST})
SET(CPACK_PACKAGE_NAME "icl")
SET(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
SET(CPACK_PACKAGE_VENDOR "iclcv.org")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Image Component Library")

# Combining components into groups and enable component build
SET(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
SET(CPACK_COMPONENT_APPLICATIONS_GROUP tools)
SET(CPACK_COMPONENT_DEMOS_GROUP dev)
SET(CPACK_COMPONENT_DEVELOPMENT_GROUP dev)
SET(CPACK_COMPONENT_DOCUMENTATION_GROUP doc)
SET(CPACK_COMPONENT_EXAMPLES_GROUP dev)
SET(CPACK_COMPONENT_LIBRARIES_GROUP core)

SET(CPACK_COMPONENTS_ALL libraries development)

IF(BUILD_APPS)
LIST(APPEND CPACK_COMPONENTS_ALL applications)
ENDIF(BUILD_APPS)

IF (BUILD_DEMOS)
LIST(APPEND CPACK_COMPONENTS_ALL demos)
ENDIF(BUILD_DEMOS)

IF (BUILD_EXAMPLES)
LIST(APPEND CPACK_COMPONENTS_ALL examples)
ENDIF(BUILD_EXAMPLES)

IF(BUILD_REDIST STREQUAL DEB)

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

  SET(ICL_DEB_PACKAGE_MAINTAINER "Alexander Neumann <aleneum@gmail.com>")
  SET(ICL_DEB_PACKAGE_DESCRIPTION_SHORT "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
  SET(ICL_DEB_PACKAGE_DESCRIPTION "ICL is a novel C++ computer-vision library developed in the NeuroInformatics group at CITEC/Bielefeld University, Germany. It unifies both, performance and user friendliness. ICL provides a large set of simple-to-use classes and functions to facilitate development of complex computer vision applications.")
  SET(ICL_DEB_PACKAGE_SECTION "devel")
  SET(ICL_DEB_PACKAGE_PRIORITY "optional")

  #SET(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
  SET(CPACK_PACKAGE_FILE_NAME "${CPACK_DEBIAN_PACKAGE_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

  SET(CPACK_DEBIAN_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
  SET(CPACK_DEBIAN_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}${VERSION_SUFFIX}+${LSB_RELEASE_CODENAME}")
  SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "${ICL_DEB_PACKAGE_MAINTAINER}")
  SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${ICL_DEB_PACKAGE_DESCRIPTION}")
  SET(CPACK_DEBIAN_PACKAGE_SECTION ${ICL_DEB_PACKAGE_SECTION})
  SET(CPACK_DEBIAN_PACKAGE_PRIORITY ${ICL_DEB_PACKAGE_PRIORITY})
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${ICL_DEB_PACKAGE_ARCH}")

  # Dependencies for building ICL; will be parsed into debian/control
  STRING(CONCAT ICL_DEB_BUILD_DEPENDENCIES "cmake3 | cmake, libjpeg-dev, libpng-dev, ocl-icd-opencl-dev, opencl-headers, libeigen3-dev, libopenni-dev, "
         "libopencv-dev, libmagick++-dev, qtbase5-dev, qtmultimedia5-dev, libqt5opengl5-dev, libfreenect-dev, libpcl-all | libpcl-dev, "
         "libzmq3-dev, libglew-dev, libusb-dev, libavcodec-dev, libavresample-dev, libavdevice-dev, doxygen, graphviz, python3-sphinx, "
         "python3-pyparsing, libbullet-dev, texlive-latex-base, ghostscript")
  SET(ICL_DEB_CORE_DEPENDENCY "icl-core (= ${CPACK_PACKAGE_VERSION})")

  # Dependencies for using ICL
  # Unfortunately, shlip does not work with CPACK for ICL
  # These package lists have been retrieved from the  binary packages
  # built with debuild (ppa versions)

  # Merge each group into a debian package, set dependencies for core
  # and add icl-core as a dependency if necessary
  SET(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
  SET(CPACK_DEBIAN_CORE_PACKAGE_DEPENDS ${ICL_DEB_CORE_DEPENDENCIES}) # should be automatically set
  SET(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS ${ICL_DEB_CORE_DEPENDENCY})
  SET(CPACK_DEBIAN_DOC_PACKAGE_SUGGESTS ${ICL_DEB_CORE_DEPENDENCY})
  SET(CPACK_DEBIAN_TOOLS_PACKAGE_DEPENDS ${ICL_DEB_TOOLS_DEPENDENCIES})

  # --- Preparing Debian folder for source packge built
  FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/debian
       DESTINATION ${PROJECT_SOURCE_DIR}
       PATTERN "*.in" EXCLUDE)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/debian/control.in
                 ${PROJECT_SOURCE_DIR}/debian/control @ONLY)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/debian/rules.in
                 ${PROJECT_SOURCE_DIR}/debian/rules @ONLY)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/debian/changelog.in
                 ${PROJECT_SOURCE_DIR}/debian/changelog @ONLY)
  CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/LICENSE.LGPL
                 ${PROJECT_SOURCE_DIR}/debian/copyright COPYONLY)
  execute_process(COMMAND ${CMAKE_COMMAND} -E echo "##[group]Install build dependencies")
  execute_process(COMMAND sudo mk-build-deps --install -t "apt-get --no-install-recommends -y" ${PROJECT_SOURCE_DIR}/debian/control)
  execute_process(COMMAND ${CMAKE_COMMAND} -E echo "##[endgroup]")


ELSEIF(BUILD_REDIST STREQUAL WIX)
  SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${ARCH}")
  SET(CPACK_WIX_SKIP_PROGRAM_FOLDER TRUE)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "tools/iclcv/${ARCH}")

  # This GUID has been generated in an initial run
  SET(CPACK_WIX_UPGRADE_GUID "C61CF96C-EA18-4A7A-A1F6-986E89BDCC1B")

  FILE(COPY ${PROJECT_SOURCE_DIR}/LICENSE.LGPL
       DESTINATION ${CMAKE_BINARY_DIR})
  FILE(RENAME ${CMAKE_BINARY_DIR}/LICENSE.LGPL ${CMAKE_BINARY_DIR}/LICENSE.txt)
  SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_BINARY_DIR}/LICENSE.txt)


  SET(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Tools")
  SET(CPACK_COMPONENT_DEMOS_DISPLAY_NAME "Demo applications")
  SET(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "C++ Header")
  SET(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "ICL Manual")
  SET(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Example applications")
  SET(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "ICL Libraries")

  SET(CPACK_COMPONENT_GROUP_CORE_DISPLAY_NAME "Mandatory")
  SET(CPACK_COMPONENT_GROUP_DEV_DISPLAY_NAME "Development")
  SET(CPACK_COMPONENT_GROUP_DOC_DISPLAY_NAME "Documentation")
  SET(CPACK_COMPONENT_GROUP_TOOLS_DISPLAY_NAME "Tools")

  SET(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION
  "Applications built with ICL")
  SET(CPACK_COMPONENT_DEMOS_DESCRIPTION
  "ICL demo applications")
  SET(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION
  "ICL Header and development tools")
  SET(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION
  "Sphinx manual")
  SET(CPACK_COMPONENT_EXAMPLES_DESCRIPTION
  "Compiled ICL examples")
  SET(CPACK_COMPONENT_LIBRARIES_DESCRIPTION
  "ICL libraries")

  SET(CPACK_COMPONENT_GROUP_CORE_DESCRIPTION "Shared libraries required to run ICL apps")
  SET(CPACK_COMPONENT_GROUP_DEV_DESCRIPTION "Components related to working with ICL")
  SET(CPACK_COMPONENT_GROUP_DOC_DESCRIPTION "Documentation")
  SET(CPACK_COMPONENT_GROUP_TOOLS_DESCRIPTION "ICL computervision tools")

ENDIF(BUILD_REDIST STREQUAL DEB)
INCLUDE(CPack)
