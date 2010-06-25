/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/NeighborhoodOp.h                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#ifndef NEIGHBORHOOD_OP_H
#define NEIGHBORHOOD_OP_H

#include <ICLFilter/UnaryOp.h>

namespace icl {
  /// unary operators that work on each pixels neighborhood \ingroup UNARY \ingroup NBH
  /** TODO:: check!!
      The NeighborhoodOp class builds a base class for unary operations employing
      a filter mask which is moved over the ROI of the source image(s), 
      e.g. convolution filters use some convolution masks.
      To this end the class provides members to store the size and anchor
      of the filter mask.

      Special care has to be taken, when applying a filter mask to the border
      of an image, e.g. in case of full-image ROI. In this case the filter
      might access undefined pixel values outside the image, actually causing
      a segfault in most cases.

      Hence, the used ROI size of the source image is <em> shrinked </em> if
      neccessary, such that the filter mask always fits into the image, when
      moved over the ROI. For this purpose the method adaptROI is provided,
      which computes the to-be-used ROI. The ROI of the source image is not
      actually changed, which makes the filter operation thread safe, because
      simultaneous operations running on the source image within different
      threads do not interfere.
  */
  class NeighborhoodOp : public UnaryOp {
    public:
    
    ///Destructor
    virtual ~NeighborhoodOp(){}
    
    /// compute neccessary ROI offset and size
    /** This functions computes the to-be-used ROI for the source image, 
        such that the filter mask of given size (oMaskSize) fits everywhere
        into the image if placed arbitrarily within the ROI.
        The original ROI of the source image is not changed, instead the
        adapted ROI is returned in parameters oROIsize and oROIoffset.
        @param poSrc  image whose ROI is adapted
        @param oROIoffset  new ROI offset
        @param oROIsize    new ROI size
        @return whether a valid ROI remains
        */
    bool computeROI(const ImgBase *poSrc, Point& oROIoffset, Size& oROIsize);

    /// *NEW* apply function for multithreaded filtering (reimplemented here for special roi handling!)
    virtual void applyMT(const ImgBase *operand1, ImgBase **dst, unsigned int nThreads);
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    protected:
    NeighborhoodOp() : m_oMaskSize(1,1), m_oAnchor (0,0) {}
    NeighborhoodOp(const Size &size) {
      setMask (size);
    }
    
    void setMask(const Size &size) {
        m_oMaskSize = adaptSize(size);
        m_oAnchor   = Point (m_oMaskSize.width/2, m_oMaskSize.height/2);
    }
    void setMask(const Size &size, const Point &anchor){
      m_oMaskSize = adaptSize(size);
      m_oAnchor = anchor;
    }
    void setROIOffset(const Point &offs){
      m_oROIOffset = offs;
    }
    public:
    const Size &getMaskSize() const{
      return m_oMaskSize;
    } 
    const Point &getAnchor() const {
      return m_oAnchor;
    }
    const Point &getROIOffset() const{
      return m_oROIOffset;
    }
    protected:

    /// prepare filter operation: ensure compatible image format and size
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc);
    
    /// prepare filter operation: as above, but with depth parameter
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepht);
    
    /// this function can be reimplemented e.g to enshure an odd mask width and height
     /** E.g. some implementations of Neighborhood-operation could demand odd or even
         mask size parameters. In this case, this function can be implemented in another
         way. (Example: MedianOp)
         @param size size to ajust
         @return the given size in this base implementation
         **/
    virtual Size adaptSize(const Size &size){ return size; }
     
    protected:
     ///TODO: later private with getter and setter functions
     Size  m_oMaskSize;  ///< size of filter mask
     Point m_oAnchor;    ///< anchor of filter mask
     Point m_oROIOffset; ///< to-be-used ROI offset for source image
  };
}
#endif
