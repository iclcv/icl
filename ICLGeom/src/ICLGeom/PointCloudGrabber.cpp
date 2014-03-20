/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudGrabber.cpp              **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
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

#include <ICLGeom/PointCloudGrabber.h>
#include <ICLUtils/TextTable.h>

namespace icl{
  namespace geom{
    PointCloudGrabber::Register::Register(){}
    
    PointCloudGrabber::Register &PointCloudGrabber::Register::instance(){
      static Register r;
      return r;
    }
    void PointCloudGrabber::Register::registerGrabberType(const std::string &name, CreateFunction create, 
                                                          const std::string &description){
      
      RegisteredGrabberType t = { name, description, create };
      types[name] = t;
    }

    PointCloudGrabber *PointCloudGrabber::Register::createGrabberInstance(const std::string &name, 
                                                                          const std::string &params){
      std::map<std::string,RegisteredGrabberType>::iterator it = types.find(name);
      if(it == types.end()){
        ERROR_LOG("unable to create grabber instance of type " + name + "(unknown type!)");
        return 0;
      }
      return it->second.create(params);
    }

    std::string PointCloudGrabber::Register::getRegisteredInstanceDescription(){
      utils::TextTable t(2,types.size()+1,50);
      t(0,0) = "Grabber ID";
      t(1,0) = "Description";
      int i=1;
      for(std::map<std::string,RegisteredGrabberType>::iterator it = types.begin(); 
          it != types.end(); ++it){
        t(0,i) = it->second.name;
        t(1,i) = it->second.description;
      }
      return t.toString();
    }
  }
}

