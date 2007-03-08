#include <ICLPoint.h>
#include <ICLSize.h>
#include <ICLRect.h>
#include <ICLTypes.h>
#include <ICLException.h>
#include <ICLMacros.h>
#ifndef IMG_PARAMS_H
#define IMG_PARAMS_H


namespace icl{
  /// The ImgParams class stores all image parameters
  /** This class offers an encapsulation of all Img parameters
      - size
      - channels
      - format
      - roi
     
      The class helps to create image processing classes getting
      certain image parameters. This parameters can now directly 
      be aquired from an ImgBase object. In addition to this, except
      to the ImgBases underlying depth (which is commited to the
      depth of the underlying Img object) Images can be made
      compatible by calling imageA.setParams(imageB.getParams())
  */
  class ImgParams{
    public:
    /// as default size=(0,0), channels=0, format=matrix, roi=(0,0,0,0)
    static const ImgParams null;


    /// creates a null ImgParams object
    /** @see setup
        @see null   
    */
    ImgParams(const ImgParams &params=null)
       { setup(params.getSize(), params.getFormat(), params.getChannels(), params.getROI()); }
    
    /// creates an ImgParams object with specified size, channels, roi and formatMatrix
    /** @see setup */
    ImgParams(const Size &size, int channels, const Rect &roi = Rect::null)
       { setup(size,formatMatrix,channels,roi); }

    /// creates an ImgParams object with specified size, channels, roi and format
    /** @see setup*/
    ImgParams(const Size &size, format fmt, const Rect &roi = Rect::null);

    /// creates an ImgParams object with all given parameters
    /** Note that channels and format are <b>not independent</b>. Hence, if the given
        channel count is not compatible to the given format, an exception is thrown.
        @see setup
    */
    ImgParams(const Size &size, int channels, format fmt, const Rect& roi = Rect::null)
       { setup(size,fmt,channels,roi); }

    /// creates an ImgParams object with specified size, format and roi given as POD-Types
    /** channel count is adapted to the given format
        @see setup
    */
    ImgParams(int width, int height, format fmt, 
              int roiX=0, int roiY=0, int roiWidth = 0, int roiHeight = 0);

    /// creates an ImgParams object with specified size, channel and roi given as POD-Types
    /** The format is set to "formatMatrix" 
        @see setup
    */
    ImgParams(int width, int height, int channels, 
              int roiX=0, int roiY=0, int roiWidth = 0, int roiHeight = 0);


    /// creates an ImgParams object with ALL possible parameters
    /** Note that channels and format are <b>not independent</b>. Hence, if the given
        channel count is not compatible to the given format, an exception is thrown.
        @see setup
    */
    ImgParams(int width, int height, int channels, format fmt, 
              int roiX=0, int roiY=0, int roiWidth=0, int roiHeight=0);

    /// checks wether the object instance is null, i.e. all elements are zero
    bool isNull() const { return (*this)==null; }
    
    /// returns !(*this==other)
    bool operator!=(const ImgParams &other) const{ return !((*this)==other); } 

    /// test is all parameters (size, roi, channels, format) are identical
    bool operator==(const ImgParams &other) const;
    
    /// sets the size to the current value (and resets the roi to null)
    void setSize(const Size &size);
   
    /// sets the format to the given format (the channel count is adapted on demand)
    void setFormat(format fmt);

    /// sets the channels to the given channel count (format is set to "formatMatrix" on demand)
    void setChannels(int channels);
    
    /// sets the image ROI offset to the given value
    /** If the offset is not inside of the image or the new offset causes the roi 
        not to fit into the image, nothing is done, and an exception is thrown.
    */
    void setROIOffset(const Point &offset);
         
    /// sets the image ROI size to the given value
    /** If the new roi size causes the roi not to fit into the image, nothing is done
        and an exception is thrown
    */
    void setROISize(const Size &roisize);
     
    /// set both image ROI offset and size
    /** This function evaluates if the new offset is inside of the image, as well as
        if the resulting roi does fit into the image.
    */
    void setROI(const Point &offset, const Size &roisize);
    
    /// sets the image ROI to the given rectangle
    void setROI(const Rect &roi) { setROI (roi.ul(),roi.size()); }
   
    /// checks, eventually adapts and finally sets the image ROI offset
    void setROIOffsetAdaptive(const Point &offset);
      
    /// checks, eventually adapts and finally sets the image ROI size
    void setROISizeAdaptive(const Size &size);

