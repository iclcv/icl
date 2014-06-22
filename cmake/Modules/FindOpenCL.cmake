#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindOpenCL.cmake                         **
#** Module : FindOpenCL                                             **
#** Authors: Michael Goetting                                       **
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

SET(_CUDA_PATH $ENV{CUDA_PATH})
IF(_CUDA_PATH)
  FILE(TO_CMAKE_PATH ${_CUDA_PATH} _CUDA_PATH)
ENDIF(_CUDA_PATH)

ICL_FIND_PACKAGE(NAME OpenCL
                 HEADERS "CL/cl.hpp;CL/cl.h;CL/opencl.h"
                 LIBS "OpenCL"
                 PATHS ${_CUDA_PATH}
                 OPTIONAL)

IF(OPENCL_FOUND)
  FILE(STRINGS ${OPENCL_INCLUDE_DIRS}/CL/cl.h HAVE_CL_1_0 REGEX "CL_VERSION_1_0")
  FILE(STRINGS ${OPENCL_INCLUDE_DIRS}/CL/cl.h HAVE_CL_1_1 REGEX "CL_VERSION_1_1")
  FILE(STRINGS ${OPENCL_INCLUDE_DIRS}/CL/cl.h HAVE_CL_1_2 REGEX "CL_VERSION_1_2")

  
  IF("${HAVE_CL_1_2}" STREQUAL "")
    IF("${HAVE_CL_1_1}" STREQUAL "")
      message(STATUS "Found OpenCL Version: 1.0")
    ELSE()
      message(STATUS "Found OpenCL Version: 1.1")
    ENDIF()
  ELSE()
    message(STATUS "Found OpenCL Version: 1.2 (adding compatibility definition for 1.1)")
    ADD_DEFINITIONS( -DCL_USE_DEPRECATED_OPENCL_1_1_APIS)
  ENDIF()
ENDIF()
