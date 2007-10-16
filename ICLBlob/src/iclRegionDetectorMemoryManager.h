#ifndef DATASTORE_H
#define DATASTORE_H
#include <iclException.h>
#include <stdlib.h>


namespace icl{
  namespace regiondetector{
    /// Utility class for efficient memory management \ingroup G_RD
  /**
      The RegionDetectorMemoryManager offers the ability of dynamic memory handling without
      allocation of new data at runtime (unless the fill-factor has reached
      100%. In this case the underlying memory array is reallocated with
      double size.
      
      <b>Note:</b> RegionDetectorMemoryManager of template type <b>T</b> internally stores data
      elements of type <b>T*</b>
  */
    template<class T>
    class RegionDetectorMemoryManager{
      public:
      /// internally used iterator type
      typedef T** iterator;
      
      /// Create a new memory manager with given initial data size and grow factor
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
      /// Destructor
      inline ~RegionDetectorMemoryManager(){
        //delete [] data;
        for(int i=0;i<curr_size;i++){
          delete data[i];
        }
        free(data);
      }
      /// first data element
      inline iterator begin(){
        return data;
      }
      
      /// first not assigned data element
      inline iterator end(){
        return data+curr_index;
      }
      /// count of assigned data elements
      inline int size(){
        return curr_index;
      }
      /// resets the current assigned data index
      /** This function is highly optimized, so it does <b>not</b> check
          whether the internal data array is as large as newsize */
      inline void resize(int newsize=0){
        curr_index = newsize;
      }
      /// resets the internal data to 0 and shriks the data array if possible
      inline void clear(){
        if(curr_index < curr_size/growfactor){
          shrink();
        }
        //printf("cleared curr_index = %d    size = %d \n",curr_index,curr_size);
        curr_index = 0;
      }
      
      /// returns the next free data element
      inline T* next(){
        if(curr_index == curr_size){
          grow();
        }
        return data[curr_index++];
      }
      private:
      
      T **data; ///!< internal data array
      int curr_size; ///!< current data array size
      int curr_index;///!< current count of assigned data
      int growfactor;///!< current grow factor
      static const int INITIAL_CAPACITY = 20; ///!< static default initial data element count (20)
      static const int DEF_GROW_FACTOR = 2;   ///!< static default initial grow factor (2)
   
      /// returns the current amount of used data 
      /** this function is only valied if the data type T has a function called <em>mem</em>
          of type <b>int mem(void)</b> */
      int getCurrBytes(){
        int n = 0;
        for(int i=0;i<curr_size;i++){
          n+=data[i]->mem();
        }
        return n;
      }

      /// internally shrinks the data array to the current size (but not smaller than INITIAL_CAPACITY)
      void shrink(){
        if(curr_size < INITIAL_CAPACITY) return;
        
        //printf("calling \"shrink\" oldsize = %d newsize = %d\n",curr_size,(int)(curr_size/growfactor));

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
      /// internall enlarges the data array
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
