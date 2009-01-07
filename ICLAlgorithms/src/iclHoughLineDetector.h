#ifndef ICL_HOUGH_LINE_DETECTOR_H
#define ICL_HOUGH_LINE_DETECTOR_H

#include <iclHoughLine.h>
#include <iclUnaryOp.h>
#include <iclCannyOp.h>
#include <iclUncopyable.h>

namespace icl{
  /// Image Line etection using IPP's Hough transformation for lines (IPP only)
  /** This class provides the following functionalities:
      - hough transformation for lines (using IPP function)
      - optional preprocessing an input image with given unary op (by default: Canny-edge detector)
      - internal handling of IPP-buffer
      - internal transformation from IppPointPolar line representation into richer HoughLine structure
      - automatic adaption of input-image/preprocessor output to depth8u
      - parameter handling 
         - maximum number of lines searched for
         - angular and radial line parameter steps
         - minimum numver of pixles that are needed to find a line
      
  */
  class HoughLineDetector : public Uncopyable{
    public:
    
    /// create a new HoughLineDetector
    /** @param maxLines maximum number of lines, that will be detected 
        @param sizeHint this value can be given if the maximum size of input images
                        is already known
        @param deltaAngular discretization of angle domain for detected lines
                            e.g. if deltaAngular is pi/2 only vertical and horizontal 
                            lines are detected
        @param deltaRadial discretization of displacement domain of detected lines
        @param minPointsPerLine minimum number of border-pixels in the input-image/
                                preprecessed image that must be accumulated to form 
                                a line
        @param preprocessor optional preprocessing unit (ownerships is handle by
                            a Smart-Pointer)
    */
    HoughLineDetector(int maxLines=1000,
                      const Size &sizeHint=Size::null,
                      float deltaAngular=0.1,
                      float deltaRadial=1.0,
                      int minPointsPerLine = 2,
                      SmartPtr<UnaryOp,PointerDelOp> preprocessor=new CannyOp(240,255,true));
    
    /// extract lines regarding current parameters
    const std::vector<HoughLine> detectLines(const ImgBase *image);

    /// Destructor
    ~HoughLineDetector();

    /// set smax line count (buffers are updated)
    void setMaxLines(int maxLines);
    
    /// get max line count
    inline int getMaxLines() const { return m_maxLines; }

    /// sets new preprocessor
    void setPreprocessor(SmartPtr<UnaryOp,PointerDelOp> preprocessor);

    /// returns current preprocessor
    inline SmartPtr<UnaryOp,PointerDelOp> getPreprocessor() const{
      return m_preprocessor;
    }
    
    /// set up a new size hint
    /** internal buffer is adapted if size's dim becomes larger */
    void setSizeHint(const Size &size);
    
    /// returns current buffer's associated ROI dim
    inline Size getBufferROIDim() const { return m_roiDimForBuffer; }

    /// sets new angular step (buffers are updated) 
    void setDeltaAngular(float da);
    
    /// returns current angular steps
    inline float getDeltaAngular() const { return m_deltaAngular; }

    /// sets new radial step (buffers are updated)
    void setDeltaRadial(float dr);
    
    /// returns current radial step
    inline float getDeltaRadial() const { return m_deltaRadial; }
    
    /// sets new step values (buffers are updated)
    void setDelta(float deltaRadial, float deltaAngular);
    
    /// sets new value for min-points-per-line
    void setMinPointsPerLine(int minPts);
    
    /// returns current min-points-per-line value
    inline int getMinPointsPerLine() const { return m_minPointsPerLine; }
    
    /// returns a const instance of the current preprocessing buffer (for debugging)
    inline const ImgBase *getPreprocessingBuffer() const { return m_preprocessingBuffer; }

    /// returns a const instance of the current depth-8u buffer (for debugging)
    inline const Img8u &getInputBuffer8u() const { return m_inputBuffer8u; }
    
    private:

    /// internally used (updates buffers)
    void updateBufferSize();
    
    int m_maxLines;
    SmartPtr<UnaryOp,PointerDelOp> m_preprocessor;
    std::vector<icl8u> m_buffer;
    Size m_roiDimForBuffer;
    std::vector<HoughLine> m_lines;
    float m_deltaAngular;
    float m_deltaRadial;
    int m_minPointsPerLine;

    void *m_detectionLineBuffer;
    ImgBase *m_preprocessingBuffer;
    Img8u m_inputBuffer8u;
  };
  
}

#endif
