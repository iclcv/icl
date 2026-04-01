// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Patrick Nobou, Christof Elbrechter

#pragma once

#include <ICLGeom/PointCloudGrabber.h>


namespace icl{

  namespace geom{

    class PCDFileGrabber : public PointCloudGrabber{
      struct Data;  // !< pimpl type
      Data *m_data; // !< pimpl pointer

      public:

      /// creates a new PCD file grabber instance
      /** @param filename to be grabbed PCD file name or file pattern. (e.g. files/ *.pcd)
          @param repeat specifies whether to play PCD file in an endless loop or not.
          @param timestamp time stamp to render the files.
          @param forceExactPCLType if this flag is set to true, the PointCloudObjectBase-reference
          given to grab must have exactly the same fiels as the pcl-file
          @param offset Similar to the offset variable in the file pcd_io.h.
      */
      PCDFileGrabber(const std::string &filepattern="", bool loop = true);

      /// Destructor
      virtual ~PCDFileGrabber();

      /// grab implementation
      virtual void grab(PointCloudObjectBase &dst);
    };
  }
}
