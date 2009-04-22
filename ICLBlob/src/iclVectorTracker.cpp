#include "iclVectorTracker.h"
#include "iclExtrapolator.h"
#include "iclHungarianAlgorithm.h"
#include <iclMathematics.h>
#include <iclException.h>
#include <iclDynMatrix.h>
#include <set>
#include <limits>

namespace icl{
  typedef VectorTracker::Vec Vec;
  
  static inline float square(const float &a){ return a*a; }

  static float eucl_dist(const Vec &a, const Vec &b){
    float sum  = 0;
    for(int i=0;i<a.size();++i){
      sum += square(a[i]-b[i]);
    }
    return ::sqrt(sum);
  }
  static float sqrt_eucl_dist(const Vec &a, const Vec &b){
    return ::sqrt(eucl_dist(a,b));
  }
  
  struct PearsonDist{
    const std::vector<float> &v;
    PearsonDist(const std::vector<float> &normFactors):v(normFactors){ }
    float operator()(const Vec &a, const Vec &b) const{
      float sum  = 0;
      for(int i=0;i<a.size();++i){
        sum += square((a[i]-b[i])/v[i]);
      }
      return ::sqrt(sum);
    }
  };

  struct VTMat{
    int dim;
    int height;
    std::vector<Vec> *cols[4]; // t-3,t-2,t-1, pred
    std::vector<int> goodV;
    std::vector<int> ids; 
   
    VTMat(int dim):dim(dim),height(0){
      for(int c=0;c<4;++c){
        cols[c]=new std::vector<Vec>();
      }
    }
    ~VTMat(){
      for(int c=0;c<4;++c){
        delete cols[c];
      }
    }
    VTMat(const VTMat &other):dim(other.dim),height(other.height),goodV(other.goodV),ids(other.ids){
      for(int c=0;c<4;++c){
        cols[c]=new std::vector<Vec>(*other.cols[c]);
      }
    }

    VTMat &operator=(const VTMat &other){
      dim = other.dim;
      height = other.height;
      goodV = other.goodV;
      ids = other.ids;
      for(int c=0;c<4;++c){
        delete cols[c];
        cols[c]=new std::vector<Vec>(*other.cols[c]);
      }
      return *this;
    }
    
    
    
    Vec &pred(int y){ return (*cols[3])[y]; }
    
    int h() const { return height; }
    
    void reset(){
      height = 0;
      goodV.clear();
      ids.clear();
      for(int c=0;c<4;++c){
        cols[c]->clear();
      }    
    }
    static Vec predict_vec(const Vec &a, const Vec &b){
      Vec p(a.size());
      for(unsigned int i=0;i<a.size();++i){
        p[i] = Extrapolator<float,int>::predict(a[i],b[i]);
      }
      return p;
    }
    static Vec predict_vec(const Vec &a, const Vec &b, const Vec &c){
      Vec p(a.size());
      for(unsigned int i=0;i<a.size();++i){
        p[i] = Extrapolator<float,int>::predict(a[i],b[i],c[i]);
      }
      return p;
    }

    
    void predict(){
      for(int y=0;y<height;++y){
        switch(goodV[y]){
          case 0:
            ERROR_LOG("this shell not happen!");
            break;
          case 1: 
            (*cols[3])[y] = (*cols[2])[y]; 
            break;
          case 2: 
            (*cols[3])[y] = predict_vec( (*cols[1])[y], (*cols[2])[y] );
            break;
          default: 
            (*cols[3])[y] = predict_vec( (*cols[0])[y], (*cols[1])[y], (*cols[2])[y] );
        }
      }
    }

    static void showVec(const std::vector<Vec> &a, const std::string &title="just a vec"){
      std::cout << "std::vector<Vec>:" << title << ":"<<std::endl;
      if(!a.size()){
        std::cout << "null-sized" << std::endl;
        return;
      }
      for(unsigned int y=0;y<a[0].size();++y){
        for(unsigned int x=0;x<a.size();++x){
          printf("%4.3f  ",a[x][y]);
        }
        printf("\n");
      }
    }
#define SHOW_VEC(x) showVec(x,#x)
    
