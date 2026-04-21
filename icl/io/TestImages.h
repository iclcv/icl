// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/Img.h>
#include <string>

namespace icl::io {
  /// Utility class for creating test images \ingroup UTILS_G
  class ICLIO_API TestImages{
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
        @param f core::format of the image
        @param d core::depth of the image
        @return new image (ownership is passed to the caller!)
    */
    static core::ImgBase* create(const std::string& name,
                                 const utils::Size &size,
                                 core::format f=core::formatRGB,
                                 core::depth d=core::depth8u);

    /// creats testimages in original size
    /** @param name name identifier of the image
        @param f core::format of the image
        @param d core::depth of the image
        @return new image (ownership is passed to the caller!)
    **/
    static core::ImgBase *create(const std::string& name,
                                 core::format f=core::formatRGB,
                                 core::depth d=core::depth8u);

    /// writes the image to the disc an shows it using xv.
    /** @param image image to write and to show
        @param tmpName temporary filename for this image
        @param msec_to_rm_call this time in msec is waited for
                               xv to come up and to read the tmp image
    **/
    static void xv(const core::ImgBase *image,
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
    static void show(const core::ImgBase *image,
                     const std::string &showCommand="icl-xv -input %s -delete",
                     long msec_to_rm_call=0,
                     const std::string &rmCommand="");
    private:
    /// internal creation funtion for image
    static core::Img8u *internalCreate(const std::string &name);
  };

  /// shortcurt function to create the "macaw"-image
  /** @return new image (ownership is passed to the caller!) */
  ICLIO_API core::ImgBase *createImage_macaw();

  /// shortcurt function to create the "windows"-image
  /** @return new image (ownership is passed to the caller!) */
  ICLIO_API core::ImgBase *createImage_windows();

  /// shortcurt function to create the "flowers"-image
  /** @return new image (ownership is passed to the caller!) */
  ICLIO_API core::ImgBase *createImage_flowers();

  /// shortcurt function to create the famous "lena"-image
  /** @return new image (ownership is passed to the caller!) */
  ICLIO_API core::ImgBase *createImage_lena();

  /// shortcurt function to create the famous "cameraman"-image
  /** @return new image (ownership is passed to the caller!) */
  ICLIO_API core::ImgBase *createImage_cameraman();

  /// shortcurt function to create the "mandril"-image
  /** @return new image (ownership is passed to the caller!) */
  ICLIO_API core::ImgBase *createImage_mandril();



  } // namespace icl::io