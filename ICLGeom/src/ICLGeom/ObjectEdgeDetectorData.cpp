/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetectorData.cpp         **
 ** Module : ICLGeom                                                **
 ** Authors: Andre Ueckermann                                       **
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

#include <ICLGeom/ObjectEdgeDetectorData.h>

namespace icl {
using namespace math;
namespace geom {

ObjectEdgeDetectorData::ObjectEdgeDetectorData(){
}

ObjectEdgeDetectorData::~ObjectEdgeDetectorData() {
}

ObjectEdgeDetectorData::m_kernel ObjectEdgeDetectorData::getKernel(int size){
    m_kernel returnKernel;
	
	//set default values
    returnKernel.norm = 1;
    returnKernel.kernel = DynMatrix<float>(1, 1, 0.0);
    returnKernel.l = 0;
    returnKernel.kSize = 1;
    returnKernel.rowSize = 1;
	
	if (size <= 1) {
	    // nothing!
    } else if (size <= 3) {
	    returnKernel.norm = 16.;
	    returnKernel.l = 1;
	    returnKernel.kSize = 3 * 3;
	    returnKernel.rowSize = 3;
	    DynMatrix<float> k1 = DynMatrix<float>(1, 3, 0.0);
	    k1(0, 0) = 1.;
	    k1(0, 1) = 2.;
	    k1(0, 2) = 1.;
	    returnKernel.kernel = k1 * k1.transp();
    } else if (size <= 5) {
	    returnKernel.norm = 256.;
	    returnKernel.l = 2;
	    returnKernel.kSize = 5 * 5;
	    returnKernel.rowSize = 5;
	    DynMatrix<float> k1 = DynMatrix<float>(1, 5, 0.0);
	    k1(0, 0) = 1.;
	    k1(0, 1) = 4.;
	    k1(0, 2) = 6.;
	    k1(0, 3) = 4.;
	    k1(0, 4) = 1.;
	    returnKernel.kernel = k1 * k1.transp();
    } else {
	    returnKernel.norm = 4096.;
	    returnKernel.l = 3;
	    returnKernel.kSize = 7 * 7;
	    returnKernel.rowSize = 7;
	    DynMatrix<float> k1 = DynMatrix<float>(1, 7, 0.0);
	    k1(0, 0) = 1.;
	    k1(0, 1) = 6.;
	    k1(0, 2) = 15.;
	    k1(0, 3) = 20.;
	    k1(0, 4) = 15.;
	    k1(0, 5) = 6.;
	    k1(0, 6) = 1.;
	    returnKernel.kernel = k1 * k1.transp();
    }

    return returnKernel;
}
	
ObjectEdgeDetectorData::m_params ObjectEdgeDetectorData::getParameters(){
    m_params returnParams;
    
    returnParams.medianFilterSize = 3;
	returnParams.normalRange = 2;
	returnParams.normalAveragingRange = 1;
	returnParams.neighborhoodMode = 0;
	returnParams.neighborhoodRange = 3;
	returnParams.binarizationThreshold = 0.89;
	returnParams.useNormalAveraging = true;
	returnParams.useGaussSmoothing = false;
    
    return returnParams;
}


} // namespace geom
}
