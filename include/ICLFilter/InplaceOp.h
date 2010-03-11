/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

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
