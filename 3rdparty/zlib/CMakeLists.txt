#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : 3rdparty/zlib/CMakeLists.txt                           **
#** Module : zlib                                                   **
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

SET(SOURCES adler32.c
            compress.c
            crc32.c
            deflate.c
            gzclose.c
            gzlib.c
            gzread.c
            gzwrite.c
            infback.c
            inffast.c
            inflate.c
            inftrees.c
            trees.c
            uncompr.c
            zutil.c)

SET(HEADERS crc32.h
            deflate.h
            gzguts.h
            inffast.h
            inffixed.h
            inflate.h
            inftrees.h
            trees.h
            zconf.h
            zlib.h
            zutil.h)

ADD_ICL_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

ADD_LIBRARY(zlib STATIC ${SOURCES} ${HEADERS})

SET_TARGET_PROPERTIES(zlib
                      PROPERTIES
                      VERSION ${SO_VERSION})
