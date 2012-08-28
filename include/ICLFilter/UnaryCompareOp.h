/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/UnaryCompareOp.h                     **
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

#ifndef UNARY_COMPARE_OP_H
#define UNARY_COMPARE_OP_H

#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{
    
     /// Class for comparing operations  \ingroup UNARY
     /** This class Compares each pixelvalue of an image with a constant value
         using a specified compare operation. The result is written to a
         binarized image of type Img8u. If the result of the comparison is true,
         the corresponding output pixel is set to 255; otherwise, it is set to 0.
     */
    class UnaryCompareOp : public UnaryOp {
      public:
   
  #ifdef HAVE_IPP
      /// this enum specifiy all possible compare operations
      enum optype{
        lt   = ippCmpLess,      /**< "<"- relation */
        lteq = ippCmpLessEq,    /**< "<="-relation */
        eq   = ippCmpEq,        /**< "=="-relation */
        gteq = ippCmpGreaterEq, /**< ">="-relation */
        gt   = ippCmpGreater,   /**< ">" -relation */
        eqt                     /**< "=="-relation using a given tolerance level */
      };
  #else
      /// this enum specifiy all possible compare operations
      enum optype{
        lt,   /**< "<"- relation */
        lteq, /**< "<="-relation */
        eq,   /**< "=="-relation */
        gteq, /**< ">="-relation */
        gt,   /**< ">" -relation */
        eqt   /**< "=="-relation using a given tolerance level */
      };
  #endif
      
      /// translate a given relation into an optype
      /** - UnaryCompareOp::lt -> "<" 
          - UnaryCompareOp::gt -> ">" 
          - UnaryCompareOp::lteq -> "<=" 
          - UnaryCompareOp::gteq -> ">=" 
          - UnaryCompareOp::eq -> "==" 
          - UnaryCompareOp::eqt -> "~=" 
      */
      static optype translate_op_type(const std::string &stringVersion) throw (utils::ICLException){
        if(stringVersion == "<") return UnaryCompareOp::lt;
        if(stringVersion == ">") return UnaryCompareOp::gt;
        if(stringVersion == "<=") return UnaryCompareOp::lteq;
        if(stringVersion == ">=") return UnaryCompareOp::gteq;
        if(stringVersion == "==") return UnaryCompareOp::eq;
        if(stringVersion == " ~=") return UnaryCompareOp::eqt;
        throw utils::ICLException("UnaryCompareOp::translate_op_type(" + stringVersion + "): invalid optype string!");
        /// satisfy the compiler :-)
        return UnaryCompareOp::lt;
      }
      
      /// Creates a new UnaryCompareOp object with given optype, value and tolerance level
      /** @param ot operation type ("<","<=",...)
          @param value value to compare each pixel with
          @param tolerance tolerance level (only of optype==eqt)
      **/
      UnaryCompareOp(optype ot=gt, icl64f value=128, icl64f tolerance=0):
      m_eOpType(ot), m_dValue(value), m_dTolerance(tolerance){ }
  
      /// creates a new UnaryCompareOp instance with given parameters
      /** The optype parameters is here given as a string 
          @see translate_op_type(const std::string&)*/
      UnaryCompareOp(const std::string &op, icl64f value=128, icl64f tolerance=0):
      m_eOpType(translate_op_type(op)), m_dValue(value), m_dTolerance(tolerance){ }
      
      /// Destructor
      virtual ~UnaryCompareOp(){}
      
      /// sets the current optype
      /** @param ot new optype value */
      void setOpType(optype ot){ m_eOpType = ot; }
  
      /// sets the current compare value
      /** @param value new compare value */
      void setValue(icl64f value){ m_dValue = value; }
      
      /// sets the current tolerance level
      /** @param tolerance new tolerance level */
      void setTollerance(icl64f tolerance){ m_dTolerance = tolerance; }
      
      /// returns the current optype
      optype getOpType() const { return m_eOpType; }
      
      /// returns the current compare-value
      /** @return current value */
      icl64f getValue() const { return m_dValue; }
  
      /// returns the current tolerance level
      /** @return current tolerance level */
      icl64f getTolerance() const { return m_dTolerance; }
      
      /// applies the operation to a source image 
      /** @param poSrc source image 
          @param ppoDst destination image 
      **/
      virtual void apply(const core::ImgBase *poSrc, core::ImgBase **ppoDst);
      
      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;
      private:
      
      /// internal storage of the current optype
      optype m_eOpType;
      
      /// internal storage of the current value
      icl64f m_dValue;
  
      /// internal storage of the current tolerance level
      icl64f m_dTolerance;
    };
  } // namespace filter
} // namespace icl

#endif
