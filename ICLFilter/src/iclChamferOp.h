#ifndef ICL_CHAMFER_OP
#define ICL_CHAMFER_OP

#include "iclUnaryOp.h"
#include <vector>
#include <iclPoint.h>

namespace icl{
  /// Chamfering Unit
  /** TODO comment here 
      \section BENCH Benchmarks
      The chamfering operation komplexity is linear to pixel count of an
      image, so a single benchmark for differnt depths are sufficient:
      
      Image-size 320x240 single channel (Pentium M 1.6GHz)
      - icl8u:  approx. 3.6ms
      - icl16s  approx  3.7ms
      - icl32s: approx. 3.7ms
      - icl32f: approx. 3.6ms
      - icl64f: approx. 3.7ms
      
      calculation of the "real-euclidian norm takes much longer" as it runs linear to
      "image-size"*"white-pixel-count" 
      - e.g. icl8u: approx. 6.6<b>sec</b> (using an image with 2193 white pixels)
  */
  class ChamferOp : public UnaryOp{
    public:
    /// Metric to use for horizontal/vertical and diagonal pixels
    enum metric{
      metric_1_1,            /**< h/v and d are handled equally */
      metric_1_2,            /**< city block metric */       
      metric_2_3,            /**< simple approximation of the euclidian metic \f$1:sqrt(2)\f$*/
      metric_7071_10000,     /**< better approximation of the euclidian metic */
      metric_real_euclidian  /**< use the <b>real</b> euclidian distance instead of the high performance matching (not realtime capable)*/
    };
    
    enum hausdorffMetric{
      hausdorff_max,
      hausdorff_mean
    };

    ChamferOp( metric m = metric_7071_10000 );

    virtual ~ChamferOp(){}
    
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);
    
    void setMetric( metric m);
    
    inline metric getMetric() const{ return m_eMetric; }

    inline int getMaxVal() const { return m_iMaxVal; }

    /// overwrites the settings applied by setMetrix ! 
    inline void setMaxVal(int val) { m_iMaxVal = val; }

    static double computeDirectedHausdorffDistance(const Img32s *chamferImage, 
                                                   const std::vector<Point> &model,
                                                   hausdorffMetric m=hausdorff_mean, 
                                                   int penalty=-1,
                                                   metric m=metric_7071_10000);
    static double computeDirectedHausdorffDistance(const Img32s *chamferImageA, const Img32s *chamferImageB, ChamferOp::hausdorffMetric m);

    static double computeSymmetricHausdorffDistance(const Img32s *chamferImageA, const Img32s *chamferImageB,hausdorffMetric m=hausdorff_mean);
    static double computeSymmetricHausdorffDistance(const std::vector<Point> setA, ImgBase **bufferA, 
                                                    const std::vector<Point> setB, ImgBase **bufferB,
                                                    const Size &imageSize,hausdorffMetric m=hausdorff_mean);
    static double computeSymmeticHausdorffDistance(const Img32s *chamferImage, 
                                                   const std::vector<Point> &model, 
                                                   ImgBase **bufferImage, 
                                                   hausdorffMetric m=hausdorff_mean,
                                                   int penalty=-1,
                                                   metric m=metric_7071_10000);
    
    
    
    private:
    metric m_eMetric;
    int m_iMaxVal;    /// maxinum image value multiplicator (defined by the metric and mutliplied by (image.widht+image.height)  
    int m_aiDist[2];   /// neighbour-pixel distances (horz/vert and diag)
  };
}
#endif
