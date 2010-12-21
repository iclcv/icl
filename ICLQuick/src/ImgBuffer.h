#ifndef ICL_IMG_BUFFER_H
#define ICL_IMG_BUFFER_H

#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>

namespace icl{
  
  /// Singelton class that provides access to reusable temporary images
  /** This class is heavily used within ICL's Quick-API in order to 
      avoid unessessary memory allocations. */
  class ImgBuffer : public Uncopyable{
    struct Data; //!< Internal data
    Data data;   //!< Internal data
    ImgBuffer(); //!< private Constructur -> use instance() function to access singelton
    public:
    /// Destructor
    ~ImgBuffer(){}
    
    /// Provides access to the singelton instance
    static ImgBuffer *instance();
    
    /// returns any available (or new) temporary image of given depth
    template<class T>
    Img<T> *get();

    /// returns any available (or new) image with given depth, size and channel count
    template<class T>
    Img<T> *get(const Size &size, int channels);

    /// returns any available (or new) image given depth and parameters
    template<class T>
    Img<T> *get(const ImgParams &params);


    /// non-template based version with given depth
    ImgBase *get(depth d);

    /// non-template based version with given depth, size and channels
    ImgBase *get(depth d, const Size &size, int channels);

    /// non-template based version with given depth and param struct
    ImgBase *get(depth d, const ImgParams &p);
  };
  
}

#endif
