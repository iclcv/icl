#ifndef PROXIMITY_H
#define PROXIMITY_H

#include <Filter.h>
#include "Img.h"
namespace icl {
  
  /// Class for Morphological operations
  class Proximity : public Filter {
  public:
    void SqrDistanceFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
    void SqrDistanceSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
    void SqrDistanceValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R

    void CrossCorrFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
    void CrossCorrSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
    void CrossCorrValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R

    void CrossCorrFull_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
    void CrossCorrSame_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
    void CrossCorrValid_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst); //32f_C1R od. 8u32f_C1R
  //protected:



    static void SqrDistanceFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    static void SqrDistanceSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    static void SqrDistanceValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);

    static void CrossCorrFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    static void CrossCorrSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    static void CrossCorrValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);

    static void CrossCorrFull_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst);
    static void CrossCorrSame_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst);
    static void CrossCorrValid_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst);

    static void SqrDistanceFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    static void SqrDistanceSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    static void SqrDistanceValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);

    static void CrossCorrFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    static void CrossCorrSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    static void CrossCorrValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);

    static void CrossCorrFull_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst);
    static void CrossCorrSame_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst);
    static void CrossCorrValid_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst);












  };
} // namespace icl
#endif


