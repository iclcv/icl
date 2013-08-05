/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/RDPApproximation.cpp                   **
** Module : ICLCV                                                  **
** Authors: Sergius Gaulik                                         **
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

#include <ICLCV/RDPApproximation.h>
#include <iostream>
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{

    const std::vector<Point32f> &RDPApproximation::approximate(const std::vector<Point> &poly, bool polygon) {
      ChainPoint *cps = new ChainPoint[poly.size()];
      ChainPoint *current = cps, *prev = &(cps[poly.size() - 1]);

      for (std::vector<Point>::const_iterator it = poly.begin(); it != poly.end(); ++current, ++it) {
        current->prev = prev;
        current->prev->next = prev = current;
        current->x = it->x;
        current->y = it->y;
      }

      approximatePolygon(cps, poly.size(), polygon);

      delete[] cps;

      return approximation;
    }

    const std::vector<Point32f> &RDPApproximation::approximate(const std::vector<Point32f> &poly, bool polygon) {
      ChainPoint *cps = new ChainPoint[poly.size()];
      ChainPoint *current = cps, *prev = &(cps[poly.size() - 1]);

      for (std::vector<Point32f>::const_iterator it = poly.begin(); it != poly.end(); ++current, ++it) {
        current->prev = prev;
        current->prev->next = prev = current;
        current->x = it->x;
        current->y = it->y;
      }

      approximatePolygon(cps, poly.size(), polygon);

      delete[] cps;

      return approximation;
    }

    const std::vector<Point32f> &RDPApproximation::approximate(const Point *begin, const Point *end, const int size, bool polygon) {
      ChainPoint *cps = new ChainPoint[size];
      ChainPoint *current = cps, *prev = &(cps[size - 1]);

      for (const Point *it = begin; it != end; ++current, ++it) {
        current->prev = prev;
        current->prev->next = prev = current;
        current->x = it->x;
        current->y = it->y;
      }

      approximatePolygon(cps, size, polygon);

      delete[] cps;

      return approximation;
    }

    void RDPApproximation::approximateCurve(const ChainPoint *first, const ChainPoint *last) {
      const ChainPoint *current = first->next, *highest = 0;
      float dx, dy, d_max = 0;

      dx = first->x - last->x;
      dy = first->y - last->y;

      while (current != last) {
        int d = abs((last->x - current->x) * dy - (last->y - current->y) * dx);

        if (d > d_max) {
          d_max = d;
          highest = current;
        }

        current = current->next;
      }

      if (d_max > epsilon) {
        approximateCurve(first, highest);
        approximateCurve(highest, last);
      } else approximation.push_back(Point32f(first->x, first->y));
    }

    void RDPApproximation::approximatePolygon(const ChainPoint *cps, const int size, bool polygon) {
      const ChainPoint *current = cps, *first = cps, *last = cps->prev;
      int d_max = 0;

      approximation.clear();

      if (size < 3) {
        for (int i = 0; i < size; ++i) {
          approximation.push_back(Point32f(current->x, current->y));
          current = current->next;
        }
      }

      if (polygon) {
        for (int i = 0; i < search_iters; ++i) {
          first = last;
          current = first->next;

          do {
            int d = pow(first->x - current->x, 2) + pow(first->y - current->y, 2);

            if (d >= d_max) {
              d_max = d;
              last = current;
            }

            current = current->next;
          } while (current != first);
        }

        approximateCurve(first, last);
        approximateCurve(last, first);
      } else {
        approximateCurve(cps, cps->prev);
        approximation.push_back(Point32f(cps->prev->x, cps->prev->y));
      }
    }

    template<class T> static void draw_points(Img<T> &img, const icl64f &value,
                                              std::vector<Point32f> &approx){
      T *d = img.getData(0);
      int lineStep = img.getLineStep();
      for(std::vector<Point32f>::iterator it = approx.begin(); it != approx.end(); ++it){
        *(d + (int)(it->y) * lineStep + (int)(it->x)) = value;
      }
    }

    void RDPApproximation::drawAllPoints(core::ImgBase *img, const icl64f &value) {
      ICLASSERT_THROW(img,ICLException("Contour::draw: img was null"));
      switch(img->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: draw_points(*img->as##D(),value, approximation); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
        default: ICL_INVALID_DEPTH;
#undef ICL_INSTANTIATE_DEPTH
      }
    }

  } // namespace cv
}
