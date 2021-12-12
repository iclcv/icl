#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindWinProtobuf.cmake                    **
#** Module : FindWinProtobuf                                        **
#** Authors: Michael Goetting, Sergius Gaulik                       **
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

INCLUDE(FindPackageHandleStandardArgs)

# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

# Ask the root directory of protobuf.
SET(PROTOBUF_ROOT PROTOBUF_ROOT CACHE PATH "Root directory of protobuf")

IF(PROTOBUF_ROOT)
  SET(PROTOBUF_SRC_ROOT_FOLDER PROTOBUF_ROOT)
ENDIF()

FIND_PATH(PROTOBUF_INCLUDE_DIR
          NAMES google/protobuf/service.h
          PATHS ${PROTOBUF_ROOT}
          PATH_SUFFIXES "include"
          DOC "The path to protobuf header files"
          NO_DEFAULT_PATH)

FIND_LIBRARY(PROTOBUF_LIBRARY
             NAMES libprotobuf
             PATHS ${PROTOBUF_ROOT}
             PATH_SUFFIXES lib bin
             NO_DEFAULT_PATH)

find_program(PROTOBUF_PROTOC_EXECUTABLE
             NAMES protoc
             DOC "The Google Protocol Buffers Compiler"
             PATHS ${PROTOBUF_ROOT}
             PATH_SUFFIXES bin
             NO_DEFAULT_PATH)

# Use the normal search first
INCLUDE(FindProtobuf)

MARK_AS_ADVANCED(PROTOBUF_SRC_ROOT_FOLDER)
MARK_AS_ADVANCED(CLEAR PROTOBUF_INCLUDE_DIR)
MARK_AS_ADVANCED(CLEAR PROTOBUF_LIBRARY)
MARK_AS_ADVANCED(CLEAR PROTOBUF_PROTOC_EXECUTABLE)

# Check for executable because FindProtobuf does not check for it
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PROTOBUF REQUIRED_VARS
                                  PROTOBUF_PROTOC_EXECUTABLE)
