#ifndef FASTMEDIANLIST_H
#define FASTMEDIANLIST_H

#include <string.h>

namespace icl{
  
  class FastMedianList{
    public:
    inline FastMedianList(int size=0,int t2=-1, int t3=-1, int t4=-1):
      m_iSize(size),m_iT2(t2),m_iT3(t3),m_iT4(t4), m_piTable(size ? new int[size]: 0){
      clear();
    }
    inline FastMedianList(const FastMedianList &l):
      m_iSize(l.m_iSize),m_iT2(l.m_iT2),m_iT3(l.m_iT3),m_iT4(l.m_iT4){
      m_piTable = m_iSize ? new int[m_iSize] : 0;
      clear();
    }
    inline FastMedianList &operator=(const FastMedianList &l){
      m_iSize = l.m_iSize;
      m_iT2   = l.m_iT2;
      m_iT3   = l.m_iT3;
      m_iT4   = l.m_iT4;
      m_piTable = m_iSize ? new int[m_iSize] : 0;
      clear();
      return *this;
    }
    inline ~FastMedianList(){
      if(m_piTable) delete [] m_piTable;
    }
    
    inline void init(int size, int t2=-1, int t3=-1, int t4=-1){
      m_iSize = size;
      m_iT2 = t2;
      m_iT3 = t3;
      m_iT4 = t4;
      if(m_piTable)delete [] m_piTable;
      m_piTable = new int[size];
    }
    
    inline int add(int index){
      m_piTable[index]++;
      m_iCount++;
      return m_iCount;
    }
    inline void add(int index, int val){
      m_piTable[index]++;
      m_iCount++;
      if(val > m_iT2)return;
      m_piTable[index]++;
      m_iCount++;
      if(val > m_iT3)return;
      m_piTable[index]++;
      m_iCount++;
      if(val > m_iT4)return;
      m_piTable[index]++;
      m_iCount++;
    }
    
    inline int median(){
      if(m_iCount == 0)return -1;
      int mass=0;
      int count2 = m_iCount/2;
      for(int i=0;i<m_iSize;i++){
        mass+=m_piTable[i];
        if(mass > count2){
          return i;
        }
      }
      return 0;
    }
    
    inline void clear(){
      memset(m_piTable,0,m_iSize*sizeof(int));
      m_iCount=0;
    }
    
    inline int mean(){
      int m=0;
      for(int i=0;i<m_iSize;i++){
        m+=m_piTable[i]*i;
      }
      return m/m_iCount;
      
    }
    
    inline int variance(){
      int var=0;
      int m = mean();
      for(int i=0;i<m_iSize;i++){
        var+=( m_piTable[i]*(int)pow(float(i-m),2) );
      }
      return var/m_iCount;
      
    }
    inline int getSize(){
      return m_iCount;
    }
    
    protected:
    int m_iCount,m_iSize,m_iT2,m_iT3,m_iT4,*m_piTable;
  };
  
}

#endif
