#include "ImgRegionDetector.h"
#include "RegionDetector.h"
#include "Img.h"


namespace icl{
  
  using namespace regiondetector;
  
  ImgRegionDetector::ImgRegionDetector(unsigned int minSize,unsigned int maxSize,icl8u minVal, icl8u maxVal){
    m_poRD = new RegionDetector(1,1, minSize, maxSize, minVal, maxVal);
    m_poImage8uBuffer = new Img8u;
  }
  
  void ImgRegionDetector::setRestrictions(int minSize, int maxSize,icl8u minVal, icl8u maxVal){
    m_poRD->setMinValue(minVal);
    m_poRD->setMaxValue(maxVal);
    m_poRD->setMinSize(minSize);
    m_poRD->setMaxSize(maxSize);
  }
  ImgRegionDetector::~ImgRegionDetector(){
    delete m_poRD;
    delete m_poImage8uBuffer;
  }
  
  BlobList *get_blob_list(ImgBase *image,RegionDetector *rd, Img8u *bufROI=0){
    if(image->getDepth() != depth8u || !(image->hasFullROI())){
      bufROI->setSize(image->getROISize());
      image->deepCopyROI(bufROI);
      return get_blob_list(bufROI,rd);
    }
    rd->setSize(image->getSize());
    return rd->find_blobs(image->asImg<icl8u>()->getData(0));
  }

  
  const Array<int> &ImgRegionDetector::detect(ImgBase *image){
    if(!image || image->getChannels() != 1){
      m_oCenterBuffer.clear();
      return m_oCenterBuffer;
    }
    BlobList *blobList = get_blob_list(image, m_poRD, m_poImage8uBuffer);

    m_oCenterBuffer.resize(blobList->size()*2);    
    Array<int>::iterator dst = m_oCenterBuffer.begin();
    Point p; // tmp point buffer
    icl8u v; // tmp value buffer
    for(BlobList::iterator it = blobList->begin(); it != blobList->end(); ++it){
      (*it)->getFeatures(p,v);
      *dst++ = p.x; 
      *dst++ = p.y; 
    }
    return m_oCenterBuffer;
  }
  
  void ImgRegionDetector::detect(ImgBase *image, Array<int> &centers, Array<icl8u> &values){
    if(!image || image->getChannels() != 1){
      return;
    }
    BlobList *blobList = get_blob_list(image, m_poRD, m_poImage8uBuffer);

    centers.resize(blobList->size()*2);
    values.resize(blobList->size());
    Array<int>::iterator dstC = centers.begin();
    Array<icl8u>::iterator dstV = values.begin();
    Point p; // tmp point buffer
    for(BlobList::iterator it = blobList->begin(); it != blobList->end(); ++it){
      (*it)->getFeatures(p,*dstV++);
      *dstC++ = p.x; 
      *dstC++ = p.y; 
    }
  }
    
  void ImgRegionDetector::detect(ImgBase *image, Array<int> &centers, Array<icl8u> &values,
                                 Array<int> &boundingBoxes, Array<icl32f> &pcaInfos){
      if(!image || image->getChannels() != 1){
      return;
    }
    BlobList *blobList = get_blob_list(image, m_poRD, m_poImage8uBuffer);

    
    int s = blobList->size();
    centers.resize(2*s);
    values.resize(s);
    boundingBoxes.resize(4*s);
    pcaInfos.resize(4*s);
    Array<int>::iterator dstC = centers.begin();
    Array<icl8u>::iterator dstV = values.begin();
    Array<int>::iterator dstBB = boundingBoxes.begin();
    Array<icl32f>::iterator dstPCA = pcaInfos.begin();

    
    Point p; // tmp point buffer
    Rect bb; // tmp bounding box bufffer
    Size size = image->getSize();
    
    for(BlobList::iterator it = blobList->begin(); it != blobList->end(); ++it){
      (*it)->getAllFeatures(size,p,*dstV++,bb,*dstPCA,*(dstPCA+1),*(dstPCA+2),*(dstPCA+3));
      *dstC++ = p.x; 
      *dstC++ = p.y; 
      *dstBB++ = bb.x;
      *dstBB++ = bb.y;
      *dstBB++ = bb.width;
      *dstBB++ = bb.height;
      dstPCA+=4;
    }
  }
  
}
