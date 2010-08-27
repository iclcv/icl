#ifndef ICL_RUN_LENGTH_ENCODER_H
#define ICL_RUN_LENGTH_ENCODER_H

#include <WorkingLineSegment.h>
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
  class RunLengthEncoder : public Uncopyable{
    /// internal typedef
    typedef WorkingLineSegment WLS;
    
    /// internal data buffer
    std::vector<WLS> m_data;
    
    std::vector<WLS*> m_ends;
    Size m_imageSize;

    template<class T, bool useOffset>
    void encode_internal(const Img<T> &image, const Point &pointOffset);
    
    public:
    RunLengthEncoder(const Size &sizeHint=Size::null);
    
    void prepareForImageSize(const Size &size);
    
    void resetLineSegments();
    
    void encode(const ImgBase *image, const Point &roiOffset=Point::null);
    
    inline WorkingLineSegment *begin(int row){ return &m_data[row*m_imageSize.width]; }
    inline const WorkingLineSegment *begin(int row) const { return &m_data[row*m_imageSize.width]; }

    inline WorkingLineSegment *end(int row){ return m_ends[row]; }
    inline const WorkingLineSegment *end(int row) const { return m_ends[row]; }
  };

}

#endif
