/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/src/ICLGeom/ObjectEdgeDetector.cpp             **
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

#include <ICLGeom/ObjectEdgeDetector.h>

#ifdef ICL_HAVE_OPENCL
#include <ICLGeom/ObjectEdgeDetectorGPU.h>
#endif


#include <ICLGeom/ObjectEdgeDetectorCPU.h>

namespace icl {
using namespace core;
using namespace utils;
namespace geom {

ObjectEdgeDetector::ObjectEdgeDetector(Size size, ObjectEdgeDetectorMode mode){
    if(mode==GPU){
        std::cout<<"GPU"<<std::endl;
        #ifdef ICL_HAVE_OPENCL
        objectEdgeDetector = new ObjectEdgeDetectorGPU(size);
        if(objectEdgeDetector->isCLReady()==false){
            std::cout<<"OpenCL is not available"<<std::endl;//End
        }
        #else
        std::cout<<"OpenCL is not available"<<std::endl;//End
        #endif
    }else if(mode==CPU){
        std::cout<<"CPU"<<std::endl;
        objectEdgeDetector = new ObjectEdgeDetectorCPU(size);
    }else{
        std::cout<<"BEST"<<std::endl;
        #ifdef ICL_HAVE_OPENCL
        objectEdgeDetector = new ObjectEdgeDetectorGPU(size);
        if(objectEdgeDetector->isCLReady()==false){
            objectEdgeDetector = new ObjectEdgeDetectorCPU(size);
        }
        #else
        objectEdgeDetector = new ObjectEdgeDetectorCPU(size);
        #endif
    }
        
}

ObjectEdgeDetector::~ObjectEdgeDetector() {
	delete objectEdgeDetector;
}

void ObjectEdgeDetector::setDepthImage(const Img32f &depthImg) {
    objectEdgeDetector->setDepthImage(depthImg);
}

void ObjectEdgeDetector::applyMedianFilter() {
    objectEdgeDetector->applyMedianFilter();
}

const Img32f &ObjectEdgeDetector::getFilteredDepthImage() {
    return objectEdgeDetector->getFilteredDepthImage();
}

void ObjectEdgeDetector::setFilteredDepthImage(const Img32f &filteredImg) {
    objectEdgeDetector->setFilteredDepthImage(filteredImg);
}

void ObjectEdgeDetector::applyNormalCalculation() {
    objectEdgeDetector->applyNormalCalculation();
}

void ObjectEdgeDetector::applyLinearNormalAveraging() {
    objectEdgeDetector->applyLinearNormalAveraging();
}

void ObjectEdgeDetector::applyGaussianNormalSmoothing() {
    objectEdgeDetector->applyGaussianNormalSmoothing();
}

const Vec *ObjectEdgeDetector::getNormals() {
    return objectEdgeDetector->getNormals();
}

void ObjectEdgeDetector::applyWorldNormalCalculation(const Camera &cam) {
    objectEdgeDetector->applyWorldNormalCalculation(cam);
}

const Vec* ObjectEdgeDetector::getWorldNormals() {
    return objectEdgeDetector->getWorldNormals();
}

const core::Img8u &ObjectEdgeDetector::getRGBNormalImage() {
    return objectEdgeDetector->getRGBNormalImage();
}

void ObjectEdgeDetector::setNormals(Vec* pNormals) {
    objectEdgeDetector->setNormals(pNormals);
}

void ObjectEdgeDetector::applyAngleImageCalculation() {
    objectEdgeDetector->applyAngleImageCalculation();
}

const Img32f &ObjectEdgeDetector::getAngleImage() {
    return objectEdgeDetector->getAngleImage();
}

void ObjectEdgeDetector::setAngleImage(const Img32f &angleImg) {
    objectEdgeDetector->setAngleImage(angleImg);
}

void ObjectEdgeDetector::applyImageBinarization() {
    objectEdgeDetector->applyImageBinarization();
}

const Img8u &ObjectEdgeDetector::getBinarizedAngleImage() {
    return objectEdgeDetector->getBinarizedAngleImage();
}

void ObjectEdgeDetector::setMedianFilterSize(int size) {
    objectEdgeDetector->setMedianFilterSize(size);
}

void ObjectEdgeDetector::setNormalCalculationRange(int range) {
    objectEdgeDetector->setNormalCalculationRange(range);
}

void ObjectEdgeDetector::setNormalAveragingRange(int range) {
    objectEdgeDetector->setNormalAveragingRange(range);
}

void ObjectEdgeDetector::setAngleNeighborhoodMode(int mode) {
    objectEdgeDetector->setAngleNeighborhoodMode(mode);
}

void ObjectEdgeDetector::setAngleNeighborhoodRange(int range) {
    objectEdgeDetector->setAngleNeighborhoodRange(range);
}

void ObjectEdgeDetector::setBinarizationThreshold(float threshold) {
    objectEdgeDetector->setBinarizationThreshold(threshold);
}

void ObjectEdgeDetector::setUseNormalAveraging(bool use) {
    objectEdgeDetector->setUseNormalAveraging(use);
}

void ObjectEdgeDetector::setUseGaussSmoothing(bool use) {
    objectEdgeDetector->setUseGaussSmoothing(use);
}

const Img8u &ObjectEdgeDetector::calculate(const Img32f &depthImage,
                    bool filter, bool average, bool gauss) {
    return objectEdgeDetector->calculate(depthImage, filter, average, gauss);
}

} // namespace geom
}
