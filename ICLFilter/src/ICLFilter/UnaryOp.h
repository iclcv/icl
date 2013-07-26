/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryOp.h                      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLFilter/OpROIHandler.h>
#include <ICLUtils/Configurable.h>

namespace icl{
  /** \cond */
  namespace utils{
    class MultiThreader;
  }
  /** \endcond */

  namespace filter{
    
    
    /// Abstract Base class for Unary Operators \ingroup UNARY
    /** A list of unary operators can be found here:\n
        \ref UNARY
    **/
    class UnaryOp : public utils::Configurable{
      void initConfigurable();
      
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
      virtual void apply(const core::ImgBase *operand1, core::ImgBase **dst)=0;
  
      /// apply function for multithreaded filtering (currently even slower than using one thread)
      virtual ICL_DEPRECATED void applyMT(const core::ImgBase *operand1, 
                                          core::ImgBase **dst, unsigned int nThreads);
  
      /// applys the filter usign an internal buffer as output image 
      /** Normally, this function must not be reimplemented, because it's default implementation
          will call apply(const ImgBase *,ImgBase**) using an internal buffer as destination image.
          This destination image is returned. */
      virtual const core::ImgBase *apply(const core::ImgBase *src);
      
      /// function operator (alternative for apply(src,dst)
      inline void operator()(const core::ImgBase *src, core::ImgBase **dst){
        apply(src,dst);
      }

      /// function operator for the implicit destination apply(src) call
      inline const core::ImgBase *operator()(const core::ImgBase *src){
        return apply(src);
      }

      /// reference based function operator
      inline const core::ImgBase &operator()(const core::ImgBase &src){
        return *apply(&src);
      }

      
      /// sets if the image should be clip to ROI or not
      /**
        @param bClipToROI true=yes, false=no
      */    
      void setClipToROI (bool bClipToROI) { 
        m_oROIHandler.setClipToROI(bClipToROI); 
        prop("UnaryOp.clip to ROI").value = bClipToROI ? "on" : "off";
        call_callbacks("UnaryOp.clip to ROI",this);
      }
      
      /// sets if the destination image should be adapted to the source, or if it is only checked if it can be adapted.
      /**
        @param bCheckOnly true = destination image is only checked, false = destination image will be checked and adapted.
      */
      void setCheckOnly (bool bCheckOnly) { 
        m_oROIHandler.setCheckOnly(bCheckOnly); 
        prop("UnaryOp.check only").value = bCheckOnly ? "on" : "off";
        call_callbacks("UnaryOp.check only",this);
      }
      
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
      
      
      /// sets value of a property (always call call_callbacks(propertyName) or Configurable::setPropertyValue)
      virtual void setPropertyValue(const std::string &propertyName, const utils::Any &value) throw (utils::ICLException);
  
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
      static UnaryOp *fromString(const std::string &definition) throw (utils::ICLException);
  
      /// gives a string syntax description for given opSpecifier
      /** opSpecifier must be a member of the list returned by the static function listFromStringOps */
      static std::string getFromStringSyntax(const std::string &opSpecifier) throw (utils::ICLException);
  
      /// returns a list of all supported OP_SPEC values for the fromString function
      static std::vector<std::string> listFromStringOps();
  
      /// creates, applies and releases a UnaryOp defined by given definition string
      static void applyFromString(const std::string &definition, 
                                  const core::ImgBase *src, 
                                  core::ImgBase **dst) throw (utils::ICLException);
      
      protected:
      bool prepare (core::ImgBase **ppoDst, core::depth eDepth, const utils::Size &imgSize, 
                    core::format eFormat, int nChannels, const utils::Rect& roi, 
                    utils::Time timestamp=utils::Time::null){
        return m_oROIHandler.prepare(ppoDst, eDepth,imgSize,eFormat, nChannels, roi, timestamp);
      }
      
      /// check+adapt destination image to properties of given source image
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc) {
        return m_oROIHandler.prepare(ppoDst, poSrc);
      }
      
      /// check+adapt destination image to properties of given source image
      /// but use explicitly given depth
      virtual bool prepare (core::ImgBase **ppoDst, const core::ImgBase *poSrc, core::depth eDepth) {
        return m_oROIHandler.prepare(ppoDst, poSrc, eDepth);
      }
  
      utils::MultiThreader *m_poMT;
      
      private:
    
      OpROIHandler m_oROIHandler;
      
      core::ImgBase *m_buf;
    };    
  
  
  #define DYNAMIC_UNARY_OP_CREATION_FUNCTION(NAME)       \
    extern "C" {                                         \
      UnaryOp *create_##NAME(const std::string &s){      \
        return NAME(s);                                  \
      }                                                  \
    }
    
  } // namespace filter
}


