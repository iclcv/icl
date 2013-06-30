/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ContourDetector.cpp                    **
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


#include <ICLCV/ContourDetector.h>

#include <ICLCore/Img.h>
#include <ICLUtils/SSEUtils.h>

#include <vector>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{

    ContourDetector::ContourDetector(const icl8u thresh, const bool hierarchy)
      : id_count(0), threshold(thresh), hierarchy(hierarchy) {
    };

    ContourDetector::~ContourDetector() {
    };

    void ContourDetector::createBinaryValues(Img<icl8u> &img) {
      icl8u *d = img.getData(0);
      icl8u *dstEnd = d + img.getDim();

    #ifdef HAVE_SSE2
      icl8u *dstSSEEnd = dstEnd - 15;

      for (; d < dstSSEEnd; d += 16) {
        // convert 'rvalues' values at the same time
        icl128i v = icl128i(d);
        v.v0 = _mm_sub_epi8(v.v0, _mm_set1_epi8(128));
        v.v0 = _mm_cmplt_epi8(v.v0, _mm_set1_epi8((char)threshold-128));
        v.v0 = _mm_andnot_si128(v.v0, _mm_set1_epi32(0x01010101));
        v.storeu(d);
      }

      for (; d < dstEnd; ++d) {
        // convert 1 value
        *d = (*d < threshold) ? 0 : 1;
      }
    #else
      for (; d < dstEnd; ++d) *d = (*d < threshold) ? 0 : 1;
    #endif
    }

    std::vector<Contour> &ContourDetector::findContours(core::Img<icl8u> &img) {
      if (hierarchy) findContoursWithHierarchy(img);
      else findContoursWithoutHierarchy(img);
      return contours;
    }

    void ContourDetector::findContoursWithoutHierarchy(core::Img<icl8u> &_img) {
      if (_img.getFormat() != formatGray) {
        ERROR_LOG("the image format should be formatGray");
        return;
      }

      core::Img<icl8u> img;
      _img.deepCopy(&img);
      Size size = img.getSize();
      char *img_d = (char*)(img.getData(0));

      // the image border values have to be 0
      memset(img_d, 0, size.width);
      memset(img_d + size.width * (size.height - 1), 0, size.width);

      for (int y = 1; y < (size.height - 1); ++y) {
        img_d += size.width;
        *img_d = 0;
        *(img_d + size.width - 1) = 0;
      }

      // convert gray values to binary values
      createBinaryValues(img);

      contours.clear();

      int NBD = 1;
      const int w = size.width;
      const int h = size.height;
      char *pos0, *pos1, *pos3, *pos4;
      int npos;

      const int int_to_inc[8] = {1, 1, 0, -1, -1, -1, 0, 1};

      Contour c;
      c.id = -1;
      c.is_hole = -1;
      c.parent = -1;
      img_d = (char*)(img.getData(0));

      for (int y = 1; y < h-1; ++y) {
        // previous value is 0 because the for-loop starts at x = 1
        char prev_val = 0;
        img_d += w;

        for (int x = 1; x < w; ++x, prev_val = *pos0) {
          pos0 = img_d + x;

          if (prev_val && *pos0) {
            continue;
          }

          if ((*pos0 != 1) || (prev_val)) {
            if (*pos0) {
              continue;
            }
            if (prev_val < 1) {
              continue;
            }
            // an inner contour was found
            npos = 0;
            --pos0;

            c.clear();
            c.push_back(Point(x-1, y));
          } else {
            // an outer contour was found
            npos = 4;

            c.clear();
            c.push_back(Point(x, y));
          }

          // it is enough if the value of NBD is 2,
          // but somehow the calculation is faster with this if-statement
          if (++NBD > 127) {
            NBD = 2;
          }

          char* nbs[8] = {pos0+1, pos0-w+1, pos0-w, pos0-w-1, pos0-1, pos0+w-1, pos0+w, pos0+w+1};

          pos1 = 0;
          // find last position of the current contour
          for (int end = npos + 1; npos != end;) {
            npos = (npos - 1) & 7;
            if (*(nbs[npos])) {
              pos1 = nbs[npos++];
              break;
            }
          }

          if (pos1) {
            pos3 = pos0;
            int it = 0;

            // follow contour
            while (true) {
              char tmp = *pos3;
              if (tmp == 1) tmp = NBD;

              // find the next neighbour
              for (; ; ++npos) {
                npos &= 7;

                if (*(nbs[npos])) {
                  pos4 = nbs[npos];
                  *pos3 = tmp;

                  break;
                }

                // mark the right side of the contour with a negative value
                if (!npos) tmp = -NBD;
              }

              if (pos4 == pos0) if (pos3 == pos1) {
                break;
              }

              c.push_back(utils::Point(c[it].x + int_to_inc[npos], c[it].y + int_to_inc[(npos+2)&7]));
              ++it;
              npos += 5;

              pos3 = pos4;

              nbs[0] = pos3 + 1;
              nbs[1] = pos3 - w + 1;
              nbs[2] = pos3 - w;
              nbs[3] = pos3 - w - 1;
              nbs[4] = pos3 - 1;
              nbs[5] = pos3 + w - 1;
              nbs[6] = pos3 + w;
              nbs[7] = pos3 + w + 1;
            }
          } else {
            // the contour is just a point
            *pos0 = -NBD;
          }

          contours.push_back(c);

          if (*pos0 != *(pos0+1)) ++pos0;
        }
      }
    }

    void ContourDetector::findContoursWithHierarchy(Img<icl8u> &_img) {
      if (_img.getFormat() != formatGray) {
        ERROR_LOG("the image format should be formatGray");
        return;
      }

      Img<icl8u> img;
      _img.deepCopy(&img);
      Size size = img.getSize();
      char *img_d = (char*)(img.getData(0));

      // the image border values have to be 0
      memset(img_d, 0, size.width);
      memset(img_d + size.width * (size.height - 1), 0, size.width);

      for (int y = 1; y < (size.height - 1); ++y) {
        img_d += size.width;
        *img_d = 0;
        *(img_d + size.width - 1) = 0;
      }

      // convert gray values to binary values
      createBinaryValues(img);

      contours.clear();

      int NBD = 1;
      const int w = size.width;
      const int h = size.height;
      char *pos0, *pos1, *pos3, *pos4;
      int npos, lnbdx;

      int lnbd[128] = { 0 };
      const int int_to_inc[8] = {1, 1, 0, -1, -1, -1, 0, 1};

      Contour c;
      id_count = 0;
      img_d = (char*)(img.getData(0));

      for (int y = 1; y < h-1; ++y) {
        // previous value is 0 because the for-loop starts at x = 1
        char prev_val = 0;
        img_d += w;
        lnbdx = 0;

        for (int x = 1; x < w; ++x, prev_val = *pos0) {
          pos0 = img_d + x;

          if (prev_val && *pos0) {
            if (*pos0 & -2) lnbdx = x;
            continue;
          }

          if ((*pos0 != 1) || (prev_val)) {
            if (*pos0) {
              if (*pos0 & -2) lnbdx = x;
              continue;
            }
            if (prev_val < 1) {
              if (*pos0 & -2) lnbdx = x;
              continue;
            }
            // an inner contour was found
            npos = 0;
            --pos0;

            c.clear();
            c.push_back(Point(x-1, y));
            c.id = id_count++;
            c.is_hole = 1;
          } else {
            // an outer contour was found
            npos = 4;

            c.clear();
            c.push_back(Point(x, y));
            c.id = id_count++;
            c.is_hole = 0;
          }

          if (++NBD > 127) {
            NBD = 2;
          }


          // decide the parent of the current border
          int lc = lnbd[(*(img_d + lnbdx) & 127)] - 1;

          if (lc >= 0) {
            while (lc > 126) {
              std::vector<icl::utils::Point>::const_iterator it = contours[lc].begin();
              for (; it != contours[lc].end(); ++it) {
                if (it->y == y) if (it->x == lnbdx) {
                  break;
                }
              }

              if (it != contours[lc].end()) break;

              lc -= 126;
            }

            if (c.is_hole ^ contours[lc].is_hole) {
              c.parent = lc;
              contours[lc].children.push_back(id_count);
            } else {
              c.parent = contours[lc].parent;
              if (contours[lc].parent >= 0) contours[contours[lc].parent].children.push_back(id_count);
            }
          } else c.parent = -1;


          char* nbs[8] = {pos0+1, pos0-w+1, pos0-w, pos0-w-1, pos0-1, pos0+w-1, pos0+w, pos0+w+1};

          pos1 = 0;
          // find last position of the current contour
          for (int end = npos + 1; npos != end;) {
            npos = (npos - 1) & 7;
            if (*(nbs[npos])) {
              pos1 = nbs[npos++];
              break;
            }
          }

          if (pos1) {
            pos3 = pos0;
            int it = 0;

            // follow contour
            while (true) {
              char tmp = *pos3;
              if (tmp == 1) tmp = NBD;

              // find the next neighbour
              for (; ; ++npos) {
                npos &= 7;

                if (*(nbs[npos])) {
                  pos4 = nbs[npos];
                  *pos3 = tmp;

                  break;
                }

                // mark the right side of the contour with a negative value
                if (!npos) tmp = -NBD;
              }

              if (pos4 == pos0) if (pos3 == pos1) {
                lnbdx = x - c.is_hole;
                break;
              }

              c.push_back(Point(c[it].x + int_to_inc[npos], c[it].y + int_to_inc[(npos+2)&7]));
              ++it;
              npos += 5;

              pos3 = pos4;

              nbs[0] = pos3 + 1;
              nbs[1] = pos3 - w + 1;
              nbs[2] = pos3 - w;
              nbs[3] = pos3 - w - 1;
              nbs[4] = pos3 - 1;
              nbs[5] = pos3 + w - 1;
              nbs[6] = pos3 + w;
              nbs[7] = pos3 + w + 1;
            }
          } else {
            // the contour is just a point
            *pos0 = -NBD;
          }

          contours.push_back(c);
          lnbd[NBD] = id_count;

          if (*pos0 != *(pos0+1)) ++pos0;
        }
      }
    }

  } // namespace cv
}

