#ifndef REGION_DETECTOR_VECTOR_H
#define REGION_DETECTOR_VECTOR_H
namespace icl{
 namespace regiondetector{
/**
The RegionDetectorVector-Template class defines a Abstract Data Type
for incremental storage of Data using the push_back(T t)
function.
For better performance of the push_back mechanism, the 
RegionDetectorVector uses special memory-management.

The data is stored in a unique array, which size is doubled
if a push_back call would write beyond the array border.

a push_back call runs in constant time, with only 3 operations
and without allocation of new memory
(unless the array size must be doubled)

Note that the memory usage of a RegionDetectorVector may be twice as much
as the memory usage of the contained data.
*/

    template<class T>
      class RegionDetectorVector{
        public:
      typedef T** iterator;
      
      inline RegionDetectorVector(int size=0, int growfactor=0){
        if(size == 0)size = INITIAL_CAPACITY;
        if(growfactor == 0) growfactor = DEF_GROW_FACTOR;
        
        data = new T*[size];
        curr_size = size;
        this->growfactor = growfactor; 
        curr_index = 0;
      }
      inline ~RegionDetectorVector(){
        delete [] data;
      }
      inline iterator begin(){
        return data;
      }
      inline iterator end(){
        return data+curr_index;
      }
      inline int size() const{
        return curr_index;
      }
      inline void resize(int newsize=0){
        curr_index = newsize;
      }
      inline void push_back(T *t){
        if(curr_index==curr_size){
          grow();
        }
        data[curr_index++]=t;
      }
      inline T* operator[](int i){
        return data[i];
      }
      inline const T* operator[](int i) const{
        return data[i];
      }
      
      inline void clear(){
        curr_index = 0;
      }
      
        private:
      T **data;
      int curr_size;
      int curr_index;
      int growfactor;
      static const int INITIAL_CAPACITY = 20;
      static const int DEF_GROW_FACTOR = 2;
      
      void grow(){
        curr_size*=growfactor;
        T** new_data = new T*[curr_size];
        memcpy(new_data,data,curr_size/growfactor*sizeof(T*));
     
        delete [] data;
        data = new_data;
      }
    };
  }
}


#endif
