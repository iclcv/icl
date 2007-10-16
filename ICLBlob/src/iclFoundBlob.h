#ifndef FOUND_BLOB_H
#define FOUND_BLOB_H

namespace icl{
  
  /// struct that holds all information for a found blob of a color blob searcher \ingroup G_CBS
  template<class BlobRatingType>
  class FoundBlob{
    
    public:
    /// Empty contructor initialized all data with 0
    FoundBlob():m_iX(0),m_iY(0),m_iID(-1),m_tRating(BlobRatingType(0)){}
    
    /// Default constructor given location, id and rating
    FoundBlob(int x, int y, int id, BlobRatingType rating):
      m_iX(x),m_iY(y),m_iID(id),m_tRating(rating){  }
    
    /// x location
    int x(){ return m_iX; }
    
    /// y location
    int y(){ return m_iY; }
    
    /// id
    int id(){ return m_iID; } 
    
    /// rating
    BlobRatingType rating(){ return m_tRating; }    

    private:
    
    /// internal storage of the x location
    int m_iX;

    /// internal storage of the y location
    int m_iY;
    
    /// internal storage of the blobs id
    int m_iID;
    
    /// internal storage of the blobs rating
    BlobRatingType  m_tRating;
  };
}

#endif
