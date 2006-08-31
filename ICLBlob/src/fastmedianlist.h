#ifndef FASTMEDIANLIST_H
#define FASTMEDIANLIST_H

#include <string.h>

namespace icl{
  
  class FastMedianList{
    public:
    inline FastMedianList(int size,int t2=-1, int t3=-1, int t4=-1):
      size(size),t2(t2),t3(t3),t4(t4){
      table = new int[size];
      clear();
    }
    inline ~FastMedianList(){
      delete [] table;
    }
    
    inline int add(int index){
      table[index]++;
      count++;
      return count;
    }
    inline void add(int index, int val){
      table[index]++;
      count++;
      if(val > t2)return;
      table[index]++;
      count++;
      if(val > t3)return;
      table[index]++;
      count++;
      if(val > t4)return;
      table[index]++;
      count++;
    }
    
    inline int median(){
      if(count == 0)return -1;
      int mass=0;
      int count2 = count/2;
      for(int i=0;i<size;i++){
        mass+=table[i];
        if(mass > count2){
          return i;
        }
      }
      return 0;
    }
    
    inline void clear(){
      memset(table,0,size*sizeof(int));
      count=0;
    }
    
    inline int mean(){
      int m=0;
      for(int i=0;i<size;i++){
        m+=table[i]*i;
      }
      return m/count;
      
    }
    
    inline int variance(){
      int var=0;
      int m = mean();
      for(int i=0;i<size;i++){
        var+=( table[i]*(int)pow(i-m,2) );
      }
      return var/count;
      
    }
    inline int getSize(){
      return count;
    }
    
    protected:
    int count,size,t2,t3,t4,*table;
  };
  
}

#endif