    template<class DistanceFunc>
    SimpleMatrix<float> createDistanceMatrix(const std::vector<Vec> &newData, DistanceFunc dist_func, float largeVal){
      // DEBUG_LOG("creating distance matrix:");
      // SHOW_VEC(newData);
      // SHOW_VEC(*cols[3]);
      
      int w = (int)newData.size();
      int h = height;
      int maxWH = iclMax(w,h);
      SimpleMatrix<float> distMat(maxWH,maxWH);
      std::fill(distMat.data(),distMat.data()+distMat.dim(),largeVal);
      for(int x=0;x<newData.size();++x){
        for(int y=0;y<height;++y){
          distMat[x][y] = dist_func(newData[x],pred(y));
        }
      }
      return distMat;
    }
    
    virtual void notifyIDLoss(int id)=0;
    virtual void appendNewIDs(int n)=0;
    
    void addRows(int n){
      int newH = height+n;
      for(int c=0;c<4;++c){
        cols[c]->resize(newH,Vec(dim,0.0));
      }
      goodV.resize(newH,0);
      appendNewIDs(n);
      height = newH;
    }
    void removeRows(const std::vector<int> &idxs){
      int newH = height - (int)idxs.size();
      std::vector<Vec> *newCols[4];
      std::vector<int> newGoodV;
      std::vector<int> newIDs;
      newGoodV.reserve(newH);
      newIDs.reserve(newH);
      for(int c=0;c<4;++c){
        newCols[c]=new std::vector<Vec>;
        newCols[c]->reserve(newH);
      }
      std::vector<bool> hold(height,true);
      for(unsigned int i=0;i<idxs.size();++i){
        hold[ idxs[i] ] = false;
      }
      for(unsigned int y=0;y<height;++y){
        if(hold[y]){
          for(int c=0;c<4;++c){
            newCols[c]->push_back((*cols[c])[y]);
          }
          newGoodV.push_back(goodV[y]);
          newIDs.push_back(ids[y]);
        }else{
          notifyIDLoss(ids[y]);
        }
      }
      for(int c=0;c<4;++c){
        delete cols[c];
        cols[c] = newCols[c];
      }
      goodV = newGoodV;
      ids = newIDs;
      height = newH;
    }
    
    // newCol is already assigned
    void pushCol(const std::vector<Vec> &newCol){
      std::vector<Vec> *c0 = cols[0];
      cols[0]=cols[1];
      cols[1]=cols[2];
      cols[2]=c0;
      
      std::copy(newCol.begin(),newCol.end(),c0->begin());
      
      for(int y=0;y<height;++y){
        if(goodV[y]<3) goodV[y]++;
      }
    }
  };
  
  
  struct VectorTracker::Data : public VTMat{
    
    Data(int dim, bool tryOpt,VectorTracker::IDmode idMode, 
         float distanceThreshold, float largeVal, 
         const std::vector<float> &normFactors):
      VTMat(dim),nextID(0),tryOpt(tryOpt),idMode(idMode),
      thresh(distanceThreshold),largeVal(largeVal),
      normFactors(normFactors){
      
      bool all1 = true;
      for(int i=0;i<normFactors.size();++i){
        if(normFactors[i] != 1) all1 = true;
        if(normFactors[i] == 0){
          ERROR_LOG("detected normfactor[" << i << "]  == 0  (deactivating normfactors to avoid div0 - errors))");
          this->normFactors.clear();
          return;
        }
      }
      /// if normFactors are all equal to 1.0, we can just use default euclidian distance
      if(all1) this->normFactors.clear();
    }
  
    std::vector<int> ass;
    
    bool tryOpt;
    int nextID;
    VectorTracker::IDmode idMode;
    float thresh;
    float largeVal;
    std::vector<bool> idMask;
    std::vector<float> normFactors;
    
