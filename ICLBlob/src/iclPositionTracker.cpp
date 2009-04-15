#include "iclPositionTracker.h"
#include "iclExtrapolator.h"
#include "iclHungarianAlgorithm.h"
#include <math.h>
#include <set>
#include <limits>

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
  namespace positiontracker{
    const int X = 0;
    const int Y = 1;
    const int BLIND_VALUE = 9999;
  }
  using namespace positiontracker;

#define SAY(X)

  void show_vec(const std::vector<int> &v, const std::string &s){
    // {{{ open

    printf("%s[%d][ ",s.c_str(),v.size());
    for(unsigned int i=0;i<v.size();i++){
      printf("%d ",v[i]);
    }
    printf("]\n");
  }

  // }}}
  
  void showDataMatrix(deque<vector<int> > m[2]){
    // {{{ open

    int DIM = m[0][0].size();
    for(int d=0;d<DIM;d++){
      for(int i=0;i<3;i++){
        printf("(%3d,%3d) ",m[0][i][d],m[1][i][d]);
      }
      printf("\n");
    }
    printf("\n");
  }

  // }}}


  template<class valueType>
  void removeElemsFromVector(vector<valueType> &v,const  std::vector<int> &rows){
    // {{{ open
    vector<valueType> newV;//(v.size()-rows.size());
    int r=0,vidx=0;
    for(; vidx<(int)(v.size()) && r<(int)rows.size() ; vidx++){
      if(rows[r]!=vidx){
        newV.push_back(v[vidx]);// [newvidx++] = v[vidx];
      }else{
        r++;
      }
    }
    for(;vidx<(int)v.size();vidx++){
      newV.push_back(v[vidx]);
    }
    v=newV;
   }

  // }}}
  
  template<class valueType>
  void removeRowsFromDataMatrix(deque<vector<valueType> > *m,const std::vector<int> &rows){
    // {{{ open

    /// rows must be sorted !
    for(int d=0; d <= 1; d++){
      deque<vector<valueType> > &md = m[d];
      for(int x=0;x<3;x++){ 
        vector<valueType> &v = md[x];
        removeElemsFromVector(v,rows);
        /*
        vector<valueType> newV(v.size()-rows.size());
        for(int r=0,vidx=0,newvidx=0;vidx<(int)v.size();vidx++){
        if(rows[r]!=vidx){
        newV[newvidx++] = v[vidx];
        }else{
        r++;
        }
        }
        v=newV;
        */
      }
    }
  }

  // }}}
  

   
  template<class valueType>
  void PositionTracker<valueType>::pushData(valueType *xys, int n){
    // {{{ open

    Vec x(n),y(n);
    for(int i=0;i<n;i++){
      x[i] = xys[2*i];
      y[i] = xys[2*i+1];
    }
    pushData(x,y);
  }

  // }}}


  template<class valueType>
  SimpleMatrix<valueType> createDistMat(const vector<valueType> a[2], const vector<valueType> b[2]){
    // {{{ open

    ICLASSERT( a[X].size() == b[X].size() );
    // DEBUG if( a[X].size() != b[X].size() ) printf("error: %d != %d \n",a[X].size(),b[X].size());
    int dim = (int)a[X].size();
    SimpleMatrix<valueType> m(dim);
    for(int i=0;i<dim;++i){
      for(int j=0;j<dim;++j){
        double dx = a[X][j] - b[X][i];
        double dy = a[Y][j] - b[Y][i];
        // !! use sqrt of the euclidian distance to stabilize the association procedure (..)
        m[i][j] = (int)round (sqrt(sqrt( dx*dx + dy*dy )) );
        // m[i][j] = (valueType)sqrt(sqrt (pow((double) (a[X][j] - b[X][i]), 2) +
        // pow((double) (a[Y][j] - b[Y][i]), 2) ));
      }
    }
    return m;
  }

  // }}}
  
  template<class valueType>
  inline vector<valueType> predict(int dim, deque<vector<valueType> > &data, const vector<int> &good){
    // {{{ open
    vector<valueType> pred;
    for(int y=0 ; y < dim; ++y){
      //if(y>=(int)good.size()) printf("!!!!!!!!!!!!!! waring dim=%d good.size = %d \n",dim,good.size());
      switch(good[y]){
        case 1: pred.push_back( data[2][y] ); break;
        case 2: pred.push_back( Extrapolator<valueType,int>::predict( data[1][y], data[2][y]) ); break;
        default: pred.push_back( Extrapolator<valueType,int>::predict( data[0][y], data[1][y], data[2][y]) ); break;
      }
    }
      // ok old pred.push_back( Extrapolator<valueType,int>::predict( data[0][y], data[1][y], data[2][y] ) );
    return pred;
  }

  // }}}

  
  inline vector<int> get_n_new_ids(const vector<int> &currentIDS, int n, icl::IDAllocationMode iaMode){
   // {{{ open 
    vector<int> ids;
    set<int> lut;
    switch(iaMode){
      case allocateFirstFreeIDs:
        for(unsigned int i=0;i<currentIDS.size();i++){
          lut.insert( currentIDS[i] );
        }
        for(int i=0,id=0;i<n;i++){
          while(lut.find(id) != lut.end()){
            id++;
          }
          ids.push_back(id);
          lut.insert(id);      
        }
        return ids;
      case allocateBrandNewIDs:{
        for(int i=0;i<n;i++){
          ERROR_LOG("this is not implemented correctly !!");
          //          ids.push_back(m_currentID);
          //++m_currentID;
        }
        return ids;
      }
      default:
        ERROR_LOG("unsupported IDAllocationmode");
        return ids;
    }
  }

  // }}}
  
  template<class valueType>
  void push_and_rearrange_data(int                       dim, 
                               deque<vector<valueType> > data[2],
                               const vector<int>         &assignment,
                               const vector<valueType>   newData[2]){
    // {{{ open
    vector<valueType> arrangedData[2] = {vector<valueType>(dim),vector<valueType>(dim)};
    for(int i=0;i<dim;i++){
      arrangedData[X][ assignment[i] ] = newData[X][i]; 
      arrangedData[Y][ assignment[i] ] = newData[Y][i]; 
    }
    data[X].push_back(arrangedData[X]);
    data[Y].push_back(arrangedData[Y]);
    
    data[X].pop_front();
    data[Y].pop_front();
  }
  // }}}

  template<class valueType>
  void push_data_intern_diff_zero(int dim,
                                  deque<vector<valueType> > data[2], 
                                  vector<int>               &assignment,  
                                  vector<valueType>         newData[2],
                                  vector<int>               &good){
    
    // {{{ open

    vector<valueType> pred[2] = { predict(dim,data[X],good), predict(dim,data[Y],good) };
    
    SimpleMatrix<valueType> distMat = createDistMat( pred , newData );
    
    assignment = HungarianAlgorithm<valueType>::apply(distMat);
    
    push_and_rearrange_data(dim, data, assignment, newData);
    
    for(unsigned int i=0;i<good.size();++i){
      good[i]++;
    }
    
  }

  // }}}

  template<class valueType>
  inline valueType distance(valueType x1, valueType y1, valueType x2, valueType y2){
    return (valueType) sqrt( pow((double)(x1-x2),2) +  pow((double)(y1-y2),2) );
  }

  template<class valueType>
  bool push_data_first_optimized_try(int dim,
                                     deque<vector<valueType> > data[2], 
                                     vector<int>               &assignment,  
                                     vector<valueType>         newData[2],
                                     vector<int>               &good,
                                     valueType threshold){
    
    // {{{ open

    vector<valueType> xsOld(dim);
    vector<valueType> ysOld(dim);
    
    for(int y=0 ; y < dim; ++y){
      xsOld[y] = data[0][2][y];
      ysOld[y] = data[1][2][y];
    }
    
    vector<valueType> &xsNew = newData[0];
    vector<valueType> &ysNew = newData[1];

    vector<bool> assigned(dim,false);

    vector<int> newAssignment(dim,-1);
    //    printf("==================== step ============\n");
    //printf("optimzation ... for %d centers\n",dim);
    
    for(int i=0;i<dim;i++){
      //printf("processing step old data index %d \n",i);
      // find best hit for nr i
      valueType minDist = numeric_limits<valueType>::max();
      int minIdx = -1;
      for(int j=0;j<dim;j++){
        valueType dist = distance(xsOld[i],ysOld[i],xsNew[j],ysNew[j]);
        if(dist < minDist){
          //  printf("found new data %d was only %d away \n",j,dist);
          minDist = dist;
          minIdx = j;
        }
      }
      // printf("ok, index %d was best for old index
      // %d distance is %d thresh is %d \n",
      // minIdx,i,minDist, threshold);
      if(minDist < threshold && !assigned[minIdx]){
        assigned[minIdx] = true;
        newAssignment[minIdx] = i;
      }else{
        return false;
      }
    }

    assignment = newAssignment;

    push_and_rearrange_data(dim, data, assignment, newData);
    
    for(unsigned int i=0;i<good.size();++i){
      good[i]++;
    }

    return true;
    
    
  }
  // }}}
 

  template<class valueType>
  void push_data_intern_diff_gtz(int DIFF, 
                                 deque<vector<valueType> > data[2], 
                                 vector<int>               &ids, 
                                 vector<int>               &assignment,  
                                 vector<valueType>         newData[2],
                                 vector<int>               &good){
    // {{{ open

    // new data contains less points -> enlarge new new data
    for(int i=0;i<DIFF;i++){
      newData[X].push_back(BLIND_VALUE);
      newData[Y].push_back(BLIND_VALUE);
    }
    int dim = data[X][0].size();
    vector<valueType> pred[2] = { predict(dim,data[X],good), predict(dim,data[Y],good) };
    
    SimpleMatrix<valueType> distMat = createDistMat( pred , newData );
    
    assignment = HungarianAlgorithm<valueType>::apply(distMat);

    push_and_rearrange_data(dim, data, assignment, newData);
    
    vector<int> delRows;
    for(int i=0;i<DIFF;i++){
      delRows.push_back( assignment [dim-1-i]);
    }
    std::sort(delRows.begin(),delRows.end());
    removeRowsFromDataMatrix(data,delRows);
    
    removeElemsFromVector(ids, delRows);

    removeElemsFromVector(good, delRows);
    
    for(unsigned int i=0;i<good.size();++i){
      good[i]++;
    }

  } 

  // }}}

  template<class valueType>
  void push_data_intern_diff_ltz(int DIFF,
                                 deque<vector<valueType> > data[2], 
                                 vector<int>               &ids, 
                                 vector<int>               &assignment,  
                                 vector<valueType>         newData[2],
                                 vector<int>               &good,
                                 icl::IDAllocationMode iaMode){
    // {{{ open

    DIFF *= -1; // now positive
    for(int j=0;j<DIFF;j++){
      for(int i=0;i<3;i++){
        data[X][i].push_back(BLIND_VALUE);
        data[Y][i].push_back(BLIND_VALUE);
      }
    }  

    int dim = data[X][0].size();
    /// good vector is temporarily filled with ones for prediction
    for(int i=0;i<DIFF;i++){
      good.push_back(1);
    }
    vector<valueType> pred[2] = { predict(dim,data[X],good), predict(dim,data[Y],good) };
    /// restore good
    good.resize(good.size()-DIFF);
    
    SimpleMatrix<valueType> distMat = createDistMat( pred , newData );
    
    assignment = HungarianAlgorithm<valueType>::apply(distMat);
    
    /// <old>
    //vector<int> newDataCols;
    vector<valueType> newDataColsValues[2];
    for(int x=0;x<dim;++x){ // x is the col index of newData
      if(assignment[x] >= dim-DIFF){
        newDataColsValues[X].push_back(newData[X][ assignment[x] ]);
        newDataColsValues[Y].push_back(newData[Y][ assignment[x] ]);
      }
    }
    if((int)newDataColsValues[X].size() != DIFF){
      printf("WARNING: newDataColsValues[X].size()[%d] is != DIFF[%d]",(int)newDataColsValues[X].size(),DIFF);
    }
        
    vector<int> newIDS = get_n_new_ids(ids,DIFF,iaMode);
    
    for(int i=0;i<DIFF;i++){
      for(int j=0;j<3;j++){
        data[X][j][dim-DIFF+i] = newDataColsValues[X][i]; //newData[X][newDataCols[i]];
        data[Y][j][dim-DIFF+i] = newDataColsValues[Y][i]; //newData[Y][newDataCols[i]];
      }
      ids.push_back( newIDS[i] );
      good.push_back( 0 );
    }
    
    for(unsigned int i=0;i<good.size();++i){
      good[i]++;
    }    

    push_and_rearrange_data(dim, data, assignment, newData);

  }

  // }}}

  template<class valueType>
  void push_data_intern_first_step(deque<vector<valueType> > data[2], 
                                   vector<int>               &ids, 
                                   vector<int>               &assignment,  
                                   vector<valueType>         newData[2],
                                   vector<int>               &good){
    // {{{ open

    for(int i=0;i<3;i++){
      data[X].push_back(newData[X]);
      data[Y].push_back(newData[Y]);
    }
    ids.resize(newData[X].size());
    for(unsigned int i=0;i<newData[X].size();i++){
      ids[i]=i;
      good.push_back(1);
    }
  }

  // }}}
  

 
  
  template<class valueType>
  void PositionTracker<valueType>::pushData(const Vec &dataXs, const Vec &dataYs){
    // {{{ open

    ICLASSERT_RETURN( dataXs.size() > 0 );
    ICLASSERT_RETURN( dataXs.size() == dataYs.size() );

    Vec newData[2] = {dataXs, dataYs};
    if(!m_matData[X].size()){
      push_data_intern_first_step(m_matData, m_vecIDs, m_vecCurrentAssignment, newData, m_vecGoodDataCount);
      return;
    }
    const int DATA_MATRIX_HEIGHT = (int)(m_matData[X][0].size());
    const int NEW_DATA_DIMENSION = (int)(dataXs.size());
    const int DIFF = DATA_MATRIX_HEIGHT - NEW_DATA_DIMENSION;

    if(DIFF <  0){
      push_data_intern_diff_ltz(DIFF,m_matData, m_vecIDs, m_vecCurrentAssignment, newData,m_vecGoodDataCount, m_IDAllocationMode);
    }else if(DIFF > 0){
       push_data_intern_diff_gtz(DIFF,m_matData, m_vecIDs, m_vecCurrentAssignment, newData,m_vecGoodDataCount);
    }else{
       if(m_bTryOptimize && m_tThreshold > 0){
        bool succ = push_data_first_optimized_try(DATA_MATRIX_HEIGHT,m_matData,m_vecCurrentAssignment,newData,m_vecGoodDataCount,m_tThreshold);
        if(!succ){
          push_data_intern_diff_zero(DATA_MATRIX_HEIGHT,m_matData, m_vecCurrentAssignment, newData,m_vecGoodDataCount);
        }        
      }else{
        push_data_intern_diff_zero(DATA_MATRIX_HEIGHT,m_matData, m_vecCurrentAssignment, newData,m_vecGoodDataCount);
      }
    }
  }

  // }}}
    
  // {{{ old code

  /*
    if(!m_matData[X].size()){ // first step
    for(int i=0;i<3;i++){
    m_matData[X].push_back(dataXs);
    m_matData[Y].push_back(dataYs);
    }
    }
    m_matData[X][0].size(),m_matData[X][1].size(),m_matData[X][2].size(),
    m_matData[Y][0].size(),m_matData[Y][1].size(),m_matData[Y][2].size());
    //static const valueType BLIND_VALUE = valueType(999);
    //const int DATA_MATRIX_HEIGHT = (int)(m_matData[X][0].size());
    //const int NEW_DATA_DIMENSION = (int)(dataXs.size());
    //const int DIFF = DATA_MATRIX_HEIGHT - NEW_DATA_DIMENSION;
    // create a working copy of the new data
    Vec newData[2] = {dataXs, dataYs};
     if(DIFF > 0){ // redundant
     // new data contains less points -> enlarge new new data
    for(int i=0;i<DIFF;i++){
    newData[X].push_back(BLIND_VALUE);
    newData[Y].push_back(BLIND_VALUE);
    }
    }
    if(DIFF < 0){ // todo optimize later
    printf("part 5.a \n");
    // push lines to the data matrix
    for(int j=0;j<-DIFF;j++){
    for(int i=0;i<3;i++){
    m_matData[X][i].push_back(BLIND_VALUE);
    m_matData[Y][i].push_back(BLIND_VALUE);
    }
    }      
    }
    const int DATA_AND_MATRIX_DIM = (int)(newData[X].size());
    Vec vecPrediction[2];
    m_matData[X][0].size(),m_matData[X][1].size(),m_matData[X][2].size(),
    m_matData[Y][0].size(),m_matData[Y][1].size(),m_matData[Y][2].size());
    for(int y=0 ; y < DATA_AND_MATRIX_DIM; ++y){
    vecPrediction[X].push_back( Extrapolator<valueType,int>::predict( m_matData[X][0][y], m_matData[X][1][y], m_matData[X][2][y] ) );
    vecPrediction[Y].push_back( Extrapolator<valueType,int>::predict( m_matData[Y][0][y], m_matData[Y][1][y], m_matData[Y][2][y] ) );
    }
    SimpleMatrix<valueType> distMat(DATA_AND_MATRIX_DIM,DATA_AND_MATRIX_DIM);
    for(int x=0;x<DATA_AND_MATRIX_DIM;++x){
    for(int y=0;y<DATA_AND_MATRIX_DIM;++y){
    distMat[x][y] = (valueType)sqrt (pow( vecPrediction[X][y] - newData[X][x], 2) + pow( vecPrediction[Y][y] - newData[Y][x], 2) );
    }
    }
    m_vecCurrentAssignement = HungarianAlgorithm<valueType>::apply(distMat);
      // add the new data column -> by assingned data elements
    if(DIFF > 0){
    vector<int> delRows;
    for(int i=0;i<DIFF;i++){
    delRows.push_back(m_vecCurrentAssignement[DATA_AND_MATRIX_DIM-1-i]);
    }
    removeRowsFromDataMatrix(m_matData,delRows);
    newData[X].resize(newData[X].size()-DIFF);
    newData[X].resize(newData[Y].size()-DIFF);
    }else if(DIFF < 0){
    vector<int> newDataCols;
    for(int i=0;i<DATA_AND_MATRIX_DIM;i++){
    if(m_vecCurrentAssignement[i] >= DATA_AND_MATRIX_DIM+DIFF){
    newDataCols.push_back(i); //ERROR not i but assignment[i] !!
    }
    }
    ICLASSERT( (int)newDataCols.size() == -DIFF );
    for(int i=0;i<(-DIFF);i++){
    for(int j=0;j<3;j++){
    m_matData[X][j][DATA_AND_MATRIX_DIM+DIFF-i] = newData[X][newDataCols[i]];
    m_matData[Y][j][DATA_AND_MATRIX_DIM+DIFF-i] = newData[Y][newDataCols[i]];
    }
    }
    }
    
    /// rearrange the new data arrays
    vector<valueType> rearrangedData[2] = { vector<valueType>(DATA_AND_MATRIX_DIM), vector<valueType>(DATA_AND_MATRIX_DIM)} ;
    for(int i=0;i < DATA_AND_MATRIX_DIM ;i++){
    rearrangedData[X][i] = newData[X][ m_vecCurrentAssignement[i] ];
    rearrangedData[Y][i] = newData[Y][ m_vecCurrentAssignement[i] ];

    }
      
    if(DIFF > 0){
    rearrangedData[X].resize(DATA_AND_MATRIX_DIM-DIFF);
    rearrangedData[Y].resize(DATA_AND_MATRIX_DIM-DIFF);
    }
  
    m_matData[X].push_back( rearrangedData[X] );
    m_matData[Y].push_back( rearrangedData[Y] );
    
    m_matData[X].pop_front();
    m_matData[Y].pop_front();
    */

  // }}}

                            
  template<class valueType>
  int PositionTracker<valueType>::getID(valueType x, valueType y){
    // {{{ open

    vector<valueType> &rX = m_matData[X][2];
    vector<valueType> &rY = m_matData[Y][2];
    for(unsigned int i=0;i<rX.size();++i){
      if(rX[i] == x && rY[i] == y){
        return m_vecIDs[i];
      }
    }
    return -1;
  }

  // }}}

  template<class valueType>
  int PositionTracker<valueType>::getID(int index){
    // {{{ open
    if(index >= 0 && index < (int)m_vecCurrentAssignment.size()){
      return m_vecIDs[ m_vecCurrentAssignment[index] ];
    }else{
      return -1;
    }
  }

  // }}}
  
  template class  PositionTracker<icl32s>;
  template class  PositionTracker<icl32f>;
  //template class  PositionTracker<icl64f>;

}
