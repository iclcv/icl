#ifndef PROXIMITY_H
#define PROXIMITY_H

#include <Filter.h>
#include "Img.h"
#include "Array.h"
namespace icl {
  
  /// Class for computing proximity measures
  /**  
  The functions described in this section compute the proximity (similarity) measure between an
image and a template (another image). These functions may be used as feature detection functions,
as well as the components of more sophisticated techniques.
There are several ways to compute the measure of similarity between two images. One way is to
compute the Euclidean distance, or sum of the squared distances (SSD), of an image and a
template. The smaller is the value of SSD at a particular pixel, the more similarity exists between
the template and the image in the neighborhood of that pixel.
  */
  class Proximity : public Filter {
  public:
    ///Computes normalized full Euclidean distance between an image and a template.
    void SqrDistanceFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
    ///Computes normalized Euclidean distance between an image and a template.
    void SqrDistanceSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
    ///Computes normalized valid Euclidean distance between an image and a template.
    void SqrDistanceValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);

    ///Computes normalized full cross-correlation between an image and a template.
    /**  impemented with support for interleaved, 3ch and 4ch images, may be removed
    */
    void CrossCorrFull_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, bool color=true);
    ///Computes normalized cross-correlation between an image and a template.
    void CrossCorrSame_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
    ///Computes normalized valid cross-correlation between an image and a template.
    void CrossCorrValid_Norm (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
  
    ///Computes normalized full correlation coefficients between an image and a template.
    void CrossCorrFull_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
    ///Computes normalized correlation coefficients between an image and a template.
    void CrossCorrSame_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
    ///Computes normalized valid correlation coefficients between an image and a template.  
    void CrossCorrValid_NormLevel (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);

    ///Computes normalized full Euclidean distance between an image and a template. Img8u version.
    static void SqrDistanceFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    ///Computes normalized Euclidean distance between an image and a template. Img8u version.
    static void SqrDistanceSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    ///Computes normalized valid Euclidean distance between an image and a template. Img8u version.
    static void SqrDistanceValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);

    ///Computes normalized full cross-correlation between an image and a template. Img8u version.
    static void CrossCorrFull_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst, bool color=true);
    ///Computes normalized cross-correlation between an image and a template. Img8u version.
    static void CrossCorrSame_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);
    ///Computes normalized valid cross-correlation between an image and a template. Img8u version.
    static void CrossCorrValid_Norm (const Img8u *src1, const Img8u *src2, Img32f *dst);

    ///Computes normalized full correlation coefficients between an image and a template. Img8u version.
    static void CrossCorrFull_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst);
    ///Computes normalized correlation coefficients between an image and a template. Img8u version.
    static void CrossCorrSame_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst);
    ///Computes normalized valid correlation coefficients between an image and a template. Img8u version.
    static void CrossCorrValid_NormLevel (const Img8u *src1, const Img8u *src2, Img32f *dst);

    ///Computes normalized full Euclidean distance between an image and a template. Img32f version.
    static void SqrDistanceFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    ///Computes normalized Euclidean distance between an image and a template. Img32f version.
    static void SqrDistanceSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    ///Computes normalized valid Euclidean distance between an image and a template. Img32f version.
    static void SqrDistanceValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);

    ///Computes normalized full cross-correlation between an image and a template. Img32f version.
    static void CrossCorrFull_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst, bool color=true);
    ///Computes normalized cross-correlation between an image and a template. Img32f version.
    static void CrossCorrSame_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);
    ///Computes normalized valid cross-correlation between an image and a template. Img32f version.
    static void CrossCorrValid_Norm (const Img32f *src1, const Img32f *src2, Img32f *dst);

    ///Computes normalized full correlation coefficients between an image and a template. Img32f version.
    static void CrossCorrFull_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst);
    ///Computes normalized correlation coefficients between an image and a template. Img32f version.
    static void CrossCorrSame_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst);
    ///Computes normalized valid correlation coefficients between an image and a template. Img32f version.
    static void CrossCorrValid_NormLevel (const Img32f *src1, const Img32f *src2, Img32f *dst);
    private:
  // buffer for 3ch and 4ch interleaved image, may be removed 
  Array<icl8u> m_oBuffer8u[2];
  // buffer for 3ch and 4ch interleaved image, may be removed 
  Array<icl32f> m_oBuffer32f[3];
  };
} // namespace icl
#endif


