/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GenericPointCloudGrabber.h         **
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

#pragma once

#include <ICLGeom/PointCloudGrabber.h>

namespace icl{
  namespace geom{
  
    /// Generic interface for PointCloud sources
    class GenericPointCloudGrabber : public PointCloudGrabber, public utils::Uncopyable{
      struct Data;
      Data *m_data;
      
      public:
      
      /// Empty constructor (creates a null instance)
      GenericPointCloudGrabber();

      /// Constructor with initialization
      /** Possible plugins: 
          * <b>cam</b> device description is then: "depth-cam-type:depth-cam-id:depth-cam-file"
            + optionally ":color-cam-type:color-cam-id:color-cam-file"
          * <b>file</b> filename pattern (not yet implemented)
          * <b>rsb</b> rsb-transport-list: rsb-scope-list
      */
      GenericPointCloudGrabber(const std::string &sourceType, const std::string &srcDescription);
      
      /// destructor
      ~GenericPointCloudGrabber();
      
      /// deferred intialization
      void init(const std::string &sourceType, const std::string &srcDescription);
      
      /// not initialized yet?
      bool isNull() const;
      
      /// fills the given point cloud with grabbed information
      virtual void grab(PointCloudObjectBase &dst);
      
    };
  } // namespace geom
}

