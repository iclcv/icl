#include "PositionTracker.h"
#include "Extrapolator.h"
#include "HungarianAlgorithm.h"
#include <math.h>

/**                             new data
                         | o   o  x(t)  o   o
-------------------------+-----------------------
   o      o     o      o | d   d   d    d   d      
                         |
   o      o     o      o | d   ...                 
                         |
x(t-3) x(t-2) x(t-1) ŷ(t)|                         
                         |
   o      o     o      o |
                         |
   o      o     o      o |
                         |
Data[0] Data[1] ...      |  Dist[0], Dist[1], ...

         Data                    Dist


                      /\
                   vecPredict
*/
namespace icl{
  using namespace std;

  template<class valueType>
  void removeRowsFromDataMatrix(deque<vector<valueType> > m[2], std::vector<int> &rows){
    std::sort(rows.begin(),rows.end());
    for(int d=0; d <= 1; d++){
      deque<vector<valueType> > &md = m[d];
      for(int x=0;x<3;x++){
        vector<valueType> &v = md[x];
        vector<valueType> newV(v.size()-rows.size());
        for(int r=0,vidx=0,newvidx=0;vidx<(int)newV.size();vidx++){
          if(rows[r]!=vidx){
            newV[newvidx++] = v[vidx];
          }else{
            r++;
          }
        }
        v=newV;
      }
    }
  }
  
  template<class valueType>
  void PositionTracker<valueType>::pushData(const Vec &dataXs, const Vec &dataYs){
    const int X = 0;
    const int Y = 1;
    ICLASSERT_RETURN( dataXs.size() > 0 );
    ICLASSERT_RETURN( dataXs.size() == dataYs.size() );
    if(!m_matData[X].size()){ // first step
      for(int i=0;i<3;i++){
        m_matData[X].push_back(dataXs);
        m_matData[Y].push_back(dataYs);
      }
    }
    static const valueType BLIND_VALUE = valueType(1000000000);
    const int DATA_MATRIX_HEIGHT = (int)(m_matData[X][0].size());
    const int NEW_DATA_DIMENSION = (int)(dataXs.size());

    printf("DATA_MATRIX_HEIGHT = %d  NEW_DATA_DIMENSION = %d \n",DATA_MATRIX_HEIGHT, NEW_DATA_DIMENSION);
    const int DIFF = DATA_MATRIX_HEIGHT - NEW_DATA_DIMENSION;

    // create a working copy of the new data
    Vec newData[2] = {dataXs, dataYs};
    
    if(DIFF > 0){
      // new data contains less points -> enlarge new new data
      for(int i=0;i<DIFF;i++){
        newData[X].push_back(BLIND_VALUE);
        newData[Y].push_back(BLIND_VALUE);
      }
    }
    const int DATA_AND_MATRIX_DIM = (int)(newData[X].size());
    printf("DIFF = %d   DATA_AND_MATRIX_DIM = %d \n",DIFF,DATA_AND_MATRIX_DIM);
    
    Vec vecPrediction[2];
    for(int y=0 ; y < DATA_AND_MATRIX_DIM; ++y){
      if(y>DATA_AND_MATRIX_DIM+DIFF){    // no data matrix elements available
        vecPrediction[X].push_back(BLIND_VALUE);
        vecPrediction[Y].push_back(BLIND_VALUE);
      }else{
        vecPrediction[X].push_back( Extrapolator<valueType,int>::predict( m_matData[X][0][y], m_matData[X][1][y], m_matData[X][2][y] ) );
        vecPrediction[Y].push_back( Extrapolator<valueType,int>::predict( m_matData[Y][0][y], m_matData[Y][1][y], m_matData[Y][2][y] ) );
      }
    }

    SimpleMatrix<valueType> distMat(DATA_AND_MATRIX_DIM,DATA_AND_MATRIX_DIM);
    for(int x=0;x<DATA_AND_MATRIX_DIM;++x){
      for(int y=0;y<DATA_AND_MATRIX_DIM;++y){
        distMat[x][y] = (valueType)sqrt (pow( vecPrediction[X][y] - newData[X][x], 2) + pow( vecPrediction[Y][y] - newData[Y][x], 2) );
      }
    }
    
    m_vecCurrentAssignement = HungarianAlgorithm<valueType>::apply(distMat);
    
    HungarianAlgorithm<valueType>::visualizeAssignment(distMat,m_vecCurrentAssignement);

    // add the new data column -> by assingned data elements
    if(DIFF > 0){
      // new data contains less points; points are lost  
      // -> the DIFF assigned point with highes error are removed from the data matrix 
      // rows with mark in the last DIFF colums must be removed from the data matrix
      vector<int> delRows;
      for(int i=0;i<DIFF;i++){
        delRows.push_back(m_vecCurrentAssignement[DATA_AND_MATRIX_DIM-1-i]);
      }
      removeRowsFromDataMatrix(m_matData,delRows);
    }else if(DIFF < 0){
      // new data contains more points
      // push new rows to the data matrix with the new points  
      // new data points have assingment in the lower -DIFF rows
      
      vector<int> newDataCols;
      for(int i=0;i<DATA_AND_MATRIX_DIM;i++){
        if(m_vecCurrentAssignement[i] > DATA_AND_MATRIX_DIM+DIFF){
          newDataCols.push_back(i);
        }
      }
      ICLASSERT( (int)newDataCols.size() == -DIFF );
      for(int i=0;i<(-DIFF);i++){
        for(int j=0;j<3;j++){
          m_matData[X][j].push_back( newData[X][newDataCols[i]] );
          m_matData[Y][j].push_back( newData[Y][newDataCols[i]] );
        }
      }
    }
    
    /// rearrange the new data arrays
    vector<valueType> rearrangedData[2] = { vector<valueType>(DATA_AND_MATRIX_DIM), vector<valueType>(DATA_AND_MATRIX_DIM)} ;
    for(int i=0;i < DATA_AND_MATRIX_DIM ;i++){
      rearrangedData[X][i] = newData[X][ m_vecCurrentAssignement[i] ];
      // was ist mit den besonderen zeilen/spalten
    }
    
    m_matData[X].push_back( rearrangedData[X] );
    m_matData[Y].push_back( rearrangedData[Y] );
    
    m_matData[X].pop_front();
    m_matData[Y].pop_front();
    
    printf(" association: \n");
    for(unsigned int i=0;i<m_vecCurrentAssignement.size();i++){
      printf(" %d --> %d \n",i,m_vecCurrentAssignement[i]);
    }
    printf("\n");
  }
  
  template<class valueType>
  int PositionTracker<valueType>::getID(int currIdx){
    return m_vecCurrentAssignement[currIdx];    
  }

  template class  PositionTracker<icl32s>;
  // template class  PositionTracker<icl32f>;
  //template class  PositionTracker<icl64f>;
  
}
