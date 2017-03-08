/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/LensUndistortionCalibrator.cpp     **
** Module : ICLGeom                                                **
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

#include <ICLCV/LensUndistortionCalibrator.h>
#include <ICLCore/OpenCV.h>
#include <ICLUtils/StringUtils.h>

#include <opencv/cv.h>


namespace icl{

  using namespace utils;
  using namespace math;
  using namespace core;

  namespace cv{
    
    typedef CvMatWrapper<float> CvMatF;
    typedef CvMatWrapper<int> CvMatI;
    
    struct LensUndistortionCalibrator::Data{
      std::vector<Point32f> points;
      std::vector<Point32f> objPoints;
      std::vector<int> subSetSizes;
      LensUndistortionCalibrator::GridDefinition gridDef;
      Size imageSize;
    };
      
    LensUndistortionCalibrator::GridDefinition::GridDefinition(const Size &dims, const utils::Size32f &size){
      for(int y=0;y<dims.height;++y){
        for(int x=0;x<dims.width;++x){
          push_back(Point32f(float(x)*size.width, float(y)*size.height));
        }
      }
      m_gridBoundarySize = Size32f(dims.width * size.width, dims.height * size.height);
    }
    
    static Size init_grid(std::vector<Point32f> &dst,
                          const Size &dims, 
                          const Size32f &markerSize, 
                          const Size32f &markerSpacing){
      const float w = markerSize.width*0.5, h = markerSize.height*0.5;
      const Point32f o(w,h);
      const Point32f ds[4] = { Point32f(-w, -h)+o, Point32f(w, -h)+o, Point32f(w, h)+o, Point32f(-w, h)+o };
      for(int y=0;y<dims.height;++y){
        for(int x=0;x<dims.width;++x){
          const Point32f  c(x * markerSpacing.width,
                            y * markerSpacing.height);
          for(int i=0;i<4;++i){
            dst.push_back(c+ds[i]);
          }
        }
      }
      return Size((dims.width-1) * markerSpacing.width + markerSize.width,
                  (dims.height-1) * markerSpacing.height + markerSize.height);

    }
    
    LensUndistortionCalibrator::GridDefinition::GridDefinition(const Size &markerGridDims, 
                                                             const Size32f &markerSize, 
                                                             const Size32f &markerSpacing){
      m_gridBoundarySize = init_grid(*this, markerGridDims, markerSize, markerSpacing);
    }
    LensUndistortionCalibrator::GridDefinition::GridDefinition(const Size &markerGridDims, 
                                                               float markerDim, 
                                                               float markerSpacing){
      m_gridBoundarySize = init_grid(*this, markerGridDims, Size32f(markerDim,markerDim), Size32f(markerSpacing,markerSpacing));
    }

    LensUndistortionCalibrator::LensUndistortionCalibrator():m_data(0){
    
    }
    
    LensUndistortionCalibrator::LensUndistortionCalibrator(const Size &imageSize, const GridDefinition &gridDef):m_data(0){
      init(imageSize,gridDef);
    }
    
    LensUndistortionCalibrator::~LensUndistortionCalibrator(){
      if(m_data) delete m_data;
    }

    void LensUndistortionCalibrator::init(const Size &imageSize, const GridDefinition &gridDef){
      if(!m_data) m_data = new Data;
      m_data->gridDef = gridDef; // implicitly sliced here!
      m_data->imageSize = imageSize;
      clear();
    }

    void LensUndistortionCalibrator::init(const Size &imageSize){
      if(!m_data) m_data = new Data;
      m_data->imageSize = imageSize;
      clear();
    }

    bool LensUndistortionCalibrator::isNull() const{
      return !m_data;
    }
      
    void LensUndistortionCalibrator::addPoints(const std::vector<Point32f> &imagePoints){
      addPoints(imagePoints, m_data->gridDef);
    }

    void LensUndistortionCalibrator::addPoints(const std::vector<Point32f> &imagePoints, const std::vector<utils::Point32f> &gridDef){
      ICLASSERT_THROW(m_data, ICLException("LensUndistortionCalibrator::addPoints: instance is null"));
      ICLASSERT_THROW(imagePoints.size() == gridDef.size(),
        ICLException("LensUndistortionCalibrator::addPoints: "
        "number of points in the grid does not match with the number of image points ("
                     "imagePoints.size():" + str(imagePoints.size()) + " gridDef.size(): "
                     + str(gridDef.size()) + ")"));
      std::copy(imagePoints.begin(), imagePoints.end(), std::back_inserter(m_data->points));
      std::copy(gridDef.begin(), gridDef.end(), std::back_inserter(m_data->objPoints));
      m_data->subSetSizes.push_back(imagePoints.size());
    }

    void LensUndistortionCalibrator::clear(){
      ICLASSERT_THROW(m_data,ICLException("LensUndistortionCalibrator::clear: instance is null"));
      m_data->points.clear();
      m_data->objPoints.clear();
      m_data->subSetSizes.clear();
    }

    void LensUndistortionCalibrator::undoLast(){
      if(!m_data->subSetSizes.size()) return;
      int lastSize = m_data->subSetSizes.back();
      m_data->points.resize(m_data->points.size()-lastSize);
      m_data->objPoints.resize(m_data->objPoints.size()-lastSize);
      m_data->subSetSizes.pop_back();
    }
    
    io::ImageUndistortion LensUndistortionCalibrator::computeUndistortion(){
      ICLASSERT_THROW(m_data,ICLException("LensUndistortionCalibrator::computeUndistortion: instance is null"));
      
      const std::vector<Point32f> &ps = m_data->points;
      
      CvMatF O(ps.size(),3), I(ps.size(),2), intr(3,3), dist(5,1);
      CvMatI cs(m_data->subSetSizes.size(),1);

      for(size_t i=0;i<ps.size();++i){
        O(i,0) = m_data->objPoints[i].y;
        O(i,1) = m_data->objPoints[i].x;
        O(i,2) = 0;

        I(i,0) = ps[i].x;
        I(i,1) = ps[i].y;
      }
      
      for (unsigned int i = 0; i<m_data->subSetSizes.size(); ++i){
        cs(i,0) = m_data->subSetSizes[i];
      }
      CvSize s = { m_data->imageSize.width, m_data->imageSize.height };

      for(int y=0;y<3;++y){      
        for(int x=0;x<3;++x){
          intr(x,y) = 0;
        }
      }
      intr(0,0) = intr(1,1) = 1.0f;

      cvCalibrateCamera2(O.get(), I.get(), cs.get(), s, intr.get(), 
                         dist.get(), NULL, NULL, CV_CALIB_FIX_ASPECT_RATIO);

      std::vector<double> params(10);
      params[0] = intr(0, 0);
      params[1] = intr(1, 1);
      params[2] = intr(0, 2);
      params[3] = intr(1, 2);
      params[4] = 0.;
      for(int i=0;i<5;++i){
        params[i+5] = dist(i,0);
      }

      
      return io::ImageUndistortion("MatlabModel5Params", params, 
                                   m_data->imageSize);
    }

    LensUndistortionCalibrator::Info LensUndistortionCalibrator::getInfo(){
      ICLASSERT_THROW(m_data,ICLException("LensUndistortionCalibrator::getInfo: instance is null"));
      Info info = { m_data->imageSize, (int)m_data->points.size(), 
                    (int)m_data->subSetSizes.size(), m_data->gridDef };
      return info;
    }
  }
  
}
