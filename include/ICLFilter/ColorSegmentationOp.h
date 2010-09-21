/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/ColorSegmentationOp.h                **
** Module : ICLFilter                                              **
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
#ifndef ICL_COLOR_SEGMENTATION_OP_H
#define ICL_COLOR_SEGMENTATION_OP_H

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Img.h>

namespace icl{
  /// Class for fast LUT-based color segmentation
  /** \section GEN General Information
      Color segmentation is a very common issue in computer vision applications. The ColorSegmentationOp
      class implements a common and very efficient LUT based segmentation algorithm. It can be used
      to apply any possible segmentation where the classification of one pixel is only influenced by that
      pixels color value

      
      \section ALGO The Segmentation Algorithm
      
      Please consider an input color image I. In a first step, I is converted into depth8u and
      the specified segmentation-color-format (this can be set in the constructor or with the
      corresponding setter-function ColorSegmentationOp::setSegmentationFormat). This conversion
      is skipped if the given image parameters are already correct. Now, every pixel pI = (A,B,C) 
      (e.g. if the segmentation format is formatRGB, pI=(R,G,B)) is classified by applying
      a lookup transformation on it. Basically, we use a lookup table of the whole segmentation
      color space for this. This implies that size of the lookup table becomes 256*256*256. In order
      to reduce the amount of cache misses when picking elements of this 16MB LUT, it is possible
      to adjust the number of bits that are used for each color channel. In the example above, 
      8 bits (256 values) are used. If the number of bits are shifted (using the constructors 
      channel-shift parameters or the setter function ColorSegmentationOp::setSegmentationShifts),
      the size of the lookup table becomes smaller and therefore, the lookup operation becomes 
      faster (see \ref BENCH).
      Another advantage of using the channel-shifts in order to reduce the bits used to represent
      a certain image channel is that is also provides a better generalization. Consider the
      following example (see \ref EX)
      
      \section EX Example for the Algorithm
      <b>Segmentation format: YUV, Shifts = (8,0,0).</b> \n
      In this case, the Y (brightness) information is not used for segmentation at all. Therefore
      the pixel classification does not depend on the pixels brightness, which is very useful for
      color-segmentation. The size of the used lookup table becomes 256*256 which is 65KB. 
      As the noise level of common cameras is still quite high, it might also be possible to use
      only the 7 (or even only the 6) most significant bits for the two color channels U and V 
      without a significant reduction of the segmentation quality.

      \section LUT How to Define the Segmentation Lookup Table
      Another important question that has not been answered yet is, the lookup table can
      be set in order to achieve a certain segmentation result. 
      Consider you want to segment a yellow ball in front of a black background. Now, you simply
      have to set the lookup table values for all possible colors of this ball to a value 
      that is different from 0 (so maybe 200). Then the segmentation result will be white where
      the ball was, and black (value 0) otherwise. But how can you set up the lookup table values?
      Of course, it might be quite hard to find all possible colors of the ball, but it might
      be easy to pick some of its color e.g. by mouse. If you once have these <em>color prototype
      pixels</em>, you can now use one of the the ColorSegmentationOp::lutEntry functions 
      in order to set up all corresponding lookup table entries to your class label (which was
      200 in the example above). The lutEntry methods also provide the functionality to not only
      set up the prototype-pixel's class-labels, but also the class-lables of all pixels within
      the vicinity of the prototype pixels. So sometimes, if your 'to-be-segmented'-yellow ball
      is quite homogeneously yellow, it can be sufficient to add only a single average yellow 
      prototype pixel with high radii for the single color components.\n
      If you don't want to implement mouse handling in order to be able to click at your
      objects to get color-prototypes, you can also use the ICL-example tool 'icl-color-picker'.

      \section SEVERAL Using more than one Class
      
      By adding color prototypes for different objects, but with different class labels, 
      you can also use this class to segment several objects at once. In the next step,
      you will possibly use an instance of ICLBlob/RegionDetector to extract the segemented
      image regions. Here, you can then obtain the corresponding class label by using the
      ImageRegion's getVal method.
      Please note that always the last lut-entry is used if your prototype-entries overlap.

      \section REST Restrictions 
      Maybe it is now, if not before 
      necessary to mention, that the ColorSegmentationOp can only be set up to classify pixels
      into 255 valid classes. But actually, this should not become a problem at all as the 
      classification quality usually restricts the number of classes to a maximum of about 10.

      \section BENCH Benchmark Results

  */
  class ColorSegmentationOp : public UnaryOp{
    public:
    /// Internally used class
    class LUT3D; 

    private:
    format m_segFormat;       //!< format, that is used for internal segmentation
    Img8u m_inputBuffer;      //!< internal image in depth8u and segmentation format
    Img8u m_outputBuffer;     //!< internal buffer holding the output image
    Img8u m_segPreview;       //!< internal buffer for providing a preview of the current segmentation 
    Img8u m_lastDst;          //!< last used destination image
    icl8u m_bitShifts[3];     //!< bit shifts for all 8-Bit channels
    LUT3D *m_lut;             //!< color classification lookup table

    public:
    
    /// Creates a new instance of this class with given segmentation parameters
    /** @param c0shift number of least significant bits that are removed from channel0 
        @param c1shift number of least significant bits that are removed from channel1 
        @param c2shift number of least significant bits that are removed from channel2 
        @param fmt internally used segmentation format (needs to have 3 channels)
        */
    ColorSegmentationOp(icl8u c0shift=2, icl8u c1shift=2, icl8u c2shift=2,format fmt=formatYUV) throw (ICLException);

    /// Destructor
    ~ColorSegmentationOp();
    
    /// main apply function
    virtual void apply(const ImgBase *src, ImgBase **dst);
    
    /// Imported apply from parent UnaryOp class
    UnaryOp::apply;
    
    /// clears the whole segmentation LUT
    void clearLUT(icl8u value=0);
    
    /// sets the segmentation shifts
    void setSegmentationShifts(icl8u c0shift, icl8u c1shift, icl8u c2shift);
    
    /// sets the internally used segmentation format
    void setSegmentationFormat(format fmt) throw (ICLException);

    /// returns the pointer to the 3 internally used segmentation shifts
    const icl8u *getSegmentationShifts() const { return m_bitShifts; }
    
    /// returns the current internally used segmentation format
    format getSegmentationFormat() const { return m_segFormat; }
    
    /// returns in internally managed image of the segmentation result
    /** The image has the dimensions 2^(8-shift0) x 2^(8-shift1) and it
        has 2^(8-shift3) channels */
    const Img8u &getSegmentationPreview();
    

    /// given a,b and c in segFormat, this function fills the LUT within a sub-volume with given radii
    void lutEntry(icl8u a, icl8u b, icl8u c, icl8u rA, icl8u rB, icl8u rC, icl8u value);
    
    /// given a,b and c in format fmt, this function fills the LUT within a sub-volume with given radii
    void lutEntry(format fmt, int a, int b, int c, int rA, int rB, int rC, icl8u value) throw (ICLException);
    
    /// loads the segmentation LUT only (no other parameters)
    void load(const std::string &filename);
    
    /// saves the segmentation LUT only (no other parameters)
    void save(const std::string &filename);
    
    /// this also creates a preview for the current segmentation LUT
    /** Here, you can choose, which dimension shall become the resulting images
        with and height. The remaining 3rd dimension is slices with given zValue 
    */
    const Img8u &getLUTPreview(int xDim, int yDim, icl8u zValue);

  };

}

#endif
