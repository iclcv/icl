/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/ImageSplitter.h                      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_IMAGE_SPLITTER_H
#define ICL_IMAGE_SPLITTER_H

#include <ICLCore/ImgBase.h>
#include <vector>

namespace icl{

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
  class ImageSplitter{
    public:
    /// splits a source image into given number of parts
    static std::vector<ImgBase*> split(ImgBase *src, int nParts);
    
    /// splits a const source image into a given number of const parts
    static const std::vector<ImgBase*> split(const ImgBase *src, int nParts);
    
    /// releases all images within the given vector
    static void release(const std::vector<ImgBase*> &v);
    
    private:
    /// private constructor 
    ImageSplitter(){}
    /// internally used static splitting function
    /** Note: the resulting images must be deleted manually <b>and</b>
        the given parts vector must be given, initalized with NULL pointers.
    **/
    static void splitImage(ImgBase *src, std::vector<ImgBase*> &parts);
  };
}

#endif
