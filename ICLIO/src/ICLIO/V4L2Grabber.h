/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/V4L2Grabber.h                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
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
#include <ICLUtils/Mutex.h>

namespace icl{
  namespace io{

    /// The Video for Linux 2 Grabber uses the v4l2-api to access video capturing devices \ingroup GRABBER_G \ingroup V4L_G
    /** This grabber backend is usually used for USB-Webcams as well as for Grabber cards */
    class V4L2Grabber : public Grabber{
        class Impl; //!< internal implementation
        Impl *impl; //!< internal data structure
        utils::Mutex implMutex; //!< protects the impl which is reallocated when the core::format is changed
      public:

        /// create a new grabbers instance, with given device name (
        ICLIO_API V4L2Grabber(const std::string &device="/dev/video0");

        /// Destruktoer
        ICLIO_API ~V4L2Grabber();

        /// obtains the next image
        ICLIO_API virtual const core::ImgBase *acquireImage();

        /// returns a list of all supported video devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

      private:
        /// adds properties to Configurable
        void addProperties();
        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);
    };

  } // namespace io
}

