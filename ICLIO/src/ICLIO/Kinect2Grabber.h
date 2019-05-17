/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/Kinect2Grabber.h                       **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Andre Ueckermann                  **
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

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace io{
    /// Special Grabber implementation for Microsoft's Kinect2 Device
    /** This class implements ICL's Grabber interface for Microsofts Kinect2
        Device. Internally, it uses libfreenect to access the device. */
    struct Kinect2Grabber : public Grabber{
        enum Mode{
          GRAB_RGB_IMAGE,       //!< grabs rgb images form the kinects rgb camera
          GRAB_DEPTH_IMAGE,     //!< grabs the core::depth image from kinect
          GRAB_IR_IMAGE,        //!< grabs the kinects IR-image
          DUMMY_MODE            //!< for internal use only
        };

        /// returns a list of attached kinect devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        ICLIO_API Kinect2Grabber(Mode mode = GRAB_DEPTH_IMAGE, int deviceID=0) ;

        /// Destructor
        ICLIO_API ~Kinect2Grabber();

        /// grabs a new image
        ICLIO_API virtual const core::ImgBase* acquireImage();

        /// callback for changed configurable properties
        ICLIO_API void processPropertyChange(const utils::Configurable::Property &prop);

      protected:
        struct Impl; //!< internal hidden implementation class
        Impl *m_impl;//!< hidden internal data

      private:
       // void updateState();

    };
  } // namespace io
}
