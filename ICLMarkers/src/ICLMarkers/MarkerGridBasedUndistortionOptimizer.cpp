/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/                             **
**          MarkerGridBasedUndistortionOptimizer.cpp               **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/MarkerGridBasedUndistortionOptimizer.h>
#include <ICLMarkers/MarkerGridEvaluater.h>
#include <ICLUtils/SmartPtr.h>
#include <ICLMarkers/InverseUndistortionProcessor.h>

namespace icl{
  namespace markers{
    using namespace utils;
    
    struct MarkerGridBasedUndistortionOptimizer::Data{
      std::vector<SmartPtr<MarkerGrid> > grids;
      std::vector<SmartPtr<MarkerGrid> > ugrids;

      InverseUndistortionProcessor iup;
      Data():iup(true){}
    };
    
    MarkerGridBasedUndistortionOptimizer::MarkerGridBasedUndistortionOptimizer():
      m_data(new Data){}
    MarkerGridBasedUndistortionOptimizer::~MarkerGridBasedUndistortionOptimizer(){
      delete m_data;
    }
  
    void MarkerGridBasedUndistortionOptimizer::add(const MarkerGrid &grid){
      m_data->grids.push_back(new MarkerGrid(grid));
      m_data->grids.back()->detach();
    }

  
    int MarkerGridBasedUndistortionOptimizer::size() const{
      return m_data->grids.size();
    }
  
    
    void MarkerGridBasedUndistortionOptimizer::clear(){
      m_data->grids.clear();
    }

    void MarkerGridBasedUndistortionOptimizer::undistort(const MarkerGrid &src,
                                      MarkerGrid &dst, const float k[7]) const{
      dst.setSize(src.getSize());
      
      int n = src.getDim()*4;
      std::vector<Point32f> sp(n);
      int idx = -1;
      MarkerGrid::iterator jt = dst.begin();
      for(MarkerGrid::const_iterator it = src.begin(); it != src.end(); ++it, ++jt){
        it->getImagePointsTo(sp.data()+(++idx)*4);
        jt->setFound(it->wasFound());
      }
      
      
      const std::vector<Point32f> &out = m_data->iup.run(sp, k);
      
      idx = -1;
      for(MarkerGrid::iterator it = dst.begin(); it != dst.end(); ++it){
        it->setImagePoints(out.data()+(++idx)*4);
      }
    }
    void  MarkerGridBasedUndistortionOptimizer::setUseOpenCL(bool on){
      m_data->iup.setPreferOpenCL(on);
    }
    
    /// k = k0,k1,k2,k3,k4, ix-offset, iy-offset
    float MarkerGridBasedUndistortionOptimizer::computeError(const float k[7]){
      //Time t = Time::now();
      std::vector<SmartPtr<MarkerGrid> > &gs = m_data->grids;
      std::vector<SmartPtr<MarkerGrid> > &us = m_data->ugrids;

      if(!gs.size()) return -1;
      // DEBUG_LOG("k:" << k[0] << "," << k[1] << "," << k[2] << "," << k[3] << "," 
      //          << k[4] << "," << k[5] << "," << k[6]);
      float error = 0;
      us.resize(gs.size()); // shallow copy for existing ones
      for(size_t i=0;i<us.size();++i){
        if(!us[i]) us[i] = new MarkerGrid;  // only on-demand creation
        undistort(*gs[i], *us[i], k);
        // dst points in-place
        float e = MarkerGridEvaluater::compute_error(*us[i]);
        //DEBUG_LOG("distorted error: " << gs[i]->computeError()
        //          << "UNdistorted error for grid " << i << " : " 
        //          << e);
        error += e;
      }
      //t.showAge("time for computeError with " + str(gs.size()) + " grids");
      return error/float(us.size());
    }
    
    std::vector<float> MarkerGridBasedUndistortionOptimizer::optimizeSample(const float kInit[7],
                                                         int idx, float min, float max, 
                                                         const std::vector<int> steps){
      std::vector<float> k(kInit,kInit+7);

      float opt = 0;
      for(size_t s = 0;s<steps.size();++s){
        int ns = steps[s];
        // sample [min, max] in ns steps
        float span = max - min;
        float d = span / ns;


        int minIdx = 0;
        float minError = 1.e10;
        for(int i=0;i<ns;++i){
          k[idx] = min + i * d;
          float e = computeError(k.data());
          if(e < minError){
            minIdx = i;
            minError = e;
          }
        }
        opt = min + minIdx * d;
        //DEBUG_LOG("performed step " << s << " [" << min << "," << max 
        //          << "] (span was " << span << ", d was " << d << ") "
        //          << "opt-value: " << opt << " gave error: " << minError);
                
        min = opt - d;
        max = opt + d;
      }
      k[idx] = opt;
      return k;
    }
  
    std::vector<float> MarkerGridBasedUndistortionOptimizer::optimizeAuto(const utils::Size &imageSize){
      const float kData[7] = {0,0,0,0,0, imageSize.width/2.f, imageSize.height/2.f};
      std::vector<float> k(kData,kData+7);
      static const float deltas[7] = {0.5,0.5,0.5,0.5,0.5,50,50};
      float deltaFactor = 2.0;
      int N_OUTER_LOOPS = 12;
      for(int outer=0;outer<N_OUTER_LOOPS;++outer){
        for(int k_index = 0; k_index < 7; ++k_index){
          float d = deltas[k_index]*deltaFactor;
          k = optimizeSample(k.data(), k_index, k[k_index]-d, k[k_index]+d, std::vector<int>(4,100));
        }
        deltaFactor /= 2;
      }
      return k;
    }
  }
}

