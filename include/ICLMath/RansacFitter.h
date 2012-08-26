/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/RansacFitter.h                         **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_RANSAC_FITTER_H
#define ICL_RANSAC_FITTER_H

#include <ICLMath/DynVector.h>
#include <ICLMath/FixedVector.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Random.h>

namespace icl{
 
  
  /// Generic RANSAC (RAndom SAmpling Consensus) Implementation
  /** The RansacFitter provides a generic framework, for RANSAC based model fitting.
      
      \section ALG RANSAC Algorithm
      The RANSAC Algorithm is well described on Wikipedia 
      @see http://de.wikipedia.org/wiki/RANSAC-Algorithmus
      
      \section EX Example
      We will here show an example application that is used
      for fitting a simple 2D line: The example code is also available
      as an example application: ICL/ICLMath/examples/ransac-test.cpp
     
      \code
      #include <ICLMath/ProgArg.h>
      #include <ICLMath/StringUtils.h>
      #include <ICLMath/FixedVector.h>
      #include <ICLMath/RansacFitter.h>
      #include <ICLMath/Random.h>
      #include <ICLMath/Point32f.h>
      
      using namespace icl;
      
      // the line model (y=mx+b) is defined by two values
      typedef FixedColVector<float,2> Line;

      // the fitting is done using standard least squares approach
      Line fit_line(const std::vector<Point32f> &pts){
        int n = pts.size();
        DynMatrix<float> xs(n,2), ys(n,1);
        for(int i=0;i<n;++i){
          xs(i,0) = 1;
          xs(i,1) = pts[i].x;
          ys(i,0) = pts[i].y;
        } 
        DynMatrix<float> fit = ys * xs.pinv(true);
        return Line(fit[0],fit[1]);
      }
      
      // distance function for single points (y-distance here)
      double line_dist(const Line &line, const Point32f &p){
        return sqr(line[0] + line[1]*p.x - p.y);
      }

      // the original line
      static const Line THE_LINE(1.23456, 7.89);

      // create test data:
      // 50% noisy point on the line
      // 50% random outliers
      const std::vector<Point32f> get_line_points(){
        Line l = THE_LINE;
        std::vector<Point32f> pts(100);
        URand r(-100,100);
        GRand gr(0,1);
        for(int i=0;i<50;++i){
          pts[i].x = r;
          pts[i].y = l[0] + l[1]* pts[i].x + gr;
        }
        for(int i=0;i<50;++i){
          pts[i+50] = Point32f(r,r);
        }
        return pts;
      }

      int main(int n, char **ppc){
        randomSeed();
      
        // create the fitter
        RansacFitter<Point32f,Line> fitLine(2,1000,fit_line,line_dist,1.5,30);
      
        // fit ...
        RansacFitter<Point32f,Line>::Result r = fitLine.fit(get_line_points());

        // show results
        std::cout << "original line was " << THE_LINE.transp() << std::endl;
        std::cout << "fitted result was " << r.model.transp() << std::endl;
        std::cout << "fitting error was " << r.error << std::endl;
      }
      
      // output could be something like:
      // original line was | 1.23456 7.89 |
      // fitted result was | 1.3565 7.88645 |
      // fitting error was 0.188892

      \endcode

      \section TEM Template Parameters
      The two tempalte parameters are kept very general. Therefore, there
      are just a few restrictions for the DataPoint and Model classes.
      - default constructable
      - copyable
  */
  template<class DataPoint=std::vector<float>,
           class Model=std::vector<float> >
  class RansacFitter{
    public:
    /// DataSet type (just a set of DataPoint instances)
    typedef std::vector<DataPoint> DataSet;
    
    /// Function for the fitting module (gets a dataset and returns the fitted model)
    typedef Function<Model,const DataSet&> ModelFitting;
    
    /// Error function for single points
    typedef Function<icl64f,const Model&,const DataPoint&> PointError;
    
