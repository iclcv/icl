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

#ifndef UNARY_OP_H
#define UNARY_OP_H

#include <ICLFilter/OpROIHandler.h>

namespace icl{
  
  /** \cond */
  class MultiThreader;
  /** \endcond */
  
  /// Abstract Base class for Unary Operators \ingroup UNARY
  /** A list of unary operators can be found here:\n
      \ref UNARY
  **/
  class UnaryOp{
    public:

    /// Explicit empty constructor
    UnaryOp();

    /// Explicit copy constructor
    UnaryOp(const UnaryOp &other);

    /// Explicit declaration of the assignment operator
    UnaryOp &operator=(const UnaryOp &other);

    
    /// Destructor
    virtual ~UnaryOp();
      
    /// pure virtual apply function, that must be implemented in all derived classes
    virtual void apply(const ImgBase *operand1, ImgBase **dst)=0;

    /// *NEW* apply function for multithreaded filtering (currently even slower than using one thread)
    virtual void applyMT(const ImgBase *operand1, ImgBase **dst, unsigned int nThreads);

    /// applys the filter usign an internal buffer as output image 
    /** Normally, this function must not be reimplemented, because it's default implementation
        will call apply(const ImgBase *,ImgBase**) using an internal buffer as destination image.
        This destination image is returned. */
    virtual const ImgBase *apply(const ImgBase *src);
    
    /// sets if the image should be clip to ROI or not
    /**
      @param bClipToROI true=yes, false=no
    */    
    void setClipToROI (bool bClipToROI) { m_oROIHandler.setClipToROI(bClipToROI); }
    
    /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
    /**
      @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
    */
    void setCheckOnly (bool bCheckOnly) { m_oROIHandler.setCheckOnly(bCheckOnly); }
    
    /// returns the ClipToROI status
    /**
      @return true=ClipToROI is enable, false=ClipToROI is disabled
    */
    bool getClipToROI() const { return m_oROIHandler.getClipToROI(); }
    
    /// returns the CheckOnly status
    /**
      @return true=CheckOnly is enable, false=CheckOnly is disabled
    */
    bool getCheckOnly() const { return m_oROIHandler.getCheckOnly(); }
    

    /// Creates a UnaryOp instance from given string definition
    /** Supported definitions have the followin syntax:
        OP_SPEC<(PARAM_LIST)>
         
        examples are: 
        - sobelX3x3
        - median(5x3)
        - closeBorder(3x3)

        A complete list of OP_SPECS can be obtained by the static listFromStringOps function.
        Each specific parameter list's syntax is accessible using the static getFromStringSyntax function.
        
    */
    static UnaryOp *fromString(const std::string &definition) throw (ICLException);

    /// gives a string syntax description for given opSpecifier
    /** opSpecifier must be a member of the list returned by the static function listFromStringOps */
    static std::string getFromStringSyntax(const std::string &opSpecifier) throw (ICLException);

    /// returns a list of all supported OP_SPEC values for the fromString function
    static std::vector<std::string> listFromStringOps();

    /// creates, applies and releases a UnaryOp defined by given definition string
    static void applyFromString(const std::string &definition, const ImgBase *src, ImgBase **dst) throw (ICLException);
    
    protected:
    bool prepare (ImgBase **ppoDst, depth eDepth, const Size &imgSize, 
                  format eFormat, int nChannels, const Rect& roi, 
                  Time timestamp=Time::null){
      return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
    }
    
    /// check+adapt destination image to properties of given source image
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc) {
      return m_oROIHandler.prepare(ppoDst, poSrc);
    }
    
    /// check+adapt destination image to properties of given source image
    /// but use explicitly given depth
    virtual bool prepare (ImgBase **ppoDst, const ImgBase *poSrc, depth eDepth) {
      return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
    }

    MultiThreader *m_poMT;
    
    private:
  
    OpROIHandler m_oROIHandler;
    
    ImgBase *m_buf;
  };    


#define DYNAMIC_UNARY_OP_CREATION_FUNCTION(NAME)       \
  extern "C" {                                         \
    UnaryOp *create_##NAME(const std::string &s){      \
      return NAME(s);                                  \
    }                                                  \
  }
  
}


#endif
