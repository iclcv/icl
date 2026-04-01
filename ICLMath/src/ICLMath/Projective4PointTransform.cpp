// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLMath/Projective4PointTransform.h>

namespace icl{
  using namespace utils;

  namespace math{

     static Mat3 create_mapping_matrix(const Point32f &a, const Point32f &b,
                                              const Point32f &c, const Point32f &d){
        DynMatrix<float> M(3,3), B(1,3);
        M(0,0) = a.x;
        M(1,0) = b.x;
        M(2,0) = c.x;

        M(0,1) = a.y;
        M(1,1) = b.y;
        M(2,1) = c.y;

        M(0,2) = M(1,2) = M(2,2) = 1;

        B[0] = d.x;
        B[1] = d.y;
        B[2] = 1;

        DynMatrix<float> x = M.solve(B);

        const float &l1 = x[0], &l2 = x[1], &l3 = x[2];
        return Mat3(l1*a.x,l2*b.x,l3*c.x,
                          l1*a.y,l2*b.y,l3*c.y,
                          l1,    l2,    l3);
      }

    void Projective4PointTransform::init(const utils::Point32f s[4], const utils::Point32f d[4]){
      DEBUG_LOG("init with s: " << (s ? "valid" : "null") << " d:" << (d ? "valid" : "null"));
      if(s){
        m_Ainv = create_mapping_matrix(s[0],s[1],s[2],s[3]).pinv();
      }
      if(d){
        m_B = create_mapping_matrix(d[0],d[1],d[2],d[3]);
        if(s){
          m_C = m_B * m_Ainv;
        }
      }
    }

    Projective4PointTransform::Projective4PointTransform(){

    }
    Projective4PointTransform::Projective4PointTransform(const utils::Point32f srcQuad[4],
                                                         const utils::Point32f dstQuad[4]){
      init(srcQuad,dstQuad);
    }
    Projective4PointTransform::Projective4PointTransform(const std::vector<Point32f> &srcQuad,
                                                         const std::vector<Point32f> dstQuad){
      init(srcQuad.data(), dstQuad.data());
    }

    Projective4PointTransform::Projective4PointTransform(const Mat3 &C){
      m_Ainv = Mat3::id();
      m_B = Mat3::id();
      m_C = C;
    }

    void Projective4PointTransform::init(const utils::Rect32f &srcRect,
                                         const utils::Rect32f &dstRect){
      const Point32f s[4] = { srcRect.ul(), srcRect.ur(), srcRect.lr(), srcRect.ll() };
      const Point32f d[4] = { dstRect.ul(), dstRect.ur(), dstRect.lr(), dstRect.ll() };

      init(srcRect == Rect32f::null ? 0 : s,
           dstRect == Rect32f::null ? 0 : d);
    }
    void Projective4PointTransform::init(const utils::Rect &srcRect,
                                         const utils::Rect &dstRect){
      init(Rect32f(srcRect.x, srcRect.y, srcRect.width, srcRect.height),
           Rect32f(dstRect.x, dstRect.y, dstRect.width, dstRect.height));
    }


    void Projective4PointTransform::setSrcQuad(const utils::Point32f s[4]){
      m_Ainv = create_mapping_matrix(s[0],s[1],s[2],s[3]).pinv();
      m_C = m_B * m_Ainv;
    }
    void Projective4PointTransform::setDstQuad(const utils::Point32f d[4]){
      m_B = create_mapping_matrix(d[0],d[1],d[2],d[3]);
      m_C = m_B * m_Ainv;
    }

    void Projective4PointTransform::setSrcQuad(const utils::Point32f &a, const utils::Point32f &b,
                                               const utils::Point32f &c, const utils::Point32f &d){
      const Point32f ps[4] = { a, b, c, d };
      setSrcQuad(ps);
    }

    void Projective4PointTransform::setDstQuad(const utils::Point32f &a, const utils::Point32f &b,
                                               const utils::Point32f &c, const utils::Point32f &d){
      const Point32f ps[4] = { a, b, c, d };
      setDstQuad(ps);
    }


    std::vector<Point32f> Projective4PointTransform::map(const std::vector<Point32f> &src){
      std::vector<Point32f> dst(src.size());
      map(src.begin(), src.end(), dst.begin());
      return dst;
    }
  }
}
