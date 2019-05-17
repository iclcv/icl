/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/ZmqGrabber.h                           **
** Module : ICLIO                                                  **
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

#include <ICLIO/Grabber.h>

namespace icl{
  namespace io{

    /// Grabber class that grabs images from ZeroMQ-based network interfaces
    class ZmqGrabber : public Grabber {
      /// Internal Data storage class
      struct Data;

      /// Hidden Data container
      Data *m_data;

      public:

      /// Creates a new SharedMemoryGrabber instance (please use the GenericGrabber instead)
      ICLIO_API ZmqGrabber(const std::string &host, int port=44444);

      /// Destructor
      ICLIO_API ~ZmqGrabber();

      /// returns a list of all available shared-memory image-streams
      ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

      /// grabbing function
      /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
      ICLIO_API virtual const core::ImgBase* acquireImage();
    };

  } // namespace io
}

