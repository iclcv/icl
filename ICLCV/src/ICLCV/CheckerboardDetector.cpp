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

#ifdef ICL_HAVE_OPENCV
#include <opencv/cv.h>
#endif

namespace icl{
  
  namespace cv{
    
    struct CheckerBoardDetector::Data{
      CheckerBoardDetector::CheckerBoard cb;
      bool optSubPix;
      IplImage *ipl;
      core::Img8u grayBuf;
    };

    CheckerBoardDetector::CheckerBoardDetector():m_data(0){}

    CheckerBoardDetector::CheckerBoardDetector(const utils::Size &size, bool optSubPix):
      m_data(0){
      init(size,optSubPix);
    }

    CheckerBoardDetector::~CheckerBoardDetector(){
      if(m_data) delete m_data;
    }
      
    void CheckerBoardDetector::init(const utils::Size &size, bool optSubPix){
      if(!m_data){ 
        m_data = new Data;
        m_data->ipl = 0;
      }
      m_data->cb.size = size;
      m_data->optSubPix = optSubPix;
    }
   
    bool CheckerBoardDetector::isNull() const{
      return !m_data;
    }

    const CheckerBoardDetector::CheckerBoard &
    CheckerBoardDetector::detect(const core::Img8u &image){
      const core::Img8u *useImage = &image;
      if(image.getFormat() != core::formatGray && image.getChannels() != 1){
        m_data->grayBuf.setFormat(core::formatGray);
        m_data->grayBuf.setSize(image.getSize());
        
        cc(&image, &m_data->grayBuf);
        
        useImage = &m_data->grayBuf;
      }
      core::img_to_ipl(useImage, &m_data->ipl);
      std::vector<CvPoint2D32f> corners(m_data->cb.size.getDim());

      CvSize s = {m_data->cb.size.width, m_data->cb.size.height };
      
      int n = corners.size();
      m_data->cb.found = cvFindChessboardCorners(m_data->ipl, s, corners.data(), &n,
                                                  CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
      
      if(m_data->cb.found && m_data->optSubPix){
        cvFindCornerSubPix(m_data->ipl, corners.data(), corners.size(), cvSize(3,3),
                           cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
      }
      
      if(m_data->cb.found){
        m_data->cb.corners.resize(corners.size());
        for(size_t i=0;i<corners.size();++i){
          m_data->cb.corners[i] = utils::Point32f(corners[i].x, corners[i].y);
        }
      }else{
        m_data->cb.corners.clear();
      }

      return m_data->cb;
    }
  } // namespace cv
} // namespace icl
