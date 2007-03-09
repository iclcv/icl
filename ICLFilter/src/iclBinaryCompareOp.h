#include <iclBinaryOp.h>
#ifndef BINARY_COMPARE_OP_H
#define BINARY_COMPARE_OP_H


namespace icl {
  
  /// Class for comparing two images pixel-wise
  /** Compares pixel values of two images using a specified compare
      operation. The result is written to a binarized image of type Img8u. 
      If the result of the comparison is true, the corresponding output 
      pixel is set to 255; otherwise, it is set to 0.
      */
  class BinaryCompareOp : public BinaryOp {
    public:
#ifdef WITH_IPP_OPTIMIZATION
    /// this enum specifiy all possible compare operations
    enum optype{
      lt   = ippCmpLess,      /*< "<"- relation */
      lteq = ippCmpLessEq,    /*< "<="-relation */
      eq   = ippCmpEq,        /*< "=="-relation */
      gteq = ippCmpGreaterEq, /*< ">="-relation */
      gt   = ippCmpGreater,   /*< ">" -relation */
      eqt                     /*< "=="-relation using a given tolerance level */
    };
#else
    /// this enum specifiy all possible compare operations
    enum optype{
      lt,   /*< "<"- relation */
      lteq, /*< "<="-relation */
      eq,   /*< "=="-relation */
      gteq, /*< ">="-relation */
      gt,   /*< ">" -relation */
      eqt   /*< "=="-relation using a given tolerance level */
    };
#endif
    
    /// creates a new BinaryCompareOp object with given optype and tolerance level
    /** @param ot optype to use 
        @param tolerance tolerance level to use
    **/
    BinaryCompareOp(optype ot, icl64f tolerance=0):
    m_eOpType(ot), m_dTolerance(tolerance){}
    
    /// Destructor
    virtual ~BinaryCompareOp(){}
    
    /// applies this compare operation to two source images into the given destination image
    /** @param poSrc1 first source image
        @param poSrc2 second source image
        @param ppoDst destination image
    **/
    virtual void apply(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);

    /// returns the current optype
    /** @return current optype */
    optype getOpType() const { return m_eOpType; }

    /// returns the current tolerance level
    /** @return current tolerance level */
    icl64f getTolerance() const { return m_dTolerance; }

    /// sets the current opttype
    /** @param ot new optype */
    void setOpType(optype ot) { m_eOpType = ot; }

    /// sets the current tolerance level
    /** @param tolerance new tolerance level*/
    void setTolerance(icl64f tolerance){ m_dTolerance = tolerance; }

    private:
      
    /// internal storage for the current optype
    optype m_eOpType;

    // internal storage for the current tolerance level
    icl64f m_dTolerance;
  };

} // namespace icl

#endif
