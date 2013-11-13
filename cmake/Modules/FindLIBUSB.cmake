#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindLIBUSB.cmake                         **
#** Module : FindLIBUSB                                             **
#** Authors: Christof Elbrechter                                    **
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

# Search LIBUSB_ROOT first if it is set.
IF(LIBUSB_ROOT)
  #SET(_LIBUSB_SEARCH_ROOT PATHS ${LIBUSB_ROOT} ${LIBUSB_ROOT}/lib NO_DEFAULT_PATH)
  LIST(APPEND LIB_USB_SEARCH_PATH ${LIBUSB_ROOT})
ELSE()
  LIST(APPEND LIB_USB_SEARCH_PATH "/usr")
ENDIF()


FIND_PATH(LIBUSB_INCLUDE_DIR 
  NAMES usb.h        
  PATHS ${LIB_USB_SEARCH_PATH}
  PATH_SUFFIXES "/include" 	  
  DOC "The path to LIBUSB header files"
  NO_DEFAULT_PATH)
  
FIND_LIBRARY(LIBUSB_LIBRARY  
  NAMES usb
  PATHS "${LIB_USB_SEARCH_PATH}"
  PATH_SUFFIXES "/lib/" "lib/${ARCH_DEPENDENT_LIB_DIR}/" 
  NO_DEFAULT_PATH)

# Handle the QUIETLY and REQUIRED arguments and set LIBUSB_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBUSB REQUIRED_VARS 
				  LIBUSB_LIBRARY
				  LIBUSB_INCLUDE_DIR)

IF(LIBUSB_FOUND)
  SET(LIBUSB_INCLUDE_DIRS ${LIBUSB_INCLUDE_DIR})
  SET(LIBUSB_LIBRARIES ${LIBUSB_LIBRARY})
ENDIF()

MARK_AS_ADVANCED(LIBUSB_INCLUDE_DIR)
