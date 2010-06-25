/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/TestImages.h                             **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_TESTIMAGES_H
#define ICL_TESTIMAGES_H

#include <ICLCore/Img.h>
#include <string>

namespace icl{
  /// Utility class for creating test images \ingroup UTILS_G
  class TestImages{
    public:
    /// creates a new testimage instance
    /** possible values for name are
        - lena
        - cameraman
        - mandril
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
                     const std::string &showCommand="icl-xv -input %s -delete",
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

  /// shortcurt function to create the famous "lena"-image
  /** @return new image (ownership is passed to the caller!) */
  ImgBase *createImage_lena();

  /// shortcurt function to create the famous "cameraman"-image
  /** @return new image (ownership is passed to the caller!) */
  ImgBase *createImage_cameraman();

  /// shortcurt function to create the "mandril"-image
  /** @return new image (ownership is passed to the caller!) */
  ImgBase *createImage_mandril();



}

#endif
