/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImageSerializer.h                  **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
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
#include <ICLCore/ImgBase.h>
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
      typedef std::vector<icl8u> ImageHeader;

      /// returns the size of an image header (in Bytes)
      static int getHeaderSize();

      /// estimates the size for the image data of an serialized image
      static int estimateImageDataSize(const ImgBase *image) throw (utils::ICLException);

      /// estimates the full size of an serialized image
      static int estimateSerializedSize(const ImgBase *image, bool skipMetaData=false) throw (utils::ICLException);

      /// creates an image header from given image
      static ImageHeader createHeader(const ImgBase *image) throw (utils::ICLException);

      /// serializes an image into given destination data-points (which has to be long enough)
      static void serialize(const ImgBase *image, icl8u *dst,
                            const ImageHeader &header=ImageHeader(),
                            bool skipMetaData=false) throw (utils::ICLException);

      /// serializes an image into given vector (the vector size is adapted automatically)
      static void serialize(const ImgBase *image, std::vector<icl8u> &data,
                            const ImageHeader &header=ImageHeader(),
                            bool skipMetaData=false) throw (utils::ICLException);

      /// deserializes an image (and optionally also the meta-data) from given icl8u data block
      static void deserialize(const icl8u *data, ImgBase **dst) throw (utils::ICLException);

      /// extracts only an images TimeStamp from it's serialized form
      static utils::Time deserializeTimeStamp(const icl8u *data) throw (utils::ICLException);

    };
  } // namespace core
}

