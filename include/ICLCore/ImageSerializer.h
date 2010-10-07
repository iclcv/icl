/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCore/ImageSerializer.h                      **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_IMAGE_SERIALIZER_H
#define ICL_IMAGE_SERIALIZER_H

#include <ICLCore/ImgBase.h>
#include <ICLCore/Types.h>
#include <ICLUtils/Exception.h>
#include <vector>

namespace icl{
  
  /// Utility class for binary Image Serialization
  /** Images are serialized into one binary data block as follows:
      <pre>
      Whole-Image:
      [Header-Block][Channel-0-Data][Channel-1-Data][  ... ][Meta-Data]
      <- 44 Bytes -><- #Channels x dim x sizeof(datatype) -> 
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
  struct ImageSerializer{
    
    /// Internally used type for image headers
    typedef std::vector<icl8u> ImageHeader;
    
    /// Type for meta-data
    typedef std::vector<icl8u> MetaData;
    
    /// returns the size of an image header (in Bytes)
    static int getHeaderSize();
    
    /// estimates the size for the image data of an serialized image
    static int estimateImageDataSize(const ImgBase *image) throw (ICLException);

    /// estimates the full size of an serialized image (without meta-data)
    static int estimateSerializedSize(const ImgBase *image, const MetaData &meta=MetaData()) throw (ICLException);
    
    /// creates an image header from given image
    static ImageHeader createHeader(const ImgBase *image) throw (ICLException);

    /// serializes an image into given destination data-points (which has to be long enough)
    static void serialize(const ImgBase *image, icl8u *dst, 
                          const ImageHeader &header=ImageHeader(),
                          const MetaData &meta=MetaData()) throw (ICLException);
    
    /// serializes an image into given vector (the vector size is adapted automatically)
    static void serialize(const ImgBase *image, std::vector<icl8u> &data,
                          const ImageHeader &header=ImageHeader(),
                          const MetaData &meta=MetaData()) throw (ICLException);
    
    /// deserializes an image (and optionally also the meta-data) from given icl8u data block
    static void deserialize(const icl8u *data, ImgBase **dst, MetaData *dstMeta=0) throw (ICLException);



  };
}

#endif

