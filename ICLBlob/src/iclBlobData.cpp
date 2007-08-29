#include <iclBlobData.h>
#include <iclRegionDetectorBlob.h>
#include <iclMacros.h>
#include <iclImgChannel.h>

using namespace icl::regiondetector;
using namespace icl;
using namespace std;
namespace icl{
  const PCAInfo PCAInfo::null;
  const ScanLine ScanLine::null;
  

  BlobData::BlobData(RegionDetectorBlob *poRDB, const Size &oImageSize):
    m_poRDB(poRDB),m_oImageSize(oImageSize){
    FUNCTION_LOG("");
  }

  BlobData::~BlobData(){}

  int BlobData::getSize() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,0);
    return m_poRDB->getSize();
  }

  const std::vector<Point> &BlobData::getBoundary() const {
    FUNCTION_LOG("");
    static const std::vector<Point> v;
    ICLASSERT_RETURN_VAL(m_poRDB,v);
    return m_poRDB->getBoundary(m_oImageSize);
  }
  
  
  int BlobData::getBoundaryLength() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,0);
    return m_poRDB->getBoundaryLength(m_oImageSize);
  }
  float BlobData::getFormFactor() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,0);
    return m_poRDB->getFormFactor(m_oImageSize);
  }

  icl8u BlobData::getVal() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,0);
    return m_poRDB->getVal();
  }
  
  Point BlobData::getCOG() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,Point::null);
    return m_poRDB->getCOG();
  }
  
  Point32f BlobData::getCOGFloat() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,Point32f::null);
    return m_poRDB->getCOGFloat();
  }
 
  Rect BlobData::getBoundingBox() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,Rect::null);
    return m_poRDB->getBoundingBox();
  }
  PCAInfo BlobData::getPCAInfo() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,PCAInfo::null);
    const vector<float> &pca =m_poRDB->getPCAInfo();
    const Point &cog = m_poRDB->getCOG();
    return PCAInfo(pca[0],pca[1],pca[2],pca[3],cog.x,cog.y);
  }
  
  namespace {
    template<class T>
    void draw_blobdata_to(const RegionDetectorBlob *b, Img<T> *image, T val){
      ScanLineList *sls = const_cast<ScanLineList*>(b->getScanLines());
      for(int ch=0;ch<image->getChannels();ch++){
        ImgChannel<T> c = pickChannel(image,ch);
        for(ScanLineList::iterator it = sls->begin(); it!= sls->end(); ++it){
          RegionDetectorScanLine &sl = **it;
          T *dst = &c(sl.getStart(),sl.y());
          std::fill( dst, dst+sl.size()+1, val);
        }
      }
    }
  }
  
  void BlobData::drawTo(ImgBase *image, icl64f val) const{
    ICLASSERT_RETURN(image);
    ICLASSERT_RETURN(image->getImageRect().contains(getBoundingBox()));
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: draw_blobdata_to<icl##D>(m_poRDB, image->asImg<icl##D>(),Cast<icl64f,icl##D>::cast(val)); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
      default: ICL_INVALID_DEPTH;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
    
 


}