    virtual void notifyIDLoss(int id){
      // DEBUG_LOG("notify id loss: " << id << "(idsMask.size is " << idMask.size() << ")");
      if(idMode == VectorTracker::firstFree){
        idMask[id] = false;
      }
    }
    
    virtual void appendNewIDs(int n){
      switch(idMode){
        case VectorTracker::brandNew:
          for(int i=0;i<n;++i){
            ids.push_back(nextID++);
          }
          break;
        case VectorTracker::firstFree:{
          //DEBUG_LOG("########################### asking for " << n << " new ids");
          int idsLeft = n; 
          for(unsigned int i=0;idsLeft && i<idMask.size();++i){
            if(!idMask[i]){
              idMask[i] = true;
              ids.push_back(i);
              --idsLeft;
            }
          }
          for(unsigned int nextID=idMask.size();idsLeft;++nextID,--idsLeft){
            ids.push_back(nextID);
            idMask.push_back(true);
          }
          break;
        }
        default: 
          ERROR_LOG("unknown id allocation mode");
      }
    }
    void adaptAssignment(std::vector<int> lostRows,int oldNum, int newNum){
      if(!lostRows.size()) return;
      std::sort(lostRows.begin(),lostRows.end());

      unsigned int lostRowIdx = 0;
      int shift = 0;
      
      std::vector<int> shiftTab(oldNum);
      for(int y=0;y<oldNum;++y){
        if(lostRowIdx < lostRows.size() && lostRows[lostRowIdx] == y){
          shift++;
          shiftTab[y] = -1;
          lostRowIdx++;
        }else{
          shiftTab[y] = shift;
        }
      }

      for(int x=0;x<newNum;++x){
        shift = shiftTab[ ass[x] ];
        if(shift<0){
          ass[x] = -1;
        }else{
          ass[x] -= shift;
        }
      }
    }
  };

  
  VectorTracker::VectorTracker():
    m_data(0){
  }
  
  VectorTracker::VectorTracker(int dim, float largeDistance, const std::vector<float> &normFactors,
                               IDmode idMode, float distanceThreshold, bool tryOpt):
    m_data(new VectorTracker::Data(dim,tryOpt,idMode,distanceThreshold,largeDistance,normFactors)){
  }
  
  VectorTracker::VectorTracker(const VectorTracker &other):
    m_data(other.m_data ? new VectorTracker::Data(*other.m_data) : 0){
  }
  
  VectorTracker &VectorTracker::operator=(const VectorTracker &other){
    if(&other == this) return *this;
    if(!m_data){
      m_data = other.m_data ? new VectorTracker::Data(*other.m_data) : 0;
    }else{
      if(other.m_data){
        *m_data = *other.m_data;
      }else{
        ICL_DELETE(m_data);
      }
    }
    return *this;
  }


  
  VectorTracker::~VectorTracker(){
    ICL_DELETE(m_data);
  }
  
  bool VectorTracker::isNull() const{
    return !m_data;
  }
  
