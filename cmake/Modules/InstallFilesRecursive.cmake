#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/InstallFilesRecursive.cmake              **
#** Module : InstallFilesRecursive                                  **
#** Authors: Johannes Wienke                                        **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.LGPL **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the license requirements will   **
#** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************

# - Install files recursively using their header structure in the source tree
# This function install files using their header structure in the source tree.
# This is e.g. usefull to install header files with their structure. Automatic
# support for not mixing up the path logic is provided for generated files
# if they are located in CMAKE_CURRENT_BINARY dir. Otherwise
# CMAKE_CURRENT_SOURCE_DIR is assumed as the absolute location for the files to
# install.
#
# INSTALL_FILES_RECURSIVE(DESTINATION FILE_LIST)
#
#     DESTINATION: destination folder under which the structure of the files to
#                  install is mirrored. INSTALL is used, hence relative
#                  locations will be relative to CMAKE_INSTALL_PREFIX.
#     FILE_LIST: list of files to install
#
# Exmaple:
# INSTALL_FILES_RECURSIVE(include "rsc/foo.h"
#                                 "rsc/test/bla.h"
#                                 "${CMAKE_CURRENT_BINARY_DIR}/rsc/narf/damn/xxx.h")
# will install:
#   ${CMAKE_INSTALL_PREFIX}/include/rsc/foo.h
#   ${CMAKE_INSTALL_PREFIX}/include/rsc/test/bla.h
#   ${CMAKE_INSTALL_PREFIX}/include/rsc/narf/damn/xxx.h
#

MACRO(INSTALL_FILES_RECURSIVE DESTINATION FILE_LIST)

    FOREACH(FILE ${${FILE_LIST}})

        #MESSAGE("Processing file ${FILE}")
        GET_FILENAME_COMPONENT(ABSOLUTE_FILE ${FILE} ABSOLUTE)
        #MESSAGE("ABSOLUTE_FILE: ${ABSOLUTE_FILE}")

        # first, find out if this file is relative to a source or binary dir
        FILE(RELATIVE_PATH FILE_REL_TO_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src ${ABSOLUTE_FILE})
        FILE(RELATIVE_PATH FILE_REL_TO_BINARY ${CMAKE_CURRENT_BINARY_DIR} ${ABSOLUTE_FILE})
        #MESSAGE("REL_TO_SOURCE: ${FILE_REL_TO_SOURCE}")
        #MESSAGE("REL_TO_BINARY: ${FILE_REL_TO_BINARY}")
        STRING(LENGTH ${FILE_REL_TO_SOURCE} FILE_REL_TO_SOURCE_LENGTH)
        STRING(LENGTH ${FILE_REL_TO_BINARY} FILE_REL_TO_BINARY_LENGTH)

        # chose the relative path to start from
        SET(BASE_PATH ${FILE_REL_TO_SOURCE})
        IF(${FILE_REL_TO_BINARY_LENGTH} LESS ${FILE_REL_TO_SOURCE_LENGTH})
            SET(BASE_PATH ${FILE_REL_TO_BINARY})
        ENDIF()
        #MESSAGE("BASE_PATH: ${BASE_PATH}")

        # extract destination subdirectory
        GET_FILENAME_COMPONENT(REL_PATH ${BASE_PATH} PATH)
        #MESSAGE("REL_PATH: ${REL_PATH}")

        INSTALL(FILES ${FILE} DESTINATION ${DESTINATION}/${REL_PATH} COMPONENT development)

    ENDFOREACH()

ENDMACRO()
