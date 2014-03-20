/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudGrabber.h                **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/PointCloudObjectBase.h>

#include <map>

namespace icl{
  namespace geom{
  
    /// Generic interface for PointCloud sources
    struct PointCloudGrabber{
      /// fills the given point cloud with grabbed information
      virtual void grab(PointCloudObjectBase &dst) = 0;
      
      /// virtual, but empty destructor
      virtual ~PointCloudGrabber(){}

      /// grabber type registration tool
      class Register{
        public:
        typedef utils::Function<PointCloudGrabber*,const std::string&> CreateFunction;
         struct RegisteredGrabberType{
          std::string name;
          std::string description;
          CreateFunction create;
        };
        
        static Register &instance();
        
        void registerGrabberType(const std::string &name, CreateFunction create, 
                                 const std::string &description);
        
        PointCloudGrabber *createGrabberInstance(const std::string &name, const std::string &params);
        
        std::string getRegisteredInstanceDescription();

        private:
        Register();
        
        std::map<std::string,RegisteredGrabberType> types;

      };
    };


#define REGISTER_POINT_CLOUD_GRABBER_TYPE(NAME,CREATE_FUNCTION,DESCRIPTION) \
    struct StaticPointCloudGrabberRegistrationFor_##NAME{               \
      StaticPointCloudGrabberRegistrationFor_##NAME(){                  \
        PointCloudGrabber::Register &r = PointCloudGrabber::Register::instance(); \
        r.registerGrabberType(#NAME,CREATE_FUNCTION,DESCRIPTION);       \
      }                                                                 \
    } staticPointCloudGrabberRegistrationFor_##NAME;
  } // namespace geom
}