    /// as setROI, but if checks for negative parameters
    /** While the methods setROI, setROIOffset and setROISize directly set
        the images ROI from the given arguments (if possible), the following methods 
        adapt the ROI parameters to assure a valid ROI. Negative values are interpreted 
        relative to the whole image size resp. the lower right corner of the image.
    
        E.g. an offset (5,5) with size (-10,-10) sets the ROI to the inner
        sub image with a 5-pixel margin. offset(-5,-5) and size (5,5) sets
        the ROI to the lower right 5x5 corner. 
    **/
    void setROIAdaptive(const Rect &r);
     
    /// adapt given ROI, such that it fits for the current ImgParams
    /** @see setROIAdaptive **/
    Rect& adaptROI(Rect &roi) const;

    /// returns ROISize == ImageSize
    bool hasFullROI() const { return m_oROI.size() == m_oSize;}

    /// sets the ROI to 0,0,image-width,image-height
    void setFullROI(){ setROI(Point::null, getSize()); }

    /// returns the objects size
    const Size& getSize() const { return m_oSize;  }
    
    /// returns the objects channel count
    int getChannels() const { return m_iChannels; }

    /// returns the object format
    format getFormat() const { return m_eFormat; }
    
    /// returns the objects ROI rect
    const Rect& getROI() const { return m_oROI; }
    
    /// copies the roi parameters into the given structs offset and size
    void getROI(Point &offset, Size &size) const { 
       offset=getROIOffset(); size = getROISize(); 
    } 

    /// returns the objects ROI offset
    const Point getROIOffset() const { return m_oROI.ul(); }

    /// returns the objects ROI size 
    const Size getROISize() const{ return m_oROI.size(); }

    /// returns the objects image width
    int getWidth() const { return m_oSize.width; }
    
    /// return the objects image height
    int getHeight() const{ return m_oSize.height; }

    /// returns ROI-dependent pixel offset, to address the upper left ROI pixel
    int getPixelOffset() const { return m_oROI.x+m_oROI.y*getWidth(); }

    /// returns the ROI width of the object
    int getROIWidth() const{ return m_oROI.width; }

    /// returns the ROI height of the object
    int getROIHeight() const{ return m_oROI.height; }

    /// returns the ROI X-Offset of the object
    int getROIXOffset() const{ return m_oROI.x; }

    /// returns the ROI Y-Offset of the object
    int getROIYOffset() const { return m_oROI.y; }

    /// returns the count of image pixels (width*height)
    int getDim() const { return getSize().getDim(); }

    /// returns the count of ROI pixels ( ROI_width*ROI_height )
    int getROIDim() const { return getROISize().getDim(); }

    private:
    /// initialisation function 
    /** The initialisation function does <em>all the magic</em> that is necessary to
        setup all parameters correctly. There are two issues, that must be treated in 
        a special way:
        1. size must be positive in [0,inf) x [0,inf)
        2. set up the format and channel count correctly
        3. ensure, that the roi has a valid size and position


        <h3>Format and Channel Count</h3>
        The parameters format and channel count depend on each other. The channel count
        is fixed and well defined for all allowed formats except "formatMatrix", where
        the channel count is arbitrary including zero. So, if the given format distincts
        from formatMatrix, the given channel count is disobeyed and the objects channel
        count is set to the channel count associated with the given format (e.g. the
        "formatRGB" has three channels). The association is performed by the global 
        icl-namespace function <em>getChannelsOfFormat</em> (see the functions documentation
        for more details. If the given format is "formatMatrix", then the channel count is
        checked to be positive (in [0,inf)) to avoid errors, that would occur, if e.g. -5
        channels are allocated at runtime.
     
        <h3>Ensure a Valid ROI Size</h3>
        In total, there are three problems, that need to be tackled when the ROI is set.
        - <b>Check if the given roi is null:</b> if the given roi is <em>null</em> (this
          means the rect has offset (0,0) and size (0,0)), then the objects ROI is
          set up to cover the whole image rect (offset (0,0) and size 
          (image-width,image-height)). If it is <b>not</b> <em>null</em>, the following two
          issues must be regarded.
        - <b>Check if the given offset is inside of the image:</b> This should be
          self-evident. If the offset is outside the image rect, it is set to (0,0), and
          furthermore a specific <b>exception</b> is thrown.
        - <b>Check if the given roi size is valid:</b> This can be devided into two parts:
          1. The ROI size must be positive (in [0,inf) x [0,inf)). 
             If not, the roi is set to null (whole image) and an error is thrown.
          2. The ROI must <em>fit</em> into the image. 
             Otherwise the ROI is set to null (whole image) and an error is thrown.
    */
    void setup(const Size &size, format fmt, int channels, const Rect &roi);


    /// image size
    Size m_oSize;

    /// image channel count
    int m_iChannels;

    /// image format (formatRGB, formatMatrix, ...)
    format m_eFormat;

    /// image roi
    Rect m_oROI;
  };
}

#endif
