/***********************************************************************
 **                Image Component Library (ICL)                      **
 **                                                                   **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld            **
 **                         Neuroinformatics Group                    **
 ** Website: www.iclcv.org and                                        **
 **          http://opensource.cit-ec.de/projects/icl                 **
 **                                                                   **
 ** File   : ICLGeom/src/ICLGeom/CutfreeAdjacencyFeatureExtractor.cpp **
 ** Module : ICLGeom                                                  **
 ** Authors: Andre Ueckermann                                         **
 **                                                                   **
 **                                                                   **
 ** GNU LESSER GENERAL PUBLIC LICENSE                                 **
 ** This file may be used under the terms of the GNU Lesser General   **
 ** Public License version 3.0 as published by the                    **
 **                                                                   **
 ** Free Software Foundation and appearing in the file LICENSE.LGPL   **
 ** included in the packaging of this file.  Please review the        **
 ** following information to ensure the license requirements will     **
 ** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                  **
 **                                                                   **
 ** The development of this software was supported by the             **
 ** Excellence Cluster EXC 277 Cognitive Interaction Technology.      **
 ** The Excellence Cluster EXC 277 is a grant of the Deutsche         **
 ** Forschungsgemeinschaft (DFG) in the context of the German         **
 ** Excellence Initiative.                                            **
 **                                                                   **
 **********************************************************************/

#include <ICLGeom/CutfreeAdjacencyFeatureExtractor.h>
#include <ICLGeom/PlanarRansacEstimator.h>

namespace icl {
  namespace geom {
    struct CutfreeAdjacencyFeatureExtractor::Data {
	    Data(Mode mode) {
        if(mode==BEST || mode==GPU){
          ransac=new PlanarRansacEstimator(PlanarRansacEstimator::GPU);
        }else{
          ransac=new PlanarRansacEstimator(PlanarRansacEstimator::CPU);
        }
	    }

	    ~Data() {
	    }

      PlanarRansacEstimator* ransac;
    };


    CutfreeAdjacencyFeatureExtractor::CutfreeAdjacencyFeatureExtractor(Mode mode) :
	    m_data(new Data(mode)) {
    }


    CutfreeAdjacencyFeatureExtractor::~CutfreeAdjacencyFeatureExtractor() {
	    delete m_data;
    }


    math::DynMatrix<bool> CutfreeAdjacencyFeatureExtractor::apply(core::DataSegment<float,4> &xyzh,
                std::vector<std::vector<int> > &surfaces, math::DynMatrix<bool> &testMatrix, float euclideanDistance,
                int passes, int tolerance, core::Img32s labelImage){
      math::DynMatrix<bool> cutfreeMatrix(testMatrix);
      for(unsigned int x=0; x<cutfreeMatrix.rows(); x++){
        cutfreeMatrix(x,x)=false;
      }
      math::DynMatrix<PlanarRansacEstimator::Result> result = m_data->ransac->apply(xyzh, surfaces,
                    cutfreeMatrix, euclideanDistance, passes, tolerance,
                    PlanarRansacEstimator::ON_ONE_SIDE, labelImage);

      for(unsigned int x=0; x<result.rows(); x++){
        for(unsigned int y=0; y<result.cols(); y++){
          if(result(x,y).nacc>=result(x,y).acc){
            cutfreeMatrix(x,y)=false;
            cutfreeMatrix(y,x)=false;
          }
        }
      }
      return cutfreeMatrix;
    }


    math::DynMatrix<bool> CutfreeAdjacencyFeatureExtractor::apply(core::DataSegment<float,4> &xyzh,
                std::vector<std::vector<int> > &surfaces, math::DynMatrix<bool> &testMatrix, float euclideanDistance,
                int passes, int tolerance, core::Img32s labelImage,
                std::vector<SurfaceFeatureExtractor::SurfaceFeature> feature, float minAngle){
      math::DynMatrix<bool> cutfreeMatrix(testMatrix);
      for(unsigned int x=0; x<cutfreeMatrix.rows(); x++){
        cutfreeMatrix(x,x)=false;
        for(unsigned int y=0; y<cutfreeMatrix.cols(); y++){
          if(cutfreeMatrix(x,y)==true || cutfreeMatrix(y,x)==true){
            Vec n1=feature[x].meanNormal;
            Vec n2=feature[y].meanNormal;
            float a1 = (n1[0] * n2[0]+ n1[1] * n2[1]+ n1[2] * n2[2]);
	          float ang=acos(a1)*180./M_PI;
	          if(ang<minAngle){
              cutfreeMatrix(x,y)=false;
              cutfreeMatrix(y,x)=false;
            }
          }
        }
      }
      math::DynMatrix<PlanarRansacEstimator::Result> result = m_data->ransac->apply(xyzh, surfaces,
                    cutfreeMatrix, euclideanDistance, passes, tolerance,
                    PlanarRansacEstimator::ON_ONE_SIDE, labelImage);

      for(unsigned int x=0; x<result.rows(); x++){
        for(unsigned int y=0; y<result.cols(); y++){
          if(result(x,y).nacc>=result(x,y).acc){
            cutfreeMatrix(x,y)=false;
            cutfreeMatrix(y,x)=false;
          }
        }
      }
      return cutfreeMatrix;
    }

  } // namespace geom
}
