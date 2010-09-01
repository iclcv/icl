/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLBlob/RunLengthEncoder.h                     **
** Module : ICLBlob                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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


#ifndef ICL_RUN_LENGTH_ENCODER_H
#define ICL_RUN_LENGTH_ENCODER_H

#include <ICLBlob/WorkingLineSegment.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <vector>

namespace icl { 

  /// Simple class for creation of a run-length encoding of an image
  /** 
      \section GENERAL General Information
      
      Run-length encoding means that each image line is transformed into
      a set of so called LineSegments. Each of these Segments represents
      a sequence of adjacent image pixels that have the same value. By this means,
      uniform image Regions can be compressed significantly. However, if there
      are only short LineSegments, run-length-encoding might provide a 
      larger encoding. 
  
      
      \section WLS WorkingLineSegments
      
      The RunLengthEncoder class uses WorkingLineSegments internally, where
      each of these subsumes the following information
      * xstart (x-coordinate of the left-most point of the line segment)
      * xend (x-coordinate of the pixel right to the right-most pixel of the line segment)
      * y (the row index)
      * value (the underlying value)
      * anyData (a pointer to addinional information that might be used by other algorithms
        that use the RunLengthEncoder class

      \section USAGE Usage
      The RunLengthEncoder's main function is RunLengthEncoder::encode(const Img<T>&, const Point&).
      After encode was called, the run-length-encoded representation of each image line can be accessed
      using RunLengthEncoder::begin(int) and RunLengthEncoder::end(int).
      
      \section DEPTHS Image Depths
      As the used WorkingLineSegment class provides only an int-value parameter,
      the RunLengthEncoder is <b>not</b> able to process icl32f and icl64f images

      \section ROI ROI Support
      The RunLengthEncoder does not support image ROIs, as most of the time it might be easier to extract the
      source images ROI beforehand. If the given image is acutally just a copy of another images ROI, the
      2nd optional parameter of the RunLengthEncoder::encode(const ImgBase*, const Point&) can be set.
      In this case, the give offset will be added automatically to the resulting scanline values
      xstart, xend and y
  */
  class RunLengthEncoder{
    /// internal typedef
    typedef WorkingLineSegment WLS;
    
    /// internal data buffer
    std::vector<WLS> m_data;
    std::vector<WLS*> m_ends;
    Rect m_imageROI;

    template<class T>
    void encode_internal(const Img<T> &image);

    void prepare(const Rect &roi);
    void resetLineSegments();

    inline int idx(int row, bool relToImageOffset) const {
      if(relToImageOffset){
        return (row-m_imageROI.y)*m_imageROI.width;
      }else{
        return row*m_imageROI.width;
      }
    }
    public:
    
    void encode(const ImgBase *image);
    
    // all indices are relative to the image ROI's offset
    inline WorkingLineSegment *begin(int row, bool relToImageOffset=false){ 
      return &m_data[idx(row,relToImageOffset)];
    }
    
    inline const WorkingLineSegment *begin(int row, bool relToImageOffset=false) const { 
      return &m_data[idx(row,relToImageOffset)];
    }

    inline WorkingLineSegment *end(int row, bool relToImageOffset=false){ 
      return m_ends[relToImageOffset ? row - m_imageROI.y : row]; 
    }
    
    inline const WorkingLineSegment *end(int row, bool relToImageOffset=false) const { 
      return m_ends[relToImageOffset ? row - m_imageROI.y : row]; 
    }
  };

}

#endif
