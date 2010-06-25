/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/InplaceOp.h                          **
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

#ifndef ICL_INPLACE_OP
#define ICL_INPLACE_OP

#include <ICLCore/ImgBase.h>

namespace icl{
  
  /// Interface class for inplace operators \ingroup INPLACE
  /** Inplace operators work on image pixels directly. Common examples
      are arithmetical expressions like IMAGE *= 2. Useful inplace 
      operations are arithmetical, logical, binary-logical, or table-lookups.
      
      @see ArithmeticalInplaceOp 
      @see LogicalInplaceOp
  */
  class InplaceOp{
    public:

    /// Create a new Inplace op (ROI-only flag is set to true)
    InplaceOp():m_bROIOnly(true){}

    /// apply function transforms source image pixels inplace
    virtual ImgBase* apply(ImgBase *src)=0;
    
    /// setup the operation to work on the input images ROI only or not
    void setROIOnly(bool roiOnly) { 
      m_bROIOnly=roiOnly; 
    }
    
    /// returns whether operator is in "roi-only" mode or not
    bool getROIOnly() const { 
      return m_bROIOnly; 
    }
    
    private:
    /// "roi-only" flag
    bool m_bROIOnly;
  };
}

#endif
