/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCGrabber.h                            **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Goetting, Viktor Richter  **
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


#include <ICLIO/DC.h>
#include <ICLIO/DCDevice.h>
#include <ICLIO/DCDeviceFeatures.h>
#include <ICLIO/DCDeviceOptions.h>
#include <ICLIO/Grabber.h>
#include <ICLCore/Converter.h>

namespace icl{
  namespace io{
    /** \cond */
    namespace dc{
      class DCGrabberThread;
    }
    /** \endcond */


    /// Grabber implementation for handling DC-Devices using libdc1394 (Version >= 2.0.rc9) \ingroup GRABBER_G \ingroup DC_G
    /** The DCGrabber class implements the ICL's Grabber interface for
        providing libdc1395.so.2 based camera device access. Internally it
        wraps some additional classes with name prefix "DC". \n

        The first time the "grab(..)"-function of the DCGrabber is invoked,
        it internally creates a so called DCGrabberThread. This thread
        then will create a so called DCFrameQueue internally. This queue
        is used to handle dma-image-frames, owned by the libdc which have
        been temporarily de-queued from the dma ring buffer queue into
        the user space. Here, the user has read-only access to these frames.
        The DCGrabberThread runs as fast as the current camera-settings allow
        and de-queues dma-frames from the system space into the user space
        DCFrameQueue and it en-queues old user space frames from this
        DCFrameQueue back into the dma ring buffer. At each time, the newest
        frame is available at the back of the DCFrameQueue whereas the oldest
        frame is located at the front of this queue.
        When the DCGrabbers grab-function is called, it will internally lock
        the current DCFrameQueue and convert the current frame into another
        buffer before the DCFrameQueue is unlocked again.\n
        Internally the DCGrabber wraps an instance of type DCDeviceOptions,
        which is a container for all currently implemented options. The
        wrapped classes DCGrabberThread and DCFrameQueue get a pointer to
        this option-struct at construction time, so these objects are able
        to work with the options currently set inside the parent DCGrabber
        instance.\n
        In addition, another class called DCDevice is used internally as
        a high-level wrapper for the libdc1394's camera struct. This
        DCDevice class provides some additional information to the low
        level information of the dc1394camera_t struct, e.g. some very
        camera-model specific information about the bayer-filter layout and
        so on. <b>Note:</b> New cameras, which should be supported must
        be included <b>here!</b>.\n
        As in other Grabber implementations, a static function
        "getDeviceList()" can be used to detect currently supported cameras.
        @see DCDevice, DCDeviceOptions, DCGrabberThread, DCFrameQueue
    */
    class DCGrabber : public Grabber{
      public:

        /// Constructor creates a new DCGrabber instance from a given DCDevice
        /** @param dev DCDevice to use (this device can only be created by the
                     static function getDeviceList()
          @param isoMBits give the initializer a hint to set instantiated
                          grabber to a specific iso mode by default
                          allowed values are
                          - 400 -> IEEE-1394-A (400MBit)
                          - 800 -> IEEE-1394-B (800MBit)
                          - 0 (default) value is not chaged!

                          (please note, that this parameter can also
                          be set by the property iso-speed)

          */
        ICLIO_API DCGrabber(const DCDevice &dev=DCDevice::null, int isoMBits=0);

        /// Destructor
        ICLIO_API ~DCGrabber();

        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
        ICLIO_API virtual const core::ImgBase *acquireImage();

        /// Returns a list of all connected DCDevices
        ICLIO_API static std::vector<DCDevice> getDCDeviceList(bool resetBusFirst=false);

        /// calls dc1394_reset_bus functions (see DCDevice)
        static void dc1394_reset_bus(bool verbose=false){
          DCDevice::dc1394_reset_bus(verbose);
        }

        /// filters out the size property, as it is set by the core::format property
        ICLIO_API virtual std::vector<std::string> get_io_property_list();

      private:
        /// internally used function to restart the DCGrabberThread
        /** useful if the grabber thread must have been deleted
          to update some internal properties
      */
        void restartGrabberThread();
        /// adds DCGrabbers properties to Configurable.
        void addProperties();
        /// callback function for property changes.
        void processPropertyChange(const utils::Configurable::Property &p);

        /// Wrapped DCDevice struct
        DCDevice m_oDev;

        /// Features corrsponding to m_oDev
        DCDeviceFeatures m_oDeviceFeatures;

        /// Wrapped DCGrabberThread struct
        dc::DCGrabberThread *m_poGT;

        /// Mutex for clean restarting of GrabberThread
        utils::Mutex m_GrabberThreadMutex;

        /// Internally used buffer images
        core::ImgBase *m_poImage, *m_poImageTmp;

        /// Internally used image converter
        /** This converter is used, if the wrapped DCGrabberThread
          was not able to satisfy all desired parameter claims.*/
        core::Converter m_oConverter;

        /// Internal DCDeviceOptions struct
        DCDeviceOptions m_oOptions;

        /// only for unknown device types
        std::string m_sUserDefinedBayerPattern;
    };

  } // namespace io
}

