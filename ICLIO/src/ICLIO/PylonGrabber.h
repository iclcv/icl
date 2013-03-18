/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/PylonGrabber.h                         **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#pragma once

#include <ICLIO/PylonIncludes.h>

#include <ICLIO/Grabber.h>
#include <ICLIO/PylonUtils.h>
#include <ICLIO/PylonCameraOptions.h>
#include <ICLIO/PylonGrabberThread.h>
#include <ICLIO/PylonColorConverter.h>
#include <ICLUtils/Time.h>

namespace icl {
  namespace io{

    namespace pylon {

      /// Grabber implementation for a Basler Pylon-based GIG-E Grabber \ingroup GIGE_G
      /**
          This is just a wrapper class of the underlying PylonGrabberImpl class

          Some useful hints to increase GigE camera output:

         -# Jumbo Frames: If your Network Adapter supports Jumbo Frames they
            should be enabled by setting the Maximum Transfer Unit (MTU) size
            to 8192. Accordingly the cameras property "GevSCPSPacketSize" is
            set to 8192 per default. Setting this property to a value higher
            then the Network Adapters MTU may create transfer errors.
         -# Real-time thread priorities: To minimize network packet losses it
            helps to grant pylon the permission to change a threads priority
            to real time. This can be achieved by adding the line:
            \code
               *      -      rtprio      99
            \endcode
            to
            \code
            /etc/security/limits.conf
            \endcode
            This can make the difference between a network throughput of 32 and 100Mb/s.
         -# Transmission errors: If you often get the error code
            'GX status 0xe1000014' and already followed the previous hints
            increasing the 'GevSCPD' (Inter packet delay) parameter can help to
            minimize these transmission errors.
         -# Camera IP Configuration: can be made with the IpConfigurator which
            is included in the Pylon driver package. When the tool does not
            find the camera, ICL will neither. This most commonly means that the
            camera is in an other ip-address block then the computer. Because a
            connection to the camera is needed in order to change the cameras ip
            settings, it is possible to either chnage the ip address of the
            computer to the same ip-address block or to use the Windows version
            of the IpConfigurator - which does not seem to have souch problems - to
            change the cameras ip settings once.
         -# Network Adapter: Basler is recommending Network Adapters of the
            Intel PRO 1000 series. They observed a significantly higher CPU load
            when working with other.

      **/
      class PylonGrabber : public Grabber, public Interruptable {
        public:
          /// The constructor
          /**
          * @param dev The PylonDevice that should be used for image acquisition.
          * @param args The arguments provided to this grabber.
          */
          PylonGrabber(const Pylon::CDeviceInfo &dev, const std::string args);

          /// Destructor
          ~PylonGrabber();

          /// grab function grabs an image (destination image is adapted on demand)
          /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
          virtual const core::ImgBase* acquireImage();

          /// Uses args to choose a pylon device
          /**
          * @param args The arguments provided to this grabber.
          * @throw ICLException when no suitable device exists.
          */
          static Pylon::CDeviceInfo getDeviceFromArgs(std::string args) throw(utils::ICLException);

        private:
          /// A mutex lock to synchronize buffer and color converter access.
          utils::Mutex m_ImgMutex;
          /// The PylonEnvironment automation.
          PylonAutoEnv m_PylonEnv;
          /// Count of buffers for grabbing
          static const int m_NumBuffers = 3;
          /// The camera interface.
          Pylon::IPylonDevice* m_Camera;
          /// The streamGrabber of the camera.
          Pylon::IStreamGrabber* m_Grabber;
          /// PylonCameraOptions used to get and set camera settings.
          PylonCameraOptions* m_CameraOptions;
          /// PylonColorConverter used for color conversion.
          PylonColorConverter* m_ColorConverter;
          /// PylonGrabberThread used for continous image acquisition.
          PylonGrabberThread* m_GrabberThread;
          /// A list of used buffers.
          std::vector<PylonGrabberBuffer<uint16_t>*> m_BufferList;
          /// A pointer to the last used buffer.
          core::ImgBase* m_LastBuffer;

          /// starts the acquisition of pictures by the camera
          void acquisitionStart();
          /// stops the acquisition of pictures by the camera
          void acquisitionStop();
          /// creates buffers and registers them at the grabber
          void grabbingStart();
          /// deregisters buffers from grabber and deletes them
          void grabbingStop();

          /// Prints information about the startup argument options
          static void printHelp();

          /// helper function that makes default settings for the camera.
          void cameraDefaultSettings();
          /// Converts pImageBuffer to correct type and writes it into m_Image
          void convert(const void *pImageBuffer);
      };

    } //namespace pylon

    typedef pylon::PylonGrabber PylonGrabber;

  } // namespace io
} //namespace icl

