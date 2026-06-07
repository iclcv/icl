// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Eckard Riedenklau, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/source/SourceBackend.h>

namespace icl::io {
  /// PixelSense SourceBackend class
  /** This grabber can be used to interface with devices implementing the
      Microsoft PixelSense technology, such as the Samsung SUR40 device.

      The grabbed images will contain meta data which describes the
      natively detected blobs. To extract the meta-data as a vector of Blobs,
      the static PixelSenseSource::extractBlobMetaData method can be used
      */
  class PixelSenseSource : public SourceBackend{
    struct Data;   //!< internal data structure
    Data *m_data;  //!< internal data pointer

    public:

    /// Blob structure
    /** Please note: positions and sizes are given in screen coordinates.
        corresponding image coords can be obtained by deviding by two */
    struct Blob{
      icl16u id;           //!< blob ID
      icl8u  action;       //!< 0x2 enter/exit, 0x3 update?
      icl8u  __unknown;    //!<  always 0x01 or 0x02 (no idea what this is?)
      icl16u bbx;          //!< upper left x of bounding box (in screen coords)
      icl16u bby;          //!< upper left y of bounding box (in screen coords)
      icl16u bbwidth;      //!< bounding box width (in screen coords)
      icl16u bbheight;     //!< bounding box height (in screen coords)
      icl16u posx;         //!< finger tip x-pos (in screen coords)
      icl16u posy;         //!< finger tip y-pos (in screen coords)
      icl16u cx;           //!< x-center (in screen coords);
      icl16u cy;           //!< y-center (in screen coords);
      icl16u axisx;        //!< x-axis (related to first principal axis)
      icl16u axisy;        //!< y-axis (related to 2nd principal axis)
      icl32f angle;        //!< angle of first principal axis
      icl32u area;         //!< size in pixels (correlated to pressure?)
      icl8u  padding[32];  //!< padding bytes (unused)
    };

    /// default grab function
    ICLIO_API core::Image acquireImage() override;

    /// Create a PixelSenseSource with given max. fps count
    ICLIO_API PixelSenseSource(float maxFPS = 30);

    /// destructor
    ICLIO_API ~PixelSenseSource();

    /// this utility method can be used to extract the meta-data of a grabbed image
    ICLIO_API static std::vector<Blob> extractBlobMetaData(const core::ImgBase *image);
  };

  /// overloaded ostream operator for the PixelSenseSource::Blob type
  /** concatenates relevant information using a comma delimiter */
  ICLIO_API std::ostream &operator<<(std::ostream &str, const PixelSenseSource::Blob &b);

  /// overloaded istream operator for the PixelSenseSource::Blob type
  /** relevant information is assumed to be comma delimited */
  ICLIO_API std::istream &operator>>(std::istream &str, PixelSenseSource::Blob &b);

  } // namespace icl::io