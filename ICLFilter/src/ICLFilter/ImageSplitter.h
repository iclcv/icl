// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
// forward declaration (was #include <ICLCore/ImgBase.h>)
#include <vector>
namespace icl { namespace core { class ImgBase; template<class T> class Img; } }

namespace icl{
  namespace filter{

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

    */
    class ICLFilter_API ImageSplitter{
      public:
      /// splits a source image into given number of parts
      static std::vector<core::ImgBase*> split(core::ImgBase *src, int nParts);

      /// splits a const source image into a given number of const parts
      static const std::vector<core::ImgBase*> split(const core::ImgBase *src, int nParts);

      /// releases all images within the given vector
      static void release(const std::vector<core::ImgBase*> &v);

      private:
      /// private constructor
      ImageSplitter(){}
      /// internally used static splitting function
      /** Note: the resulting images must be deleted manually <b>and</b>
          the given parts vector must be given, initalized with NULL pointers.
      **/
      static void splitImage(core::ImgBase *src, std::vector<core::ImgBase*> &parts);
    };
  } // namespace filter
}
