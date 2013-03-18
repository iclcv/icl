/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/KinectGrabber.h                        **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
    /// Special Grabber implementation for Microsoft's Kinect Device
    /** This class implements ICL's Grabber interface for Microsofts Kinect
        Device. Internally, it uses libfreenect to access the device. */
    struct KinectGrabber : public Grabber{
        enum Mode{
          GRAB_RGB_IMAGE,       //!< grabs rgb images form the kinects rgb camera
          GRAB_BAYER_IMAGE,     //!< not supported yet
          GRAB_DEPTH_IMAGE,     //!< grabs the core::depth image from kinect
          GRAB_IR_IMAGE_8BIT,   //!< grabs the kinects IR-image in most common 8Bit depth
          GRAB_IR_IMAGE_10BIT,  //!< grabs the kinects IR-image in 10Bit core::depth (use depth16s at least)
        };

        /// returns a list of attached kinect devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        KinectGrabber(Mode mode = GRAB_DEPTH_IMAGE, int deviceID=0, const utils::Size &size=utils::Size::VGA) throw (utils::ICLException);

        /// Destructor
        ~KinectGrabber();

        /// grabs a new image
        virtual const core::ImgBase* acquireImage();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

      protected:
        struct Impl; //!< internal hidden implementation class
        Impl *m_impl;//!< hidden internal data
    };
  } // namespace io
}