    private:
    /// minimum points that are used to create a coarse model
    int m_minPointsForModel;
    
    /// number of iterations
    int m_iterations;
    
    /// maximum distance of a point to the model to become an inlier
    icl64f m_maxModelDistance;
    
    /// minimum amount of inliers for a 'good' model
    int m_minClosePointsForGoodModel;
    
    /// fitting function
    ModelFitting m_fitting;
    
    /// point-model error function
    PointError m_err;
    
    /// min error criterion for early exit
    icl64f m_minErrorExit;
    
    public:
    /// result structure
    struct Result{
      /// reached error
      icl64f error;
      
      /// model (zero sized if no model was found)
      Model model;
      
      /// consensus set of best match (i.e. inliers)
      /** empty if no model was found */
      DataSet consensusSet;

      /// number of iterations needed
      int iterationCount;
      
      /// returns whether any model was found
      bool found() const { return consensusSet.size(); }
    };
    
    private:
    /// internal result buffer
    Result m_result;
    
    /// internal utility method
    static inline bool find_in(const std::vector<int> &v, int i, int n){
      return std::find(v.data(), v.data()+n, i) != v.data()+n;
    }

    /// internal utility method
    void find_random_consensus_set(DataSet &currConsensusSet,
                                   const DataSet &allPoints,
                                   std::vector<int> &usedIndices){
      const int n = currConsensusSet.size();
      URandI r(allPoints.size()-1);
      
      for(int i=0;i<n;++i){
        do { usedIndices[i] = r; } while ( find_in(usedIndices, usedIndices[i], i-1) );
        currConsensusSet[i] = allPoints[ usedIndices[i] ];
      }
    }
    
    public:
    /// empty constructor (creates a dummy instance)
    RansacFitter(){}
    
    /// constructor with given parameters
    /** The parameters functionality is documented with the
        analogously named member variables */
    RansacFitter(int minPointsForModel, 
                 int iterations, 
                 ModelFitting fitting,
                 PointError err,
                 icl64f maxModelDistance,
                 int minClosePointsForGoodModel,
                 icl64f minErrorExit=0):
      m_minPointsForModel(minPointsForModel),
      m_iterations(iterations),
      m_maxModelDistance(maxModelDistance),
      m_minClosePointsForGoodModel(minClosePointsForGoodModel),
      m_fitting(fitting),m_err(err),
      m_minErrorExit(minErrorExit){
    }
    
    /// fitting function (actual RANSAC algorithm)
    const Result &fit(const DataSet &allPoints){
      m_result.model = Model();
      m_result.consensusSet.clear();
      m_result.error = Range64f::limits().maxVal;
      m_result.iterationCount = 0;
      std::vector<DataPoint> consensusSet(m_minPointsForModel);
      std::vector<int> usedIndices(m_minPointsForModel);
      
      int i = 0;
      for(i=0;i<m_iterations;++i){
        consensusSet.resize(m_minPointsForModel);
        find_random_consensus_set(consensusSet, allPoints, usedIndices);
        
        Model model = m_fitting(consensusSet);
        for(int j=0;j<(int)allPoints.size();++j){
          if(find_in(usedIndices, j, usedIndices.size())) continue;
          if(m_err(model, allPoints[j]) < m_maxModelDistance){
            consensusSet.push_back(allPoints[j]);
          }
        }
        
        if((int)consensusSet.size() >= m_minClosePointsForGoodModel){
          model = m_fitting(consensusSet);
          double error = 0;
          for(unsigned int j=0;j<consensusSet.size();++j){
            error += m_err(model, consensusSet[j]);
          }
          error /= consensusSet.size();
          
          if(error < m_result.error){
            m_result.error = error;
            m_result.model = model;
            m_result.consensusSet = consensusSet;
            if(m_result.error < m_minErrorExit){
              m_result.iterationCount = i;
              return m_result;
            }
          }
        }
      }
      m_result.iterationCount = i;
      return m_result;
    }
  };
}

#endif
