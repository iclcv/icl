/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PCDFileGrabber.cpp                 **
** Module : ICLGeom                                                **
** Authors: Patrick Nobou                                          **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLGeom/PCDFileGrabber.h>
#include <ICLIO/FileGrabber.h>

#include <bitset>
#include <sstream>


namespace icl{

  using namespace utils;
  using namespace math;
  using namespace io;

  namespace geom{
    struct PCDFileGrabber::Data{

    };
    
    
      
    PCDFileGrabber::PCDFileGrabber(const std::string &filepattern, bool loop):
      m_data(new Data){
      
    }
      
    
    PCDFileGrabber::~PCDFileGrabber(){
      delete m_data;
    }
    
    
    void PCDFileGrabber::grab(PointCloudObjectBase &dst){
      
    }

    
  }
}
