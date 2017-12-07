/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ImageRectification.cpp         **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLFilter/ImageRectification.h>
#include <ICLMath/Homography2D.h>
#include <ICLCore/ConvexHull.h>

#include <deque>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace filter{

    template<class T>
    const Img<T> &ImageRectification<T>::apply(const FixedMatrix<float,3,3> &transform, const Img<T> &src, const Size &resultSize){
      throw ICLException("ImageRectification<T>::apply(const FixedMatrix<float,3,3> &transform,...): this method is not yet implemented!");
      return buffer;
    }

    static void convexity_check_and_sorting(Point32f ps[4]) throw (ICLException){
      std::vector<Point32f> hull = convexHull(std::vector<Point32f>(ps,ps+4));
      // first and last points are doubled
      if(hull.size() != 5) throw ICLException("ImageRectification<T>::apply: given points do not define a convex quadrangle");
      ps[0] = hull[0];
      ps[1] = hull[1];
      ps[2] = hull[2];
      ps[3] = hull[3];
    }

    static void convexity_check(const Point32f ps[4]){
      std::vector<Point32f> hull = convexHull(std::vector<Point32f>(ps,ps+4));
      if(hull.size() != 5) {
        throw ICLException("ImageRectification<T>::apply: given points do"
                           " not define a convex quadrangle");
      }
      // check whether the hull re-arranged ps
      hull.resize(4);
      std::deque<Point32f> hulld(hull.begin(),hull.end());
      for(int i=0;i<4;++i){
        if(hulld[0] == ps[0] && hulld[1] == ps[1] &&
           hulld[2] == ps[2] && hulld[3] == ps[3]) return;
        hulld.push_front(hulld.back());
        hulld.pop_back();
      }
      std::reverse(hulld.begin(),hulld.end());
      for(int i=0;i<4;++i){
        if(hulld[0] == ps[0] && hulld[1] == ps[1] &&
           hulld[2] == ps[2] && hulld[3] == ps[3]) return;
        hulld.push_front(hulld.back());
        hulld.pop_back();
      }

      throw ICLException("ImageRectification<T>::apply: given points define a crossed quadrangle");
    }

    template<class T>
    static const Homography2D create_and_check_homography(bool validateAndSortPoints,const Point32f psin[4], const Img<T> &src,
                                                          const Size &resultSize, FixedMatrix<float,3,3> *hom,
                                                          FixedMatrix<float,2,2> *Q, FixedMatrix<float,2,2> *R,float maxTilt,
                                                          Img<T> &buffer, bool advanedAlgorithm) throw(ICLException){
      Point32f ps[4]={psin[0],psin[1],psin[2],psin[3]};
      // we need this check, because otherwise, we cannot check whether
      // the image boarders are intersected by checking the four corners only
      // the check based on the convex hull seems to be both, fast and accurate
      if(validateAndSortPoints){
        convexity_check_and_sorting(ps);
      }else{
        convexity_check(ps);
      }



      buffer.setChannels(src.getChannels());
      buffer.setSize(resultSize);

      int W = resultSize.width;
      int H = resultSize.height;
      const Point32f ys[4] = {
        Point32f(0,0),
        Point32f(W-1,0),
        Point32f(W-1,H-1),
        Point32f(0,H-1)
      };

      const Homography2D HOM(ps,ys,4, (Homography2D::Algorithm)advanedAlgorithm);


      if(hom) *hom = HOM;
      if(Q || R || (maxTilt > 0)){
        FixedMatrix<float,2,2> q,r;
        FixedMatrix<float,2,2>(HOM(0,0),HOM(1,0),
                               HOM(0,1),HOM(1,1)).decompose_QR(q,r);
        if(Q) *Q = q;
        if(R) *R = r;
        if(maxTilt>0){
          float a = fabs(r(0,0)), b=fabs(r(1,1));
          if(a < 1E-6 || b < 1E-5) throw ICLException("ImageRectification<T>::apply: maxTilt criterion not "
                                                      "met (at least one diagonal element of R is too small)");
          if(iclMax(a/b,b/a) > maxTilt) throw ICLException("ImageRectification<T>::apply: maxTilt criterion not "
                                                           "met (diagonal element ratio of R > maxTilt)");
        }
      }

      const Rect rect = src.getImageRect();
      for(int i=0;i<4;++i){
        Point32f p = HOM.apply(ys[i]);
        if(!rect.contains(p.x,p.y)) throw ICLException("ImageRectification<T>::apply: at least one edge of "
                                                       "the source rect is outside the source image rectangle");
      }
      return HOM;
    }

    // image retification with nearest neighbor interpolation
    template<class T>
    const Img<T> &apply_nearest_neighbor(const Img<T> &src,
                                         const Size &resultSize, const Homography2D &hom,
                                         const Rect *resultROI, Img<T> &buffer){
      const int W=resultSize.width,H=resultSize.height;
      int x;

#ifdef ICL_HAVE_SSE2
      // convert the values of the homography matrix in sse-types
      const __m128 hom00 = _mm_set1_ps(hom(0,0));
      const __m128 hom01 = _mm_set1_ps(hom(0,1));
      const __m128 hom02 = _mm_set1_ps(hom(0,2));
      const __m128 hom10 = _mm_set1_ps(hom(1,0));
      const __m128 hom11 = _mm_set1_ps(hom(1,1));
      const __m128 hom12 = _mm_set1_ps(hom(1,2));
      const __m128 hom20 = _mm_set1_ps(hom(2,0));
      const __m128 hom21 = _mm_set1_ps(hom(2,1));
      const __m128 hom22 = _mm_set1_ps(hom(2,2));
      // constant for faster counting
      const __m128 r0123 = _mm_set_ps(3,2,1,0);

  #ifdef WIN32
      // memory for four x and y values
      __declspec(align(16)) float x4[4];
      __declspec(align(16)) float y4[4];
  #else
      // memory for four x and y values
      __attribute__((aligned(16))) float x4[4];
      __attribute__((aligned(16))) float y4[4];
  #endif
#endif

      for(int c=0;c<src.getChannels();++c){
        Channel<T> r = buffer[c];
        const Channel<T> s = src[c];
        if(resultROI){
          const int xstart = resultROI->x, xend = resultROI->right(),
                    ystart = resultROI->y, yend = resultROI->bottom();
          for(int y=ystart;y<yend;++y){
#ifdef ICL_HAVE_SSE2
            const __m128 ra = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom10), hom20);
            const __m128 rb = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom11), hom21);
            const __m128 rz = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom12), hom22);
            // calculate 4 pixel at the same time
            for(x=xstart;x<xend-3;x+=4){
              // calculate position in the image
              __m128 raa = _mm_add_ps(ra, _mm_mul_ps(hom00, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rbb = _mm_add_ps(rb, _mm_mul_ps(hom01, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rzz = _mm_add_ps(rz, _mm_mul_ps(hom02, _mm_add_ps(_mm_set1_ps(x), r0123)));
              _mm_store_ps(x4, _mm_add_ps(_mm_div_ps(raa, rzz),_mm_set1_ps(0.5f)));
              _mm_store_ps(y4, _mm_add_ps(_mm_div_ps(rbb, rzz),_mm_set1_ps(0.5f)));
              // get the values from the image
              r(x  ,y) = s(x4[0], y4[0]);
              r(x+1,y) = s(x4[1], y4[1]);
              r(x+2,y) = s(x4[2], y4[2]);
              r(x+3,y) = s(x4[3], y4[3]);
            }
            // calculate the last pixel
            for(;x<xend;++x){
              Point32f p = hom.apply(Point32f(x,y));
              r(x,y) = s(p.x, p.y);
            }
#else
            for(x=xstart;x<xend;++x){
              Point32f p = hom.apply(Point32f(x,y));
              r(x,y) = s(p.x, p.y);
            }
#endif
          }
        }else{
          for(int y=0;y<H;++y){
#ifdef ICL_HAVE_SSE2
            const __m128 ra = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom10), hom20);
            const __m128 rb = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom11), hom21);
            const __m128 rz = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom12), hom22);
            // calculate 4 pixel at the same time
            for(x=0;x<W-3;x+=4){
              // calculate position in the image
              __m128 raa = _mm_add_ps(ra, _mm_mul_ps(hom00, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rbb = _mm_add_ps(rb, _mm_mul_ps(hom01, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rzz = _mm_add_ps(rz, _mm_mul_ps(hom02, _mm_add_ps(_mm_set1_ps(x), r0123)));
              _mm_store_ps(x4, _mm_add_ps(_mm_div_ps(raa, rzz),_mm_set1_ps(0.5f)));
              _mm_store_ps(y4, _mm_add_ps(_mm_div_ps(rbb, rzz),_mm_set1_ps(0.5f)));
              // get the values from the image
              r(x  ,y) = s(x4[0], y4[0]);
              r(x+1,y) = s(x4[1], y4[1]);
              r(x+2,y) = s(x4[2], y4[2]);
              r(x+3,y) = s(x4[3], y4[3]);
            }
            // calculate the last pixel
            for(;x<W;++x){
              Point32f p = hom.apply(Point32f(x,y));
              r(x,y) = s(p.x, p.y);
            }
#else
            for(x=0;x<W;++x){
              Point32f p = hom.apply(Point32f(x,y));
              r(x,y) = s(p.x, p.y);
            }
#endif
          }
        }
      }

      return buffer;
    }

    // image retification with linear interpolation
    template<class T>
    const Img<T> &apply_linear(const Img<T> &src,
                               const Size &resultSize, const Homography2D &hom,
                               const Rect *resultROI, Img<T> &buffer){
      const int W=resultSize.width,H=resultSize.height;
      int x;

#ifdef ICL_HAVE_SSE2
      // convert the values of the homography matrix in sse-types
      const __m128 hom00 = _mm_set1_ps(hom(0,0));
      const __m128 hom01 = _mm_set1_ps(hom(0,1));
      const __m128 hom02 = _mm_set1_ps(hom(0,2));
      const __m128 hom10 = _mm_set1_ps(hom(1,0));
      const __m128 hom11 = _mm_set1_ps(hom(1,1));
      const __m128 hom12 = _mm_set1_ps(hom(1,2));
      const __m128 hom20 = _mm_set1_ps(hom(2,0));
      const __m128 hom21 = _mm_set1_ps(hom(2,1));
      const __m128 hom22 = _mm_set1_ps(hom(2,2));
      // constant for faster counting
      const __m128 r0123 = _mm_set_ps(3,2,1,0);

  #ifdef WIN32
      // memory for four x and y values
      __declspec(align(16)) float x4[4];
      __declspec(align(16)) float y4[4];
  #else
      // memory for four x and y values
      __attribute__((aligned(16))) float x4[4];
      __attribute__((aligned(16))) float y4[4];
  #endif
#endif

      for(int c=0;c<src.getChannels();++c){
        Channel<T> r = buffer[c];
        const Channel<T> s = src[c];
        if(resultROI){
          const int xstart = resultROI->x, xend = resultROI->right(),
                    ystart = resultROI->y, yend = resultROI->bottom();
          for(int y=ystart;y<yend;++y){
#ifdef ICL_HAVE_SSE2
            const __m128 ra = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom10), hom20);
            const __m128 rb = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom11), hom21);
            const __m128 rz = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom12), hom22);
            // calculate 4 pixel at the same time
            for(x=xstart;x<xend-3;x+=4){
              // calculate position in the image
              __m128 raa = _mm_add_ps(ra, _mm_mul_ps(hom00, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rbb = _mm_add_ps(rb, _mm_mul_ps(hom01, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rzz = _mm_add_ps(rz, _mm_mul_ps(hom02, _mm_add_ps(_mm_set1_ps(x), r0123)));
              _mm_store_ps(x4, _mm_div_ps(raa, rzz));
              _mm_store_ps(y4, _mm_div_ps(rbb, rzz));
              // get the values from the image
              r(x  ,y) = s(Point32f(x4[0], y4[0]));
              r(x+1,y) = s(Point32f(x4[1], y4[1]));
              r(x+2,y) = s(Point32f(x4[2], y4[2]));
              r(x+3,y) = s(Point32f(x4[3], y4[3]));
            }
            // calculate the last pixel
            for(;x<xend;++x){
              r(x,y) = s(hom.apply(Point32f(x,y)));
            }
#else
            for(x=xstart;x<xend;++x){
              r(x,y) = s(hom.apply(Point32f(x,y)));
            }
#endif
          }
        }else{
          for(int y=0;y<H;++y){
#ifdef ICL_HAVE_SSE2
            const __m128 ra = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom10), hom20);
            const __m128 rb = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom11), hom21);
            const __m128 rz = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(y), hom12), hom22);
            // calculate 4 pixel at the same time
            for(x=0;x<W-3;x+=4){
              // calculate position in the image
              __m128 raa = _mm_add_ps(ra, _mm_mul_ps(hom00, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rbb = _mm_add_ps(rb, _mm_mul_ps(hom01, _mm_add_ps(_mm_set1_ps(x), r0123)));
              __m128 rzz = _mm_add_ps(rz, _mm_mul_ps(hom02, _mm_add_ps(_mm_set1_ps(x), r0123)));
              _mm_store_ps(x4, _mm_div_ps(raa, rzz));
              _mm_store_ps(y4, _mm_div_ps(rbb, rzz));
              // get the values from the image
              r(x  ,y) = s(Point32f(x4[0], y4[0]));
              r(x+1,y) = s(Point32f(x4[1], y4[1]));
              r(x+2,y) = s(Point32f(x4[2], y4[2]));
              r(x+3,y) = s(Point32f(x4[3], y4[3]));
            }
            // calculate the last pixel
            for(;x<W;++x){
              r(x,y) = s(hom.apply(Point32f(x,y)));
            }
#else
            for(x=0;x<W;++x){
              r(x,y) = s(hom.apply(Point32f(x,y)));
            }
#endif
          }
        }
      }

      return buffer;
    }

    template<class T>
    const Img<T> &ImageRectification<T>::apply(const Point32f ps[4], const Img<T> &src,
                                               const Size &resultSize, FixedMatrix<float,3,3> *hom,
                                               FixedMatrix<float,2,2> *Q, FixedMatrix<float,2,2> *R,
                                               float maxTilt, bool advanedAlgorithm,
                                               const Rect *resultROI, const core::scalemode eScaleMode){
      const Homography2D HOM = create_and_check_homography(validateAndSortPoints,ps,src,resultSize,hom,Q,R,maxTilt,buffer, advanedAlgorithm);

      switch (eScaleMode) {
      case core::interpolateNN:
        apply_nearest_neighbor(src, resultSize, HOM, resultROI, buffer);
        break;
      case core::interpolateLIN:
        apply_linear(src, resultSize, HOM, resultROI, buffer);
        break;
      default:
        WARNING_LOG("the given interpolation method is not supported");
        WARNING_LOG("using linear interpolation as fallback!");
        apply_linear(src, resultSize, HOM, resultROI, buffer);
      }

      return buffer;
    }
