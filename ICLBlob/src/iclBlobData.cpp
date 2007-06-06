#include <iclBlobData.h>
#include <iclRegionDetectorBlob.h>
#include <iclMacros.h>

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

    
 


}
