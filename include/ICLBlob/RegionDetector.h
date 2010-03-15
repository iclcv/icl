/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLBlob/RegionDetector.h                       **
** Module : ICLBlob                                                **
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
*********************************************************************/

#ifndef ICL_REGION_DETECTOR_H
#define ICL_REGION_DETECTOR_H

#include <ICLCore/Img.h>
#include <ICLBlob/ScanLine.h>
#include <ICLBlob/Region.h>
#include <ICLBlob/RegionPart.h>

namespace icl{
  
  
  /// Region detection class (also known as "Connected Components)"
  /** Regions are detected by special implementation of common "connected components" algorithm:
      Input image I is processed iteratively from left to right and from top to bottom using a small 
      iteration window. At location X=I(x,y) the above pixel A=I(x,y-1) and the left pixel L=I(x-1,y) are
      compared. Here 4 cases can occur:
      -# X=A but X!=L in this case, X belongs to the region containing L
      -# X=L but X!=A in this case, X belongs to the region containing A
      -# X!=A
      
  */
  class RegionDetector{
    public:

    RegionDetector(unsigned int minSize=0, unsigned int maxSize=2<<20, icl64f minVal=0, icl64f maxVal = 255, bool m_createTree=false);
    ~RegionDetector();
    
    void setRestrictions(unsigned int minSize=0, unsigned int maxSize=2<<20, icl64f minVal=0, icl64f maxVal = 255);
    void setRestrictions(const Range<unsigned int> &sizeRange, const Range<icl64f> &valueRange);
    void setCreateTree(bool on) { m_createTree = on; }

    const std::vector<Region> &detect(const ImgBase *image);
    
    private:
    
    void createTree(const ImgBase *image);
 
    template<class T, typename Compare>
    void detect_intern(const Img<T> &image, Compare cmp);
    void adaptLIM(int w, int h);
    void resetPartList();
    RegionPart *newPart();
    
    std::vector<Region> m_vecBlobData;
    std::vector<Region> m_vecAllBlobData;
    std::vector<RegionPart*> m_vecLIM;
    std::vector<RegionPart*> m_vecParts;
    
    unsigned int m_uiMinSize;    
    unsigned int m_uiMaxSize;
    icl64f m_dMinVal;
    icl64f m_dMaxVal;
    
    bool m_createTree;    
    class Tree;
    Tree *m_tree;
  };
}

#endif
