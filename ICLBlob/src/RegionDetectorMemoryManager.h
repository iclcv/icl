#ifndef DATASTORE_H
#define DATASTORE_H

#include <Exception.h>
#include <stdlib.h>
/**
The RegionDetectorMemoryManager offers the ability of dynamic memory handling without
allocation of new data at runtime (unless the fill-factor has reached
100%. In this case the underlying memory array is reallocated with
double size.
*/
namespace icl{
  namespace regiondetector{
    template<class T>
      class RegionDetectorMemoryManager{
        public:
      typedef T** iterator;
      
      inline RegionDetectorMemoryManager(int size=0, int growfactor=0){
        if(size == 0)size = INITIAL_CAPACITY;
        if(growfactor == 0) growfactor = DEF_GROW_FACTOR;
        
        //data = new T*[size];
        data = (T**)malloc(size*sizeof(T*));
        
        for(int i=0;i<size;i++){
          data[i] = new T;
        }
        curr_size = size;
        this->growfactor = growfactor; 
        curr_index = 0;
      }
      inline ~RegionDetectorMemoryManager(){
        //delete [] data;
        for(int i=0;i<curr_size;i++){
          delete data[i];
        }
        free(data);
      }
      inline iterator begin(){
        return data;
      }
      inline iterator end(){
        return data+curr_index;
      }
      inline int size(){
        return curr_index;
      }
      inline void resize(int newsize=0){
        curr_index = newsize;
      }
      inline void clear(){
        if(curr_index < curr_size/growfactor){
          shrink();
        }
        printf("cleared curr_index = %d    size = %d \n",curr_index,curr_size);
        curr_index = 0;
      }
      inline T* next(){
        if(curr_index == curr_size){
          grow();
        }
        return data[curr_index++];
      }
        private:

      
      T **data;
      int curr_size;
      int curr_index;
      int growfactor;
      static const int INITIAL_CAPACITY = 20;
      static const int DEF_GROW_FACTOR = 2;
   
      int getCurrBytes(){
        int n = 0;
        for(int i=0;i<curr_size;i++){
          n+=data[i]->mem();
        }
        return n;
      }

      void shrink(){
        if(curr_size < INITIAL_CAPACITY) return;
        
        printf("calling \"shrink\" oldsize = %d newsize = %d\n",curr_size,(int)(curr_size/growfactor));

        int old_size = curr_size;
        curr_size/=growfactor;
        
        T** new_data = (T**)malloc(sizeof(T)*curr_size);
        memcpy(new_data,data,curr_size*sizeof(T*));
        for(int i=curr_size;i<old_size;i++){
          delete data[i];
        }      
        free(data);
        data = new_data;
      
      }
      void grow(){
        
        int old_size = curr_size;
        curr_size*=growfactor;
        
        //T** new_data = new T*[curr_size];
        T** new_data = (T**)malloc(sizeof(T)*curr_size);
        memcpy(new_data,data,old_size*sizeof(T*));
        for(int i=old_size;i<curr_size;i++){
          new_data[i]=new T();
        }      
        //delete [] data;
        free(data);
        data = new_data;
      }
    };
  }
}
#endif
