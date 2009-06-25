#ifndef LUT_OP_3C_H
#define LUT_OP_3C_H

#include "iclUnaryOp.h"
#include "iclImg.h"
#include <vector>
#include <iclUncopyable.h>

namespace icl {

  /// class for applying table look-up transformation to 3-channel integer-valued images \ingroup UNARY
  /** In many applications it is necessary to create a feature map on 3-channel 
      input images. These feature map creation procedure can take much time,
      depending on the complexity of the calculated feature. In a very simple
      case for instance, the LUTOp3Channel can be used to create thresholded 
      color-distance map using the the euclidean distance to a reference color and a
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
      using a simple table look-up:
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
      euclidean distance map takes about <b>18msec</b> on an 640x480 3-channel icl8u
      input image. This can be accelerated by 61% to <b>11msec</b> (1.6Mhz Pentium M)
      by using the LUTOp3Channel class. This benefit is not <em>very</em> high, but the 
      euclidean distance function is only one possible function
      \f[ 
      f:R^3~\rightarrow~R
      \f]
      which can be implemented using the 3 channel lut. Consider, that all of these
      possibly very complex operations will not take more then the above measured 
      <b>11msec</b>.

      \section SHIFT Optimization "Shift"
      In some applications, it is sufficient to use a decreased resolution for each
      channel range. E.g. if we use only 7Bit per channel, we can reduce the LUT size
      from \f$2^{24} \mbox{bytes} = 16.8\mbox{MB}\f$ to \f$2^{21} \mbox{bytes} = 2.1\mbox{MB}\f$
      further reduction of the quantization level of each channel to 6Bit results in a
      \f$2^{18} \mbox{bytes}=262\mbox{kB}\f$ LUT.\n
      The benefit of this optimization is at first of course the saving of a large part of 
      necessary memory, but in a second view, it also increases processing speed even beyond
      the speed enhancement achieved using a LUT. The reason for the speed advantage
      compared with the full size LUT can be explained by the cpu's cache management: When 
      using a 16.8MB data array which is nearly randomly accessed, the data can not be 
      cached permanently, so the cache flow in influenced negatively. The following table
      illustrates some benchmark results using different bit shifts, compared on a 640x480 
      rgb icl8u image.
      
      <table>
      <tr> <td><b>Shift</b></td>  <td><b>LUT size</b></td> <td><b>Time*</b></td></tr>
      <tr> <td>0 (no shift)</td>  <td>16.8MB</td>          <td>11ms</td></tr>
      <tr> <td>1</td>             <td>2.2MB</td>           <td>8ms</td></tr>
      <tr> <td>2</td>             <td>262kB</td>           <td>4.3ms</td></tr>
      <tr> <td>3</td>             <td>33kB</td>            <td>3.5ms</td></tr>
      <tr> <td>4</td>             <td>4kB</td>             <td>3.5ms</td></tr>
      <tr> <td>5</td>             <td>512 Bytes</td>       <td>3.5ms</td></tr>
      <tr> <td>6</td>             <td>64 Bytes</td>        <td>3.5ms</td></tr>
      <tr> <td>7</td>             <td>8 Bytes</td>         <td>3.5ms</td></tr>
      </table>
      
      A bit shift value of 2 seems to be the best compromise between data resolution 
      vs. speed and not at least memory usage. <b>But note:</b> There are some applications,
      where the data resolution can not be scaled down using bit shifts.
      
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
          @param thresh euclidean distance threshold
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
        return ::sqrt( ::pow((float) (v1-m_aiRef[0]),2) + 
                       ::pow((float) (v2-m_aiRef[1]),2) + 
                       ::pow((float) (v3-m_aiRef[2]),2) ) / 
                       ::sqrt(3.0) < m_iThresh ? T(255) : T(0);
      }
      private:

      /// internal reference colors
      int m_aiRef[3];

      /// euclidean distance threshold
      int m_iThresh;
    };

    /// creates a LUT object with given lut (LUT-mode)
    /** @param p plugin to use for the creation of the internal lut.
                 <b>Note:</b>the LUTOp3Channel takes the ownership
                 of the given plugin 
        @param shift see \ref SHIFT
    */
    LUTOp3Channel(Plugin *p=0, icl8u shift=0);
    
    /// Destructor
    virtual ~LUTOp3Channel();
    
    /// Common Filter apply function using current mode 
    /** @param src source image
        @param dst destination image**
     */
    virtual void apply(const ImgBase *src, ImgBase **dst);
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    /// Sets a new plugin and re-calculates the internal LUT
    /** @param p plugin to use for the creation of the internal lut.
        <b>Note:</b>the LUTOp3Channel takes the ownership
        of the given plugin 
        */
    void setPlugin(Plugin *p);
    
    /// removes the internal plugin so it is not deleted with the LUTOp
    Plugin *removePlugin(){
      Plugin *p = m_poPlugin;
      m_poPlugin = 0;
      return p;
    }

    private:
    
    /// Image that holds the lut data
    Img<T> m_oLUT;
    
    /// Current plugin pointer
    Plugin *m_poPlugin;
    
    /// channel range increment (...)
    icl8u m_ucShift;
  };
} // namespace icl

#endif
