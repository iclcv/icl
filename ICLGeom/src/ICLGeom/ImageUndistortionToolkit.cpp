/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/ImageUndistortionToolkit.cpp       **
** Module : ICLGeom                                                **
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

#include <ICLGeom/ImageUndistortionToolkit.h>

#ifdef ICL_HAVE_OPENCV
#include <opencv/cv.h>
#endif


namespace icl{

  using namespace utils;
  using namespace math;

  namespace geom{
    
    
    namespace {
      struct CvMatF{
        CvMat *m;
        CvMatF(int nrows, int ncols):
          m( cvCreateMat(nrows, ncols, CV_32FC1) ){}
        float &operator()(int y, int x) {
          return CV_MAT_ELEM(*m,float,y,x);
        }
        ~CvMatF(){
          cvReleaseMat(&m);
        }
      };

      struct CvMatI{
        CvMat *m;
        CvMatI(int nrows, int ncols):
          m( cvCreateMat(nrows, ncols, CV_32SC1) ){}
        int &operator()(int y, int x) {
          return CV_MAT_ELEM(*m,int,y,x);
        }
        ~CvMatI(){
          cvReleaseMat(&m);
        }
      };

    }
    
    struct ImageUndistortionToolkit::Data{
      std::vector<Point32f> points;
      int nSubSets;
      std::vector<Point32f> gridDef;
      Size imageSize;
    };
      
    ImageUndistortionToolkit::GridDefinition::GridDefinition(const Size &dims){
      
    }
    
    ImageUndistortionToolkit::GridDefinition::GridDefinition(const Size &markerGridDims, 
                                                             const Size32f &markerSize, 
                                                             const Size32f &markerSpacing){
    
    }
    ImageUndistortionToolkit::GridDefinition::GridDefinition(const Size &markerGridDims, 
                                                             float markerDim, 
                                                             float markerSpacing){
    
    }

    ImageUndistortionToolkit::ImageUndistortionToolkit():m_data(0){
    
    }
    
    ImageUndistortionToolkit::ImageUndistortionToolkit(const Size &imageSize, const GridDefinition &gridDef):m_data(0){
      init(imageSize,gridDef);
    }
    
    ImageUndistortionToolkit::~ImageUndistortionToolkit(){
      if(m_data) delete m_data;
    }

    void ImageUndistortionToolkit::init(const Size &imageSize, const GridDefinition &gridDef){
      if(!m_data) m_data = new Data;
      m_data->gridDef = gridDef; // implicitly sliced here!
      m_data->imageSize = imageSize;
      clear();
    }

    bool ImageUndistortionToolkit::isNull() const{
      return !m_data;
    }
      
    void ImageUndistortionToolkit::addPoints(const std::vector<Point32f> &imagePoints){
      ICLASSERT_THROW(m_data,ICLException("ImageUndistortionToolkit::addPoints: instance is null"));
      std::copy(imagePoints.begin(),imagePoints.end(), std::back_inserter(m_data->points));
      ++m_data->nSubSets;
    }
    
    void ImageUndistortionToolkit::clear(){
      ICLASSERT_THROW(m_data,ICLException("ImageUndistortionToolkit::clear: instance is null"));
      m_data->points.clear();
      m_data->nSubSets = 0;
    }
    
    io::ImageUndistortion ImageUndistortionToolkit::computeUndistortion(){
      ICLASSERT_THROW(m_data,ICLException("ImageUndistortionToolkit::computeUndistortion: instance is null"));
      
      const std::vector<Point32f> &ps = m_data->points;
      
      CvMatF O(ps.size(),3), I(ps.size(),2), intr(3,3), dist(5,1);
      CvMatI cs(m_data->nSubSets,1);

      for(size_t i=0;i<ps.size();++i){

        int g = i % m_data->gridDef.size();

        O(i,0) = m_data->gridDef[g].x;
        O(i,1) = m_data->gridDef[g].y;
        O(i,2) = 0;

        I(i,0) = ps[i].x;
        I(i,1) = ps[i].y;
      }
      
      for(int i=0;i<m_data->nSubSets;++i){
        cs(i,0) = (int)m_data->gridDef.size();
      }
      
      CvSize s = { m_data->imageSize.width, m_data->imageSize.height };
      cvCalibrateCamera2(O.m, I.m, cs.m, s, intr.m, dist.m, NULL, NULL, CV_CALIB_FIX_ASPECT_RATIO);
      
      std::vector<double> params(5);
      for(int i=0;i<5;++i){
        params[i] = intr(i,0);
      }

      
      return io::ImageUndistortion("MatlabModel5Params", params, 
                                   m_data->imageSize);
    }

    ImageUndistortionToolkit::Info ImageUndistortionToolkit::getInfo(){
      ICLASSERT_THROW(m_data,ICLException("ImageUndistortionToolkit::getInfo: instance is null"));
      Info info = { m_data->imageSize, (int)m_data->points.size(), m_data->nSubSets };
      return info;
    }
  }
  
}
