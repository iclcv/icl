#ifndef ICL_IMAGE_SPLITTER_H
#define ICL_IMAGE_SPLITTER_H

#include <iclShallowCopyable.h>
#include <iclImgBase.h>
#include <vector>

namespace icl{

  /** \cond */
  class ImageSplitterImpl;
  class ImageSplitterImplDelOp{
    public: static void delete_func(ImageSplitterImpl *impl);
  };
  /** \endcond */
  
  
  /// Utility class to split an images roi into a set of shallow copies
  /** In some cases it is useful, to devide a task, that should be applied on an
      images ROI, into a set of disjoint subtasks. This can be achieved by
      generating a set of shallow copied images with disjoint ROIs. E.g. if
      we have a source image of size 320x240 (with full ROI). This can be split
      into e.g. 4 sub images with the following ROIs:\n
      -# 0,0,320,60
      -# 0,60,320,60
      -# 0,120,320,60
      -# 0,180,320,60
      
      Of course, we can split the source images also vertically, but a horizontal cut
      is closer to the internal data representation, which is also aligned horizontally.
      
      A common application for splitting images is multi-threading: Instead of applying
      a filter on an images ROI, the image can easily be split using the ImageSplitter,
      and the filter can be applied on each of the resulting images parts in a dedicated
      thread.
      
      \section CONST "const-ness"
      In some application also "un-const" images should be split. In this case, the image
      should be made unconst using const_cast<ImgBase*>(..). Although this is a bit 
      circumstantial, no implicit cast operation is provided to avoid an exploit in the 
      Img's const mechanism.
  */
  class ImageSplitter : public ShallowCopyable<ImageSplitterImpl,ImageSplitterImplDelOp>{
    public:
    /// Creates a "null" image splitter (see ShallowCopyable)
    ImageSplitter();
    
    /// Create new image splitter from a given source image into n disjoint parts
    ImageSplitter(const ImgBase *src, int n);
    
    /// returns the number of parts
    int getPartCount();
    
    /// returns the image part of given image
    const ImgBase *operator[](int idx);

    /// internally used static splitting function
    /** Note: the resulting images must be deleted manually <b>and</b>
        the given parts vector must be given, initalized with NULL pointers.
    **/
    static void splitImage(const ImgBase *src,std::vector<const ImgBase*> &parts);
  };
}

#endif
