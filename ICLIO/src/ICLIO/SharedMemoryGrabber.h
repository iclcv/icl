/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SharedMemoryGrabber.h                  **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/Grabber.h>

namespace icl{
  namespace io{

    /// Grabber class that grabs images from SharedMemorySegment instances
    /** Images that are published using the SharedMemoryPublisher can
        be grabbed with this grabber type. Please don't use this
        Grabber class directly, but instantiate GenericGrabber with
        Devide type 'sm'.
    */
    class ICLIO_API SharedMemoryGrabber : public Grabber {
        /// Internal Data storage class
        struct Data;

        /// Hidden Data container
        Data *m_data;

        /// Connects an unconnected grabber to given shared memory segment
        void init(const std::string &sharedMemorySegmentID) throw (utils::ICLException);

      public:

        /// Creates a new SharedMemoryGrabber instance (please use the GenericGrabber instead)
        SharedMemoryGrabber(const std::string &sharedMemorySegmentID="") throw(utils::ICLException);

        /// Destructor
        ~SharedMemoryGrabber();

        /// returns a list of all available shared-memory image-streams
        static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        /// grabbing function
        /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
        virtual const core::ImgBase* acquireImage();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

        /// resets all 'shared-memory-segents and system-semaphores'
        static void resetBus(bool verbose);
    };

  } // namespace io
}

