#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : 3rdparty/libjpeg/CMakeLists.txt                        **
#** Module : libjpeg                                                **
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

SET(SOURCES jaricom.c
            jcapimin.c
            jcapistd.c
            jcarith.c
            jccoefct.c
            jccolor.c
            jcdctmgr.c
            jchuff.c
            jcinit.c
            jcmainct.c
            jcmarker.c
            jcmaster.c
            jcomapi.c
            jcparam.c
            jcprepct.c
            jcsample.c
            jctrans.c
            jdapimin.c
            jdapistd.c
            jdarith.c
            jdatadst.c
            jdatasrc.c
            jdcoefct.c
            jdcolor.c
            jddctmgr.c
            jdhuff.c
            jdinput.c
            jdmainct.c
            jdmarker.c
            jdmaster.c
            jdmerge.c
            jdpostct.c
            jdsample.c
            jdtrans.c
            jerror.c
            jfdctflt.c
            jfdctfst.c
            jfdctint.c
            jidctflt.c
            jidctfst.c
            jidctint.c
            jmemmgr.c
            jmemnobs.c
            jquant1.c
            jquant2.c
            jutils.c)

SET(HEADERS jconfig.h
            jdct.h
            jerror.h
            jinclude.h
            jmemsys.h
            jmorecfg.h
            jpegint.h
            jpeglib.h
            jversion.h)

ADD_LIBRARY(libjpeg STATIC ${SOURCES} ${HEADERS})

SET_TARGET_PROPERTIES(libjpeg
                      PROPERTIES
                      VERSION ${SO_VERSION})
