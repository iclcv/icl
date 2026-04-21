// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Range.h>
#include <icl/utils/Uncopyable.h>
#include <icl/core/Color.h>
#include <icl/cv/ImageRegion.h>

namespace icl::cv {
  /// Utility class for region-based colored blob detection
  /** The SimpleBlobSearcher is a re-implementation of the
      RegionBasedBlobSearcher class. It provides less features, but
      it's usability was increased significantly.

      \section ALGO The Detection Algorithm

      \subsection PRE Prerequisites
      Given: A set of reference color tuples \f[\{R_i | i \in \{1,N\} \}\f]
      with \f[R_i=(\mbox{RC}_i,\mbox{T}_i,\mbox{S}_i,\mbox{RD}_i, \mbox{BUF}_i)\f]
      where
      - <b>RC</b> is the actual reference color for blobs that a detected
      - <b>T</b> is the Euclidean color distance tolerance for that reference color
      - <b>S</b> contains min- and maximum pixel count of detected blobs of that color
      - <b>RD</b> is an internal RegionDetector instance, dedicated for that color
      - <b>BUF</b> is an internal image Image for that color

      And a given image \f$I\f$

      \subsection ACALGO Actual Algorithm (Step by Step)

      - clear(OUTPUT_DATA)
      - SIZE <- size(I)
      - set_size(\f$\mbox{BUF}_i\f$,SIZE)
      - for all pixels (X,Y) in I
        - \f$ \mbox{BUF}_i \leftarrow 255 * ( |I(x,y)-\mbox{RC}_i| < T_i )\f$
      - for \f$i = i .. N\f$
        - \f$\mbox{Regions}_i \leftarrow \mbox{RD}_i.detect(\mbox{BUF}_i)\f$
        - for all \f$r_j \in \mbox{Regions}_i\f$
          - OUTPUT_DATA.add(SimpleBlobSearcher::Blob(\f$r_j\f$, \f$RC_i\f$, i)
  */
  class ICLCV_API SimpleBlobSearcher {
    public:
    SimpleBlobSearcher(const SimpleBlobSearcher&) = delete;
    SimpleBlobSearcher& operator=(const SimpleBlobSearcher&) = delete;


    /// Internal blob-result type
      struct ICLCV_API Blob{
      Blob():region(0),refColorIndex(-1){}
      Blob(const ImageRegion *region, const core::Color &refColor, int refColorIndex);

      /// corresponding regions (provides access to region features)
      const ImageRegion *region;

      /// corresponding reference color
      core::Color refColor;

      /// corresponding ref color index
      int refColorIndex;
    };

    /// Default constructor
    SimpleBlobSearcher();

    /// Destructor
    ~SimpleBlobSearcher();

    /// Adds a new reference color, with threshold and size range
    void add(const core::Color &color,
             float thresh,
             const utils::Range32s &sizeRange);

    /// Updates data for given index
    void adapt(int index, const core::Color &color,
               float thresh, const utils::Range32s &sizeRange);

    /// removes reference color at given index
    void remove(int index);
    void clear();

    /// Actual detection function (no ROI support yet!)
    /** detects blobs in given image*/

    const std::vector<Blob> &detect(const core::Img8u &image);

  private:
    /// Internal data representation (hidden)
    struct Data;

    /// internal data pointer
    Data *m_data;
  };


  } // namespace icl::cv