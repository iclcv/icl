#ifndef FOUND_BLOB_H
#define FOUND_BLOB_H

namespace icl{
  
  /// struct that holds all information for a found blob
  template<class BlobRatingType>
  class FoundBlob{
    
    public:
    FoundBlob():m_iX(0),m_iY(0),m_iID(-1),m_tRating(BlobRatingType(0)){}
    
    FoundBlob(int x, int y, int id, BlobRatingType rating):
      m_iX(x),m_iY(y),m_iID(0),m_tRating(rating){  }
    
    int x(){ return m_iX; }
    
    int y(){ return m_iY; }
    
    int id(){ return m_iID; } 
    
    BlobRatingType rating(){ return m_tRating; }    

    private:
    
    int m_iX, m_iY, m_iID;
    
    BlobRatingType  m_tRating;
  };
}

#endif
