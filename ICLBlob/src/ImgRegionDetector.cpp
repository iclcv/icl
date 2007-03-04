#include <ImgRegionDetector.h>
#include <RegionDetector.h>
#include <Img.h>


namespace icl{
  
  using namespace regiondetector;
  
  ImgRegionDetector::ImgRegionDetector(unsigned int minSize,unsigned int maxSize,icl8u minVal, icl8u maxVal){
    m_poRD = new RegionDetector(1,1, minSize, maxSize, minVal, maxVal);
    m_poImage8uBuffer = new Img8u;
  }
  ImgRegionDetector::ImgRegionDetector(const Range<unsigned int> &sizeRange, const Range<icl8u> &valueRange){
    m_poRD = new RegionDetector(1,1, sizeRange.minVal, sizeRange.maxVal, valueRange.minVal, valueRange.maxVal);
    m_poImage8uBuffer = new Img8u;
  }
  void ImgRegionDetector::setRestrictions(int minSize, int maxSize,icl8u minVal, icl8u maxVal){
    m_poRD->setMinValue(minVal);
    m_poRD->setMaxValue(maxVal);
    m_poRD->setMinSize(minSize);
    m_poRD->setMaxSize(maxSize);
  }
  void ImgRegionDetector::setRestrictions(const Range<unsigned int> &sizeRange, const Range<icl8u> &valueRange){
    m_poRD->setMinValue(valueRange.minVal);
    m_poRD->setMaxValue(valueRange.maxVal);
    m_poRD->setMinSize(sizeRange.minVal);
    m_poRD->setMaxSize(sizeRange.maxVal);
  }

  ImgRegionDetector::~ImgRegionDetector(){
    delete m_poRD;
    delete m_poImage8uBuffer;
  }
  
  BlobList *get_blob_list(ImgBase *image,RegionDetector *rd, Img8u *bufROI=0){
    if(image->getDepth() != depth8u || !(image->hasFullROI())){   //TODO_depth
      bufROI->setSize(image->getROISize());
      image->convertROI(bufROI);
      return get_blob_list(bufROI,rd);
    }
    rd->setSize(image->getSize());
    return rd->find_blobs(image->asImg<icl8u>()->getData(0));
  }

  const std::vector<BlobData> &ImgRegionDetector::detect(ImgBase *image){
    m_vecBlobData.clear();
    if(!image || image->getChannels() != 1){
      return m_vecBlobData;
    }
    BlobList *blobList = get_blob_list(image, m_poRD, m_poImage8uBuffer);
    for(BlobList::iterator it = blobList->begin(); it != blobList->end(); ++it){
      m_vecBlobData.push_back(BlobData(*it,image->getSize()));
    }
    return m_vecBlobData;
  }
  
}
