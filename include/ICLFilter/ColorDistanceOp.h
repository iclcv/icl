/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/ColorDistanceOp.h                    **
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

#ifndef ICL_COLOR_DISTANCE_OP_H
#define ICL_COLOR_DISTANCE_OP_H

#include <ICLFilter/UnaryOp.h>

namespace icl {
  
  /// Class for creating a color distance map
  class ColorDistanceOp : public UnaryOp{
    public:
    
    /// Dummy constructor (creates an uninitialized instance)
    ColorDistanceOp(){}
    
    /// create a new instance with given reference color and optional threshold
    /** If the given threshold is not -1, the resulting distance values are 
        thresholded (so that pixels, that are closer than threshold to the
        referenceColor are set to 255. The reference color needs at least 3 entries.*/
    template<class Container>
    ColorDistanceOp(const Container &vec, double threshold=-1){
      init(vec,threshold);
    }
    
    /// create with given iterator range for the reference color initialization
    template<class ForewardIterator>
    ColorDistanceOp(ForewardIterator begin, ForewardIterator end, double threshold=-1){
      init(std::vector<double>(begin,end), threshold);
    }

    /// creates a color distance map
    /** If the current threshold is not -1, then the result image becomes
        a binary 8-image. Otherwise, the resulting image is adapted to an Img32f.
        Only if the source depth is depth64f, and Img64f is also used for the result. 

        The source image is assumed to have 3 channels
    */
    void apply(const ImgBase *src, ImgBase **dst);
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    /// sets the threshold value
    inline void setThreshold(double threshold){
      m_threshold = threshold;
    }

    /// sets the current reference color
    inline void setReferenceColor(const std::vector<double> &refColor){
      init(refColor,m_threshold);
    }

    /// sets both reference color and threshold at once
    template<class Container>
    inline void init(const Container &refColor, double threshold=-1){
      m_refColor.assign(refColor.begin(),refColor.end());
      m_threshold = threshold;
      ICLASSERT_THROW(m_refColor.size() >= 3, 
                      ICLException("ColorDistanceOp::setReferenceColor: ref color needs at least 3 entries"));
    }
    
    private:
    /// internal reference color
    std::vector<double> m_refColor;
    
    /// internal threshold
    double m_threshold;
  };
 
}
#endif
