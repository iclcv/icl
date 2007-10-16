#ifndef REGION_DETECTOR_VECTOR_H
#define REGION_DETECTOR_VECTOR_H
namespace icl{
 namespace regiondetector{
   /// Support class for efficient memory management \ingroup G_RD
   /**
       The RegionDetectorVector-Template class defines a Abstract Data Type
       for incremental storage of Data using the push_back(T t)
       function.
       For better performance of the push_back mechanism, the 
       RegionDetectorVector uses special memory-management.
       
       The data is stored in a unique array, which's size is doubled
       if a push_back call would write beyond the array border.
       
       A push_back call runs in constant time, with only 3 operations
       and without allocation of new memory
       (unless the array size must be doubled)
       
       <b>Note</b> that the memory usage of a RegionDetectorVector may be twice as much
       as the memory usage of the contained data.
       
       <b>Note further:</b> A RegionDetectorVector of template type <b>T</b> hold data elements 
       of type <b>T*</b>
    */
    template<class T>
      class RegionDetectorVector{
        public:
      /// internally used iterator type
      typedef T** iterator;
      
      /// craeate a new RegionDetectorVector with given initial data size and given growfactor
      /** @param size initial data size (size of allocated data <b>not</b> the element count,
                      which is 0 initially
          @param growfactor if the internally allocated data is not large enough to push
                            the next element, the data array is enlarged internally by this
                            factor (growfactor must be >=2) */
      inline RegionDetectorVector(int size=0, int growfactor=0){
        if(size == 0)size = INITIAL_CAPACITY;
        if(growfactor == 0) growfactor = DEF_GROW_FACTOR;
        
        data = new T*[size];
        curr_size = size;
        this->growfactor = growfactor; 
        curr_index = 0;
      }
      /// Destructor
      inline ~RegionDetectorVector(){
        delete [] data;
      }
      /// first data element
      inline iterator begin(){
        return data;
      }
      /// first invalid data element
      inline iterator end(){
        return data+curr_index;
      }
      /// current elemenmt count
      /** returns <b>not</b> the size of the internal data array */
      inline int size() const{
        return curr_index;
      }
      /// internally moved the current data pointer
      /** This function is highly optimized, so it does <b>not</b> check
          whether the internal data array has at least the given size. */
      inline void resize(int newsize=0){
        curr_index = newsize;
      }
      /// adds a new data item to the vector
      /** internal data is enlarged on demand*/
      inline void push_back(T *t){
        if(curr_index==curr_size){
          grow();
        }
        data[curr_index++]=t;
      }
      /// returns the data element at given index
      inline T* operator[](int i){
        return data[i];
      }
      /// returns the data element at given index (const)
      inline const T* operator[](int i) const{
        return data[i];
      }
      /// resets the internal data pointer to the begin of the internal data array
      /** no memory is released here ever*/
      inline void clear(){
        curr_index = 0;
      }
      
        private:
      T **data;             ///<! internal data pointer
      int curr_size;        ///<! current size of data array 
      int curr_index;       ///<! current date element counter
      int growfactor;       ///<! factor for array enalargement
      static const int INITIAL_CAPACITY = 20; ///<! static default initial capacity (20)
      static const int DEF_GROW_FACTOR = 2;   ///<! static default initial grow factor (2)
      
      /// internally used data enlargment function
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
