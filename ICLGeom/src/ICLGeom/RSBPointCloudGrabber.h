/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/RSBPointCloudGrabber.h             **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLGeom/PointCloudGrabber.h>

#if !defined(ICL_HAVE_RSB) || !defined(ICL_HAVE_PROTOBUF)
  #if WIN32
    #pragma WARNING("This header should only be included if HAVE_RSB and HAVE_PROTOBUF are defined and available in ICL")
  #else
    #warning "This header should only be included if HAVE_RSB and HAVE_PROTOBUF are defined and available in ICL"
  #endif
#endif


namespace icl{
  namespace geom{

    /// Grabs points clouds from RSB
    class ICLGeom_API RSBPointCloudGrabber : public PointCloudGrabber{

      struct Data;   //!< internal data type
      Data *m_data;  //!< internal data pointer

      public:
      /// create grabber with given scope
      /** if scope is empty, the grabber is not initialized! */
      RSBPointCloudGrabber(const std::string &scope="", const std::string &transportList="spread",
                           Camera *dCam = 0, Camera *cCam = 0);

      /// destructor
      ~RSBPointCloudGrabber();

      /// deferred initialization with given scope
      void init(const std::string &scope, const std::string &trasportList);

      /// fills the given point cloud with grabbed information
      virtual void grab(PointCloudObjectBase &dst);

      /// returns depth camera (only if explicitly given in the creation string)
      Camera getDepthCamera() const;

      /// returns color camera (only if explicitly given in the creation string)
      Camera getColorCamera() const;
    };
  } // namespace geom
}
