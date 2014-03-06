#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : 3rdparty/libpng/CMakeLists.txt                         **
#** Module : libpng                                                 **
#** Authors: Sergius Gaulik                                         **
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

SET(SOURCES png.c
            pngerror.c
            pngget.c
            pngmem.c
            pngpread.c
            pngread.c
            pngrio.c
            pngrtran.c
            pngrutil.c
            pngset.c
            pngtrans.c
            pngwio.c
            pngwrite.c
            pngwtran.c
            pngwutil.c)

SET(HEADERS png.h
            pngconf.h
            pngdebug.h
            pnginfo.h
            pnglibconf.h
            pngpriv.h
            pngstruct.h)

INCLUDE_DIRECTORIES(BEFORE ${ICL_SOURCE_DIR}/3rdparty/zlib)
                           
ADD_LIBRARY(libpng STATIC ${SOURCES} ${HEADERS})

TARGET_LINK_LIBRARIES(libpng zlib)

SET_TARGET_PROPERTIES(libpng
                      PROPERTIES 
                      VERSION ${SO_VERSION})