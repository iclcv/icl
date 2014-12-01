/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PCDFileGrabber.h                   **
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

#pragma once

#include <ICLGeom/PointCloudGrabber.h>


namespace icl{

  namespace geom{

    class PCDFileGrabber : public PointCloudGrabber{
      struct Data;  // !< pimpl type
      Data *m_data; // !< pimpl pointer
      
      public:
      
      /// creates a new PCD file grabber instance
      /** @param filename to be grabbed PCD file name or file pattern. (e.g. files/ *.pcd)
          @param repeat specifies whether to play PCD file in an endless loop or not.
          @param timestamp time stamp to render the files.
          @param forceExactPCLType if this flag is set to true, the PointCloudObjectBase-reference
          given to grab must have exactly the same fiels as the pcl-file
          @param offset Similar to the offset variable in the file pcd_io.h.
      */
      PCDFileGrabber(const std::string &filepattern="", bool loop = true);
      
      /// Destructor
      virtual ~PCDFileGrabber();
      
      /// grab implementation
      virtual void grab(PointCloudObjectBase &dst);
    };
  }
}
