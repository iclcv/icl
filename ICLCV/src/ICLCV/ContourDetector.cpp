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
** Authors: Sergius Gaulik, Christof Elbrechter                    **
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

    struct SimpleContourImpl : public ContourImpl{
      const Point *begin_point;
      const Point *end_point;
      SimpleContourImpl(){}
      SimpleContourImpl(const Point *b, const Point *e):
        begin_point(b),end_point(e){}
      virtual bool hasHierarchy() const { return false; }
      virtual int getID() const { return -1; }
      virtual bool isHole() const { return false; }
      virtual const std::vector<int> &getChildren() const { 
        static std::vector<int> _null; return _null;
      }
      virtual const Point *begin() const { 
        return begin_point;
      }
      virtual const Point *end() const { 
        return end_point;
      }
    };

    struct SimpleContourStorage{
      SimpleContourStorage(int imageDim=1):
        mem(4*imageDim){
        next = &mem[0];
      }
      void reinit(int imageDim){
        if((int)mem.size() < 4*imageDim){
          mem.resize(4*imageDim);
        }
      }
      std::vector<Point> mem;
      std::vector<Point*> cs;
      Point *next;
      
      void addContour(){
        cs.push_back(next);
      }
      void addPoint(const Point &p){
        *next++ = p;
      }
      void clear(){
        next = &mem[0];
        cs.clear();
      }
      // void copy(std::vector<std::vector<Point> > &dst) const{
      //   dst.resize(cs.size());
      //   for(size_t i=0;i<cs.size()-1;++i){
      //     int n = (int)(cs[i+1] - cs[i]);
      //     dst[i].resize(n);
      //     std::copy(cs[i],cs[i+1], dst[i].begin());
      //   }
      //   int n = next - cs.back();
      //   dst.back().resize(n);
      //   std::copy(cs.back(),next,dst.back().begin());
      // }
      void copy(std::vector<SimpleContourImpl> &dst) const{
        dst.resize(cs.size());
        for(size_t i=0;i<cs.size()-1;++i){
          dst[i] = SimpleContourImpl(cs[i],cs[i+1]);
        }
        dst.back() = SimpleContourImpl(cs.back(),next);
      }

    };

    struct ComplexContourImpl : public ContourImpl, public std::vector<utils::Point> {
      int id;             //!< contour ID
      int is_hole;        //!< is it a hole
      int parent;         //!< parent ID
      std::vector<int> children; //!< child contours
      
      virtual bool hasHierarchy() const { return id != -1; }
      virtual int getID() const { return id; }
      virtual bool isHole() const { return is_hole; }

      virtual const std::vector<int> &getChildren() const { return children; }
      virtual const Point *begin() const { return &operator[](0); }
      virtual const Point *end() const { return begin() + size(); }
    };
    

    struct ContourDetector::Data{
      Img8u buffer;
      int id_count;
      icl8u threshold;
      ContourDetector::Algorithm algo;

      SimpleContourStorage storage;
      std::vector<Contour> contoursRet;
      std::vector<ComplexContourImpl> contours;
      std::vector<SimpleContourImpl> simples;

      void findContoursWithoutHierarchy(core::Img<icl8u> &_img);

      void findContoursWithHierarchy(core::Img<icl8u> &_img);

      void createBinaryValues(core::Img<icl8u> &img);

      void findContoursFast(core::Img8u &img);

      void traceContour(Point pStart, Channel8u &c);
    };


    void ContourDetector::setThreshold(const icl8u &threshold){
      m_data->threshold = threshold;
    }
    
    void ContourDetector::setAlgorithm(ContourDetector::Algorithm algo){
      m_data->algo = algo;
    }
    
    template<class T> static void draw_contour(Img<T> &img, const icl64f &value, 
                                               const Point *begin, const Point *end){
      T *d = img.getData(0);
      int lineStep = img.getLineStep();
      for(const Point *it = begin; it != end; ++it){
        *(d + it->y * lineStep + it->x) = value;
      }
    }


    ContourDetector::ContourDetector(const icl8u thresh, ContourDetector::Algorithm a) : m_data(new Data){
      m_data->id_count = 0;
      m_data->threshold = thresh;
      m_data->algo = a;
    };

    ContourDetector::~ContourDetector() {
      delete m_data;
    };

    void ContourDetector::drawAllContours(core::ImgBase *img, const icl64f &val) {
      for (std::vector<Contour>::iterator it = m_data->contoursRet.begin(); 
           it != m_data->contoursRet.end(); ++it) {
        it->drawTo(img, val);
      }
    }

    
    void Contour::drawTo(ImgBase *img, const icl64f &value){
      ICLASSERT_THROW(img,ICLException("Contour::draw: img was null"));
      switch(img->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: draw_contour(*img->as##D(),value, begin(), end()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
        default: ICL_INVALID_DEPTH;
#undef ICL_INSTANTIATE_DEPTH
      }
    }


    void ContourDetector::Data::createBinaryValues(Img<icl8u> &img) {
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

    const std::vector<Contour> &ContourDetector::detect(core::Img<icl8u> &img) {
      if(m_data->algo == AccurateWithHierarchy){
        m_data->findContoursWithHierarchy(img);
      }else if(m_data->algo == Accurate){
        m_data->findContoursWithoutHierarchy(img);
      }else{
        m_data->storage.reinit(img.getDim());
        m_data->findContoursFast(img);
        m_data->storage.copy(m_data->simples);
          
      }
     
      if(m_data->algo == Fast){
        const size_t n = m_data->simples.size();
        m_data->contoursRet.resize(n);
        for(size_t i=0;i<n;++i){
          m_data->contoursRet[i] = Contour(&m_data->simples[i]);
        }     
      }else{
        const size_t n = m_data->contours.size();
        m_data->contoursRet.resize(n);
        for(size_t i=0;i<n;++i){
          m_data->contoursRet[i] = Contour(&m_data->contours[i]);
        }
      }
      return m_data->contoursRet;
    }

    const std::vector<Contour> &ContourDetector::detect(const core::ImgBase *image){
      ICLASSERT_THROW(image,ICLException("ContourDetector::detect: image was null"));
      image->convert(&m_data->buffer);
      return detect(m_data->buffer);
    }

    void ContourDetector::Data::findContoursWithoutHierarchy(core::Img<icl8u> &_img) {
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

      ComplexContourImpl c;
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

    void ContourDetector::Data::findContoursWithHierarchy(Img<icl8u> &_img) {
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

      ComplexContourImpl c;
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
              std::vector<icl::utils::Point>::const_iterator it = contours[lc].std::vector<Point>::begin();
              for (; it != contours[lc].std::vector<Point>::end(); ++it) {
                if (it->y == y) if (it->x == lnbdx) {
                  break;
                }
              }

              if (it != contours[lc].std::vector<Point>::end()) break;

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


    void ContourDetector::Data::traceContour(Point pStart, Channel8u &c){
      Point p = pStart;
      storage.addPoint(p);
      int dir = 1; // from top 
      /*  1
          0>  2
          3
          */
      //static int step = 0;
      //std::string names[] = {"left","top","right","bottom"};
      for(;;){
        //if(++step == 10) throw ICLException("step 100 reached at " + str(p));
        c(p.x,p.y) = 128; 
        //std::cout << " -- from " << names[dir] << " p is " << p <<  std::endl;
        switch(dir){
          case 0: // from left:
            if(c(p.x,p.y-1)){
              storage.addPoint( (p = Point(p.x,p.y-1)) ); // top
              dir = 3;
            }else if(c(p.x+1,p.y)){
              storage.addPoint( (p = Point(p.x+1,p.y)) ); // right
              dir = 0;
            }else if(c(p.x,p.y+1)){
              storage.addPoint( (p = Point(p.x,p.y+1)) ); // bottom
              dir = 1;
            }else{
              storage.addPoint( (p = Point(p.x-1,p.y)) ); // left
              dir = 2;
            }
            break;
          case 1: // from top
            if(c(p.x+1,p.y)){
              storage.addPoint( (p = Point(p.x+1,p.y)) ); // right
              dir = 0;
            }else if(c(p.x,p.y+1)){
              storage.addPoint( (p = Point(p.x,p.y+1)) ); // bottom
              dir = 1;
            }else if(c(p.x-1,p.y)){
              storage.addPoint( (p = Point(p.x-1,p.y)) ); // left
              dir = 2;
            }else{
              storage.addPoint( (p = Point(p.x,p.y-1)) ); // top
              dir = 3;
            }
            break;
          case 2: // from right
            if(c(p.x,p.y+1)){
              storage.addPoint( (p = Point(p.x,p.y+1)) ); // bottom
              dir = 1;
            }else if(c(p.x-1,p.y)){
              storage.addPoint( (p = Point(p.x-1,p.y)) ); // left
              dir = 2;
            }else if(c(p.x,p.y-1)){
              storage.addPoint( (p = Point(p.x,p.y-1)) ); // top
              dir = 3;
            }else{
              storage.addPoint( (p = Point(p.x+1,p.y)) ); // right
              dir = 0;
            }
            break;
          case 3: // from bottom
            if(c(p.x-1,p.y)){
              storage.addPoint( (p = Point(p.x-1,p.y)) ); // left
              dir = 2;
            }else if(c(p.x,p.y-1)){
              storage.addPoint( (p = Point(p.x,p.y-1)) ); // top
              dir = 3;
            }else if(c(p.x+1,p.y)){
              storage.addPoint( (p = Point(p.x+1,p.y)) ); // right
              dir = 0;
            }else{
              storage.addPoint( (p = Point(p.x,p.y+1)) ); // bottom
              dir = 1;
            } 
            break;
          default:
            break;
        }
        if(ICL_UNLIKELY(p == pStart)){
          break;
        }
      }
    }
    
    void ContourDetector::Data::findContoursFast(Img8u &img){
      icl8u *d = img.begin(0);
      const int W = img.getWidth(), H = img.getHeight(), W1=W-1, H1=H-1;
      const int DIM = W*H;
      
      // the image border values have to be 0
      memset(d, 0, W);
      memset(d + DIM-W,0,W);
      
      for(int y = 1; y<H;++y){
        d[y*W] = 0;
        d[y*W-1] = 0;
      }  
      
      Channel8u c = img[0];
      
      storage.clear();
      
      for(int y=1;y<H1;++y){
        for(int x=1;x<W1;++x){
          if( c(x-1,y)==255 && c(x,y) == 0){
            storage.addContour();
            traceContour(Point(x,y),c);
          }
        }
      }
    }
  } // namespace cv
}

