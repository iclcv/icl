/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OpenNIGrabber.h                        **
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

#include <ICLIO/Grabber.h>
#include <ICLUtils/Time.h>
#include <ICLIO/OpenNIUtils.h>
#include <ICLUtils/Thread.h>
#include <ICLIO/OpenNIIncludes.h>

namespace icl {
  namespace io{

    // Forward declaration of OpenNIGrabberImpl
    class OpenNIGrabber;

    /// Internally spawned thread class for continuous grabbing
    class OpenNIGrabberThread : public utils::Thread {
      public:
        /// Constructor sets used grabber
        OpenNIGrabberThread(OpenNIGrabber* grabber);

        /// constantly calls grabNextImage.
        void run();
      private:
        OpenNIGrabber* m_Grabber;
    };

    /// Grabber implementation for OpenNI based camera access.
    class OpenNIGrabber : public Grabber {
      public:
        friend class OpenNIGrabberThread;

        /// The constructor
        /**
        * @param args NodeInfo of the device to use.
        */
        OpenNIGrabber(std::string args);

        /// Destructor
        ~OpenNIGrabber();

        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
        virtual const core::ImgBase* acquireImage();

        /**
            returns the underlying handle of the grabber.
            In this case the corresponding MapGenerator.
        **/
        virtual void* getHandle();

      private:
        /// makes the MapGenerator grab a new image. called repeatedly in thread.
        void grabNextImage();

        /**
            switches the current generator to desired. this function works but
            after changing to another Generator the camcfg-properties will not
            be refreshed.
        **/
        void setGeneratorTo(icl_openni::OpenNIMapGenerator::Generators desired);

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

        /// Returns the string representation of the currently used device.
        std::string getName();

        /// Mutex used for concurrency issues.
        utils::Mutex m_Mutex;
        /// a grabber id
        std::string m_Id;
        /// pointer to the currently used image generator
        icl_openni::OpenNIMapGenerator* m_Generator;
        /// internally used ReadWriteBuffer
        icl_openni::ReadWriteBuffer<core::ImgBase>* m_Buffer;
        /// a thread continuously grabbing images
        OpenNIGrabberThread* m_GrabberThread;
        /// whether double frames should be omited
        bool m_OmitDoubleFrames;
    };

  } // namespace io
} //namespace icl

