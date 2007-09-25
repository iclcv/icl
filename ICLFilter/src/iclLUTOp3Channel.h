#ifndef LUT_OP_3C_H
#define LUT_OP_3C_H

#include "iclUnaryOp.h"
#include "iclImg.h"
#include <vector>
#include <iclUncopyable.h>

namespace icl {

  /// class for applying table look-up transformation to 3-channel integer-valued images \ingroup OTHER
  /** In many applications it is necessary to create a feature map on 3-channel 
      input images. These feature map creation procedure can take much time,
      depending on the complexity of the calculated feature. In a very simple
      case for instance, the LUTOp3Channel can be used to create thresholded 
      color-distance map using the the euclidian distance to a reference color and a
      threshold for each 
      pixel of the source image. This operation can be applied on each input
      (r,g,b)-Tuple by the following function (which is basically implemented in
      the "'Plugin" structure nested in the LUTOp3Channel class:
      \code
      virtual T transform(icl8u v1, icl8u v2, icl8u v3){
        return ::sqrt( ::pow(r-m_aiRGB[0],2) + ::pow(g-m_aiRGB[1],2) + ::pow(b-m_aiRGB[2],2) ) / (::sqrt(3.0)) < thresh ? 255 : 0; 
      }
      \endcode
      This virtual function can be reimplemented by special plugins to calculate
      custom LUT-accelerated maps on icl8u/icl16s and icl32s -source images. The source 
      images must be at least integer-typed, as the LUT, which is used internally has only
      discrete entries.\n
      It is recommended to work with icl8u input images to avoid illegal memory access
      when using the LUT.\n
      
      \section IMPL Implementation
      
      The following procedure is used to create the LUT internally;
      \code
      void create_lut(Plugin p, T lut[16777216]){
        for(int r = 0; r < 256; ++r){
          for(int g = 0; g < 256; ++g){
            for(int b = 0; b < 256; ++b){
              lut[r+ 256*g + 65536*b] = p.transform( r,g,b );
            }
          }
        } 
      }      
      \endcode

      When the LUT is created internally, it can be used for fast feature map calculation
      using a simple table lookup:
      \code
      \#define CAST(x) Cast<srcT,icl8u>::cast(x)       /// !!

      template<class srcT, class dstT>
      void apply_lut_op_3(const Img<srcT> &src, Img<dstT> &dst, dstT *lut){
        const srcT * pSrcR = src.getData(0); 
        const srcT * pSrcG = src.getData(1); 
        const srcT * pSrcB = src.getData(2);
        dstT *pDst = dst.getData(0);
        dstT *pDstEnd = pDst+dst.getDim();
        while(pDst < pDstEnd){
          *pDst++  = lut[ CAST(*pSrcR++) + 256*CAST(*pSrcG++) + 65536*CAST(*pSrcB++) ];
        }
      }
      \endcode

      \section PERF Performance
      
      As it can be seen in section \ref IMPL, each pixel is transformed by a simple 
      table look up, which is performed by 2 multiplications, 2 additions and a simple 
      dereferencing. The more complex part of this calculation is done by the calls to
      the Cast-class template. This is necessary to avoid index overruns due to the 
      higher range of all non-icl8u input images types.
      The performance is much better, when using icl8u input images, because the Cast-
      call is left out in this case.

      \section BENCH Benchmarks
      
      The default Plugins operation of threshold an implicitly created 
      euclidian distance map takes about <b>18msec</b> on an 640x480 3-channel icl8u
      input image. This can be accelerated by 61% to <b>11msec</b> (1.6Mhz Pentium M)
      by using the LUTOp3Channel class. This benefit is not very hight, but the 
      euclidian distance function is only one possible function
      \f[ 
      f:R^3~\rightarrow~R
      \f]
      which can be implemented using the lut table. Consider, that all of these
      possibly very complex operations will not take more then the above mesured 
      <b>11msec</b>.
  **/
  template<class T>
  class LUTOp3Channel : public UnaryOp, public Uncopyable {
    public:
    
    /// Internal plugin class for the LUTOp3Channel \ingroup OTHER
    /** The Plugin class can be reimplemented to create custom LUTOp3Channel
        functions. The basic implementation realized a default color distance 
        map on source images.
    */
    class Plugin{
      public:
      /// Empty constructor
      Plugin(){}
      /// Constructor
      /** @param ref1 first channel reference color value 
          @param ref2 second channel reference color value           
          @param ref3 third channel reference color value 
          @param thresh euclidian distance threshold
      */
      Plugin(int ref1, int ref2, int ref3, int thresh){
        m_aiRef[0] = ref1;
        m_aiRef[1] = ref2;
        m_aiRef[2] = ref3;
        m_iThresh = thresh;
      }
      /// Destructor
      virtual ~Plugin(){}
      
      /// Transformation function 
      /** This function must be reimplemented for custom LUT functions.
          The function is:
          \code
          return Cast<double,T>::cast( ::sqrt( ::pow(r-m_aiRef[0],2) + 
                                               ::pow(g-m_aiRef[1],2) + 
                                               ::pow(b-m_aiRef[2],2) ) / 
                                               ::sqrt(3.0) < m_iThresh ? 255 : 0;)
          \endcode
          @param v1 first channel pixel value of input image 
          @param v2 second channel pixel value of input image 
          @param v3 third channel pixel value of input image 
          
      */
      virtual T transform(int v1, int v2, int v3){
        return ::sqrt( ::pow(v1-m_aiRef[0],2) + 
                       ::pow(v2-m_aiRef[1],2) + 
                       ::pow(v3-m_aiRef[2],2) ) / 
                       ::sqrt(3.0) < m_iThresh ? T(255) : T(0);
      }
      private:

      /// internal reference colors
      int m_aiRef[3];

      /// euclidian distance threshold
      int m_iThresh;
    };

    /// creates a LUT object with given lut (LUT-mode)
    /** @param p plugin to use for the creation of the internal lut.
                 <b>Note:</b>the LUTOp3Channel takes the ownership
                 of the given plugin */
    LUTOp3Channel(Plugin *p=0);
    
    /// Destructor
    virtual ~LUTOp3Channel();
    
    /// Common Filter apply function using current mode 
    /** @param src source image
        @param dst destination image**
     */
    virtual void apply(const ImgBase *src, ImgBase **dst);
    
    /// Sets a new plugin and re-calculates the internal LUT
    /** @param p plugin to use for the creation of the internal lut.
        <b>Note:</b>the LUTOp3Channel takes the ownership
        of the given plugin 
        */
    void setPlugin(Plugin *p);
    
    
    
    private:
    
    /// Image that hold the lut data
    Img<T> m_oLUT;
    
    /// Current plugin pointer
    Plugin *m_poPlugin;
  };
} // namespace icl

#endif
