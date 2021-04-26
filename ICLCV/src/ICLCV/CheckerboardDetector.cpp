/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CheckerboardDetector.cpp               **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLCV/CheckerboardDetector.h>
#include <ICLCore/OpenCV.h>
#include <ICLCore/CCFunctions.h>
#include <ICLCore/Color.h>

#include <opencv2/calib3d/calib3d_c.h>

namespace icl{

  using namespace core;
  using namespace utils;

  namespace cv{

    struct CheckerboardDetector::Data{
      CheckerboardDetector::Checkerboard cb;
      IplImage *ipl;
      Img8u grayBuf;
      Img8u buf8u;
      ImgBase *buf;
      Data():buf(0){}
      ~Data(){
        ICL_DELETE(buf);
      }
    };

    CheckerboardDetector::CheckerboardDetector():m_data(0){
      init_properties();
    }

    CheckerboardDetector::CheckerboardDetector(const Size &size):
      m_data(0){
      init_properties();
      init(size);
    }

    void CheckerboardDetector::init_properties(){
      addProperty("subpixel opt.enabled","flag","",true);
      addProperty("subpixel opt.radius","range","[1,15]:1",3);
      addProperty("subpixel opt.inner radius","range","[-1,15]:1",-1);
      addProperty("subpixel opt.max iterations","range","[10,100]:1",30);
      addProperty("subpixel opt.min error","range","[0.0001,1]",0.1);
    }

    CheckerboardDetector::~CheckerboardDetector(){
      if(m_data) delete m_data;
    }

    void CheckerboardDetector::init(const Size &size){
      if(!m_data){
        m_data = new Data;
        m_data->ipl = 0;
      }
      m_data->cb.size = size;
    }

    bool CheckerboardDetector::isNull() const{
      return !m_data;
    }

    const CheckerboardDetector::Checkerboard &
    CheckerboardDetector::detect(const core::ImgBase *image){
      if(!image) throw ICLException("CheckerboardDetector::detect(const core::ImgBase *image): image is null");
      if(image->getDepth() == depth8u){
        return detect(*image->as8u());
      }
      utils::Range<icl64f> mm = image->getMinMax();
      if(mm.minVal >= 0 && mm.maxVal <= 255){
        image->convert(&m_data->buf8u);
      }else{
        image->deepCopy(&m_data->buf);
        ImgBase *buf = m_data->buf;
        buf->normalizeImg(mm, Range64f(0,255));
        buf->convert(&m_data->buf8u);
      }
      return detect(m_data->buf8u);
    }

    const CheckerboardDetector::Checkerboard &
    CheckerboardDetector::detect(const Img8u &image){
      const Img8u *useImage = &image;
      if(image.getFormat() != formatGray && image.getChannels() != 1){
        m_data->grayBuf.setFormat(formatGray);
        m_data->grayBuf.setSize(image.getSize());

        cc(&image, &m_data->grayBuf);

        useImage = &m_data->grayBuf;
      }
      img_to_ipl(useImage, &m_data->ipl);
      std::vector<CvPoint2D32f> corners(m_data->cb.size.getDim());

      CvSize s = {m_data->cb.size.width, m_data->cb.size.height };

      int n = corners.size();
      m_data->cb.found = cvFindChessboardCorners(m_data->ipl, s, corners.data(), &n,
                                                  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

      bool optSubPix = getPropertyValue("subpixel opt.enabled");
      int radius = getPropertyValue("subpixel opt.radius");
      int innerR = getPropertyValue("subpixel opt.inner radius");
      if(innerR >= radius) innerR = radius -1;
      if(innerR == 0) innerR = -1;
      int maxIter = getPropertyValue("subpixel opt.max iterations");
      float minErr = getPropertyValue("subpixel opt.min error");

      if(m_data->cb.found && optSubPix){
        cvFindCornerSubPix(m_data->ipl, corners.data(), corners.size(), cvSize(radius,radius),
                           cvSize(innerR, innerR),
                           cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, maxIter, minErr));
      }

      if(m_data->cb.found){
        m_data->cb.corners.resize(corners.size());
        for(size_t i=0;i<corners.size();++i){
          m_data->cb.corners[i] = Point32f(corners[i].x, corners[i].y);
        }
      }else{
        m_data->cb.corners.clear();
      }

      return m_data->cb;
    }

    VisualizationDescription CheckerboardDetector::Checkerboard::visualize() const{
      VisualizationDescription vis;
      static const Color cs[5] = {
        Color(255,0,0),
        Color(255,255,0),
        Color(0,255,255),
        Color(0,0,255),
        Color(255,0,255)
      };
      vis.fill(0,0,0,0);
      for(int y=0, idx=0;y<size.height;++y){
        const Color &c = cs[y%5];
        vis.color(c[0],c[1],c[2],255);
        for(int x=0;x<size.width;++x,++idx){
          const Point32f &p = corners[idx];
          vis.circle(p.x+.5,p.y+.5,2);
          vis.line(p.x-1,p.y-1,p.x+1,p.y+1);
          vis.line(p.x+1,p.y-1,p.x-1,p.y+1);
          if(x || y){
            vis.line(p.x,p.y,corners[idx-1].x, corners[idx-1].y);
          }
        }
      }
      return vis;
    }
  } // namespace cv
} // namespace icl
