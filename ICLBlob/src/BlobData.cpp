#include <BlobData.h>
#include <RegionDetectorBlob.h>
#include <Macros.h>

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

  int BlobData::getSize() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,0);
    return m_poRDB->size();
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
    return m_poRDB->val();
  }
  
  Point BlobData::getCenter() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,Point::null);
    icl8u val;
    Point c;
    m_poRDB->getFeatures(c,val);
    return c;
  }
 
  Rect BlobData::getBoundingBox() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,Rect::null);

    Point c;
    icl8u v;
    Rect bb;
    float l1,l2,a1,a2;
    m_poRDB->getAllFeatures(m_oImageSize,c,v,bb,l1,l2,a1,a2);
    return bb;
  }
  PCAInfo BlobData::getPCAInfo() const{
    FUNCTION_LOG("");
    ICLASSERT_RETURN_VAL(m_poRDB,PCAInfo::null);
    Point c;
    icl8u v;
    Rect bb;
    float l1,l2,a1,a2;
    m_poRDB->getAllFeatures(m_oImageSize,c,v,bb,l1,l2,a1,a2);
    return PCAInfo(l1,l2,a1,a2);
  }
    
 


}
