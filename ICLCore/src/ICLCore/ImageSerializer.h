// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Types.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Time.h>
#include <vector>

namespace icl{
  namespace core{

    /// Utility class for binary Image Serialization
    /** Images are serialized into one binary data block as follows:
        <pre>
        Whole-Image:
        [Header-Block][Channel-0-Data][Channel-1-Data][  ... ][Meta-Data]
        <- 44 Bytes -><- Channels x dim x sizeof(datatype) ->
        </pre>
        The Header-Block contains a binary representation of all image properties:
        <pre>
        Header-Block:
        [depth][width][height][format][channels][roi.x][roi.y][roi.width][roi.height][time-stamp]
        <- 4 -><- 4 -><- 4  -><- 4  -><-   4  -><- 4 -><- 4 -><-   4   -><-   4    -><-    8   ->
        (in Bytes)
        </pre>
        Finally the optional meta-data block can be used to attach additional information to
        images:
        <pre>
        Meta-Data-Block:
        [size][Meta-Data]
        <-4 -><- size  ->
        </pre>
    */
    struct ICLCore_API ImageSerializer{

      /// Internally used type for image headers
      using ImageHeader = std::vector<icl8u>;

      /// returns the size of an image header (in Bytes)
      static int getHeaderSize();

      /// estimates the size for the image data of an serialized image
      static int estimateImageDataSize(const ImgBase *image);

      /// estimates the full size of an serialized image
      static int estimateSerializedSize(const ImgBase *image, bool skipMetaData=false);

      /// creates an image header from given image
      static ImageHeader createHeader(const ImgBase *image);

      /// serializes an image into given destination data-points (which has to be long enough)
      static void serialize(const ImgBase *image, icl8u *dst,
                            const ImageHeader &header=ImageHeader(),
                            bool skipMetaData=false);

      /// serializes an image into given vector (the vector size is adapted automatically)
      static void serialize(const ImgBase *image, std::vector<icl8u> &data,
                            const ImageHeader &header=ImageHeader(),
                            bool skipMetaData=false);

      /// deserializes an image (and optionally also the meta-data) from given icl8u data block
      static void deserialize(const icl8u *data, ImgBase **dst);

      /// extracts only an images TimeStamp from it's serialized form
      static utils::Time deserializeTimeStamp(const icl8u *data);

    };
  } // namespace core
}
