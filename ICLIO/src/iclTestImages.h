#ifndef TESTIMAGES_H
#define TESTIMAGES_H

#include <iclImg.h>
#include <string>

namespace icl{
  /// Utility class for creating test images \ingroup UTILS_G
  class TestImages{
    public:
    /// creates a new testimage instance
    /** possible values for name are
        - women
        - house
        - tree
        - parrot
        - windows
        - flowers
        @param name name identifier of the image
        @param size destination size of the image
        @param f format of the image
        @param d depth of the image
        @return new image (ownership is passed to the caller!)
    */
    static ImgBase* create(const std::string& name, 
                           const Size &size,
                           format f=formatRGB, 
                           depth d=depth8u);
    
    /// creats testimages in original size
    /** @param name name identifier of the image 
        @param f format of the image
        @param d depth of the image 
        @return new image (ownership is passed to the caller!)
    **/
    static ImgBase *create(const std::string& name, 
                           format f=formatRGB, 
                           depth d=depth8u);

    /// writes the image to the disc an shows it using xv.
    /** @param image image to write and to show
        @param tmpName temporary filename for this image 
        @param msec_to_rm_call this time in msec is waited for
                               xv to come up and to read the tmp image
    **/
    static void xv(const ImgBase *image, 
                   const std::string& tmpName="./tmp_image.ppm",
                   long msec_to_rm_call=1000);

    /// writes the image to the hard disk and show it using the given shell command
    /** @param image image to show 
        @param showCommand command to visualize the image. As default, the iclxv
                           viewer of the ICLQt package is used. Enshure, that
                           at least a link to this viewer is available in your path
                           variable. A temporarily created filename (composed of
                           a prefix, a current-system-time-body and a file name 
                           postfix is inserted where the %s token is found
        @param msec_to_rm_call when showing images using other image viewers,
                               the temporarily created image must be deleted when
                               the extern editor has read the image. This value
                               determines how many milliseconds should be waited
                               before the rmCommand is called.
        @param rmCommand command to remove the temporary image (something like
                         "rm -rf %s" */
    static void show(const ImgBase *image, 
                     const std::string &showCommand="iclxv -input %s -delete",
                     long msec_to_rm_call=0,
                     const std::string &rmCommand="");
    private:
    /// internal creation funtion for image
    static Img8u *internalCreate(const std::string &name);
  };
  
  /// shortcurt function to create the "macaw"-image
  /** @return new image (ownership is passed to the caller!) */
  ImgBase *createImage_macaw();

  /// shortcurt function to create the "windows"-image
  /** @return new image (ownership is passed to the caller!) */
  ImgBase *createImage_windows();
  
  /// shortcurt function to create the "flowers"-image
  /** @return new image (ownership is passed to the caller!) */
  ImgBase *createImage_flowers();
}

#endif