  void VectorTracker::pushData(const std::vector<Vec> &newData){
    if(!m_data){
      ERROR_LOG("tried to push_data on a null-instance");
      return;
    }
    int newNum = (int)newData.size();
    int oldNum = m_data->h();
    int diff = oldNum-newNum;
    
    if(!newNum){
      //DEBUG_LOG("[[[!newNum case]]]");
      m_data->reset();
      m_data->ass.clear();
      m_data->idMask.clear();
      return;
    }
    
    if(!m_data->h()){
      //DEBUG_LOG("[[[!m_data.h() case]]]");
      m_data->addRows(newNum);
      m_data->pushCol(newData);
      m_data->ass.resize(newNum,-1);
      return;
    }
    
    m_data->predict();
    SimpleMatrix<float> distMat;
    if((int)m_data->normFactors.size() == m_data->dim){
      distMat = m_data->createDistanceMatrix(newData,PearsonDist(m_data->normFactors),m_data->largeVal);
    }else{
      if(m_data->normFactors.size()){
        ERROR_LOG("normFactors size must be equal to dimension of data (using 1.0 for all dimensions)");
      }
      distMat = m_data->createDistanceMatrix(newData,sqrt_eucl_dist,m_data->largeVal);
    }
    if(diff){
      m_data->ass = HungarianAlgorithm<float>::apply(distMat);
      // otherwise this is deferred to after trivial assignmnent check
    }
    
    if(newNum > oldNum){//----------------------------------------------------
      //DEBUG_LOG("[[[newNum > oldNum case]]]");
      std::vector<Vec> orderedNewData(newNum);
      int nextBlindValIndex = newNum + diff;
      
      for(int i=0;i<newNum;++i){
        if(m_data->ass[i] >= oldNum){
          orderedNewData[nextBlindValIndex++] = newData[i];
        }else{
          orderedNewData[ m_data->ass[i] ] = newData[i];
        }
      }
      m_data->addRows(-diff);
      m_data->pushCol(orderedNewData);
      
    }else if(oldNum > newNum){ //-----------------------------------------------
      //DEBUG_LOG("[[[oldNum > newNum case]]]" << "  ... ( oldNum:"<< oldNum << " newNum:"<< newNum << " )"); // curr problem...
      std::vector<Vec> orderedNewData(oldNum);
      for(int i=0;i<newNum;++i){
        orderedNewData[ m_data->ass[i] ] = newData[i];
      }
      m_data->pushCol(orderedNewData);
      
      std::vector<int> lostRows;
      lostRows.reserve(diff);
      for(int i=oldNum-diff;i<oldNum;++i){
        // DEBUG_LOG("--> adding lost row:" << m_data->ass[i] );
        lostRows.push_back( m_data->ass[i] );
      }
      m_data->removeRows(lostRows);
      m_data->adaptAssignment(lostRows,oldNum,newNum);
    }else{ // oldNum == newNum ------------------------------------------------
      //DEBUG_LOG("[[[newNum == oldNum case]]]");
      std::vector<Vec> orderedNewData(newNum);
      try{
        if(m_data->tryOpt && m_data->thresh >= 0){
          int assigned = 0;
          std::fill(m_data->ass.begin(),m_data->ass.end(),-1);
          for(int n=0;n<newNum;++n){
            for(int o=0;o<oldNum;++o){
              if(distMat[n][o] < m_data->thresh){
                if(m_data->ass[n]!=-1){
                  throw 1;
                }
                m_data->ass[n] = o;
                assigned++;
              }
            }
          }
          if(assigned != newNum) throw 1;
          //DEBUG_LOG("trivial assignment worked");
        }else{
          throw 1;
        }
      }catch(int){
        //DEBUG_LOG("trivial assignment didn't work");
        m_data->ass = HungarianAlgorithm<float>::apply(distMat);
        // Img32f(Size(distMat.w(),distMat.h()),1,std::vector<float*>(1,distMat.data())).printAsMatrix();
      }
      
      for(int o=0;o<oldNum;++o){
        //DEBUG_LOG("ass[o]: " << m_data->ass[o]);
        orderedNewData[ m_data->ass[o] ] = newData[o];
      }
      m_data->pushCol(orderedNewData);
    }
  }  

  int VectorTracker::getID(int index) const{
    ICLASSERT_RETURN_VAL(!isNull(),-1);

    // DEBUG_LOG("get id(" << index << ")");
    // DEBUG_LOG("... ass.size:" << m_data->ass.size());
    // DEBUG_LOG("... ids.size:" << m_data->ids.size());
    if(index >= 0 && index < (int)m_data->ass.size()){
      
      int ass = m_data->ass.at(index);
      // DEBUG_LOG("... ass[idx]: " << ass);
      if(ass == -1) return -1;
      return m_data->ids.at(ass);

      //      return m_data->ids[ m_data->ass[index] ];
    }else{
      return -1;
    }
  }
  int VectorTracker::getDim() const{
    ICLASSERT_RETURN_VAL(!isNull(),-1);
    return m_data->dim;
  }

} // namespace icl