//DOES NOT EXIST ANYMORE --> USE SSE2 Version
/*  #ifdef ICL_HAVE_IPP


    template<class T, class IppFunc>
    static const Img<T> &apply_image_rectificaion_ipp(bool validateAndSortPoints,const Rect *resultROI, bool advanedAlgorithm,
                                                      const Point32f ps[4], const Img<T> &src,
                                                      const Size &resultSize, FixedMatrix<float,3,3> *hom,
                                                      FixedMatrix<float,2,2> *Q, FixedMatrix<float,2,2> *R,float maxTilt,
                                                      Img<T> &buffer, const core::scalemode eScaleMode, IppFunc ippFunc){
      const Homography2D HOM = create_and_check_homography(validateAndSortPoints,ps,src,resultSize,hom,Q,R,maxTilt,buffer,advanedAlgorithm);

      const double coeffs[3][3]={ {HOM(0,0),HOM(1,0),HOM(2,0)},
                                  {HOM(0,1),HOM(1,1),HOM(2,1)},
                                  {HOM(0,2),HOM(1,2),HOM(2,2)} };

      for(int c=0;c<src.getChannels();++c){
        IppStatus s = ippFunc(src.begin(c), src.getSize(), src.getLineStep(), src.getImageRect(),
                              buffer.begin(c),buffer.getLineStep(),resultROI ? *resultROI : buffer.getImageRect(), coeffs,
                              eScaleMode);
        if(s != ippStsNoErr){
          ERROR_LOG(ippGetStatusString(s));
        }
      }

      return buffer;
    }

    template<> const Img8u &ImageRectification<icl8u>::apply(const Point32f ps[4], const Img8u &src,
                                                             const Size &resultSize, FixedMatrix<float,3,3> *hom,
                                                             FixedMatrix<float,2,2> *Q, FixedMatrix<float,2,2> *R,
                                                             float maxTilt, bool advanedAlgorithm,
                                                             const Rect *resultROI, const core::scalemode eScaleMode){
      return apply_image_rectificaion_ipp(validateAndSortPoints,resultROI,advanedAlgorithm,ps,src,resultSize,hom,Q,R,maxTilt,buffer,eScaleMode,ippiWarpPerspectiveBack_8u_C1R);
    }

    template<> const Img32f &ImageRectification<icl32f>::apply(const Point32f ps[4], const Img32f &src,
                                                               const Size &resultSize, FixedMatrix<float,3,3> *hom,
                                                               FixedMatrix<float,2,2> *Q, FixedMatrix<float,2,2> *R,
                                                               float maxTilt, bool advanedAlgorithm,
                                                               const Rect *resultROI, const core::scalemode eScaleMode){
      return apply_image_rectificaion_ipp(validateAndSortPoints,resultROI,advanedAlgorithm,ps,src,resultSize,hom,Q,R,maxTilt,buffer,eScaleMode,ippiWarpPerspectiveBack_32f_C1R);
    }
  #endif*/


  #define ICL_INSTANTIATE_DEPTH(D) template class ICLFilter_API ImageRectification<icl##D>;
    ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
  } // namespace markers
}
