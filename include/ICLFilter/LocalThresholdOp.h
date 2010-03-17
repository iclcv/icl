/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/LocalThresholdOp.h                   **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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
*********************************************************************/

#ifndef LOCAL_THRESHOLD_OP_H
#define LOCAL_THRESHOLD_OP_H

#include <ICLFilter/UnaryOp.h>
#include <ICLUtils/Size.h>
#include <vector>
#include <ICLUtils/Uncopyable.h>

namespace icl{

  /** \cond */
  class IntegralImgOp;
  class UnaryCompareOp;
  class BinaryCompareOp;
  /** \endcond*/
  
  /// LocalThreshold Filter class \ingroup UNARY
  /** The LocalThresholdOp implements a set of local threshold algorithms. Currently, 
      three implementations are available:
      - <b>regionMean:</b> In this case the actual mean of a surrounding region of a pixel
        is used to determine its threshold
      - <b>tiledNN:</b> Here, a very simple tiled threshold is used. The image is split
        into square tiles with edge-size <tt>2*maskSize</tt>. The threshold of all pixels
        within this tile is set relatively to the mean-value of the tile
      - <b>tiledLIN:</b> This is comparable with tiledNN, except, the actual threshold of
        a pixel is computed from a linear interpolation between its 4-nearest tiles mean-values
      
      \section RM Region Mean Algorithm
      
      This is the most sophisticated algorithms. Its implementation  bases on the calculation 
      of an integral image of the given input image (see icl::IntegralImgOp)\n
      Once having access to the integral image data, it is possible to calculate the mean of an
      arbitrary rectangular image region in constant time. Consider the following ASCII example.
      A rectangular image region is always described by its four corner points A,B,C and D
      <pre>
      .C....A...
      ..+++++...
      ..+++++...
      ..+++++...
      ..+++++...    
      .B++++D...
      ..........
      </pre>      

      The "mass" of pixels inside the rect can be obtained by calculating
      \f[X = D - A - B + C\f] in the integral image. The pixel count in the region is 
      \f[P = (A.x-C.x) * (B.y-C.y) \f], which directly leads to region-mean \f$X/P\f$.

      \section T__ Threshold
      A local image threshold at an image location \f$p=(px,py)\f$ must factor in the local image intensity, which can be 
      approximated by the mean value \f$\mu_p\f$ of the square region centered at \f$p\f$ with a certain radius \f$r\f$. 
      To emphazise that \f$\mu_p\f$ depends also on \f$r\f$ we denote it by \f$\mu_r{p}\f$.
      To have more influence on the actually used threshold at location \f$p\f$, we used an additive global threshold 
      variable \f$g\f$.
      
      \section A__ Algorithm
      <pre>
      Input: Input-Image i, region radius r, global threshold g, destination image d
      
      1. I := integral image of i
      2. (static) calculate region size image S
         This is nessesary because border regions do not have the expected region size
         (2*r+1)² due to the overlap with the image border. The calculation of S is not
         very complex, so it is not discussed here further.
      3. I := extend (I). To avoid accessing invalid pixel locations of the integral image,
         it is enlarged to each edge by r. To garantee, that this optimization does not lead
         to incorrect results, the upper and the left border of the integral image is filled with
         zeros, and the lower and the right edge is filled with the value of the nearest non-
         border pixel.
      4. Pixel Loop:
         For each pixel (x,y) of i
            4.1 Estimate the corresponding edge points A,B,C and D of the squared neighbourhood.
                (do not forget that the integral image has got a r-pixle border to each edge)
                C := (x-r,y-r)
                A := (x+r,y-r)     
                B := (x-1,y+r)
                D := (x+r,y+r)
            4.2 Calculate the region mean with respect to the underlying region size
                M = (I(D) - I(A) - I(B) + I(C))/S(x,y)
            4.3 Calculate the theshold operation
                d(x,y) = 255 * (i(x,y) > (M+g) )
          endfor
      </pre>

      \section M__ Mutli channel images
      This time, no special operation for multi channels images are implemneted, so each channel
      is process independently in this case.
  
      
      \section GAMMA Experimental feature gamma slope
      By applying a small adaption, the procedure presented avoid can be used to 
      apply a local gamma correction on a source image. We just have to exchange the "stair"-function
      above \f$ d(x,y) = 255 * (i(x,y) > (M+g) ) \f$ by a linear function:
      
      <pre>
      using a linear function function f(x) = m*x + b    (with clipping to range [0,255])
      with m = gammaSlope (new variable)
           k = M+g
           that f(k) = 128
         
         f(x) = m(x-k)+128     (clipped to [0,255])
      </pre>

      \section RES Results
      
      The RegionMean algorithm clearly provides the best results of the three algorithms. The nearest neighbour
      interpolation algorithm is not significantly faster than the linear version, however it's results are much
      worse. Here is a set of results images. All images use a tile size of 30 and a global threshold of 3
      
      \image html threshold-region-mean.png "RegionMean" 
      \image html threshold-tiled-lin.png "tiled-LIN" 
      \image html threshold-tiled-nn.png "tiled-NN" 

      \section BENCHMARKS Benchmarks
      
      All threshold implementations are fast and highly optimized. In particular, we spend a lot of time
      for the optimization of the RegionMean algorithms, which provides the best results.
      Benchmarks were performed on a 2GHz Intel Core2Duo Machine with 2GB Ram. Compiler g++ 4.3, optimization 
      flags -O4 -march=native.
      <table>
      <tr> <td>MaskSize</td> <td>tiledNN</td> <td>tiledLIN</td> <td>RegionMean</td> </tr>
      <tr> <td>200</td> <td>5</td> <td>5</td> <td>13ms</td> </tr>
      <tr> <td>100</td> <td>5</td> <td>5</td> <td>11ms</td> </tr>
      <tr> <td>40</td> <td>5</td> <td>5</td> <td>10ms</td> </tr>
      <tr> <td>20</td> <td>5</td> <td>5</td> <td>9ms</td> </tr>
      <tr> <td>10</td> <td>5</td> <td>5</td> <td>9ms</td> </tr>
      <tr> <td>5</td> <td>6</td> <td>6</td> <td>9ms</td> </tr>
      <tr> <td>2</td> <td>9</td> <td>9</td> <td>9ms</td> </tr>
      </table>
      
      The experimental gamma-slope-computation is much more expensive: Here, the RegionMean algorithms
      needs about 25ms.
  */
  class LocalThresholdOp : public UnaryOp, public Uncopyable{
    public:
    
