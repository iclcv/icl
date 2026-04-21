// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/Grabber.h>

namespace icl::io {
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
      void init(const std::string &sharedMemorySegmentID);

    public:

      /// Creates a new SharedMemoryGrabber instance (please use the GenericGrabber instead)
      SharedMemoryGrabber(const std::string &sharedMemorySegmentID="");

      /// Destructor
      ~SharedMemoryGrabber();

      /// returns a list of all available shared-memory image-streams
      static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

      /// grabbing function
      /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
      virtual const core::ImgBase* acquireDisplay();

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);

      /// resets all 'shared-memory-segents and system-semaphores'
      static void resetBus(bool verbose);
  };

  } // namespace icl::io