    /// Internally used algorithm 
    enum algorithm{
      regionMean, //!< regionMean threshold calculation
      tiledNN,    //!< tiled threshold with nearest neighbour interpolation
      tiledLIN    //!< tiled threshold with linear interpolation
    };
    
    /// create a new LocalThreshold object with given mask-size and global threshold and RegionMean algorithm
    /** @param maskSize size of the mask to use for calculations, the image width and
                        height must each be larger than 2*maskSize.
        @param globalThreshold additive Threshold to the calculated reagions mean
        @param gammaSlope gammaSlope (Default=0) (*Experimental feature*)
                          if set to 0 (default) the binary threshold is used
    */
    LocalThresholdOp(unsigned int maskSize=10, float globalThreshold=0, float gammaSlope=0);

    
    /// creates a LocalThresholdOp instance with given Algorithm-Type
    LocalThresholdOp(algorithm a, int maskSize=10, float globalThreshold=0, float gammaSlope=0);

    
    /// Destructor
    ~LocalThresholdOp();
    
    /// virtual apply function 
    /** roi support is realized by copying the current input image ROI into a 
        dedicate image buffer with no roi set
    **/
    virtual void apply(const ImgBase *src, ImgBase **dst);

    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    /// set a new mask size (a new mask size image must be calculate in the next apply call)
    void setMaskSize(unsigned int maskSize);
    
    /// sets a enw global threshold value to used
    void setGlobalThreshold(float globalThreshold);

    /// sets a new gamma slope to used (if gammaSlope is 0), the binary threshold is used
    void setGammaSlope(float gammaSlope);
    
    /// sets all parameters at once
    void setup(unsigned int maskSize, float globalThreshold, algorithm a=regionMean, float gammaSlope=0);
    
    /// returns the current mask size
    unsigned int getMaskSize() const;

    /// returns the current global threshold value
    float getGlobalThreshold() const;

    /// returns the current gamma slope
    float getGammaSlope() const; 
    
    /// returns currently used algorithm type
    algorithm getAlgorithms() const ;
    
    /// sets internally used algorithm
    void setAlgorithm(algorithm a);

    private:

    /// internal algorithm function
    template<algorithm a>
    void apply_a(const ImgBase *src, ImgBase **dst);
    
    /// mask size 
    unsigned int m_maskSize;

    /// global threshold
    float m_globalThreshold;

    /// gamma slope
    float m_gammaSlope;
    
    /// input ROI buffer image for ROI support
    ImgBase *m_roiBufSrc;

    /// output ROI buffer image for ROI support
    ImgBase *m_roiBufDst;
    
    /// IntegralImgOp for RegionMean algorithm
    IntegralImgOp *m_iiOp;

    /// currently used algorithm
    algorithm m_algorithm;

    /// BinaryCompareOp for tiledXXX algorithms
    BinaryCompareOp *m_cmp;

    /// first buffer for tiledXXX algorithsm
    ImgBase *m_tiledBuf1;

    /// second buffer for tiledXXX algorithsm
    ImgBase *m_tiledBuf2;

  };
  
}  
#endif
