#ifndef PROXIMITY_OP_H
#define PROXIMITY_OP_H

#include <iclBinaryOp.h>
#include <iclNeighborhoodOp.h>
#include <iclImg.h>
#include <iclUncopyable.h>

namespace icl {
  
  /// Class for computing proximity measures  \ingroup BINARY
  /** (Only available for Img8u and Img32f, IPP only!)
      \section OV Overview (taken from the IPPI-Manual)

      "The functions described in this section compute the proximity (similarity) measure between an
      image and a template (another image). These functions may be used as feature detection functions,
      as well as the components of more sophisticated techniques.
      There are several ways to compute the measure of similarity between two images. One way is to
      compute the Euclidean distance, or sum of the squared distances (SSD), of an image and a
      template. The smaller is the value of SSD at a particular pixel, the more similarity exists between
      the template and the image in the neighborhood of that pixel."
      
      The ProximityOp class summarizes these image similarity measurement techniques 
      and provides their functionality by implementing the ICLFilter packages BinaryOp 
      interface.\n
      There are two different variables, that influencing the internal functionality of 
      the ProximityOps apply function.
      
      \section AM ApplyMode
      The first variable - the so called "applymode" - determines in which region of the
      source image a specific proximity measure is applied. The following ASCII image 
      describes the differences between the values "full", "valid" and "same"

      <pre>
      Image: iiiiiiiiiiiiii   Mask: mmmmm       resulting images:
             iiiiiiiiiiiiii         mmxmm
             iiiiiiiiiiiiii         mmmmm       +---+    
             iiiiiiiiiiiiii        (5x3)        |   | := original image area
             iiiiiiiiiiiiii                     +---+
              (14 x 5)
      
      full: mmmmm                               full result:   
            mmx<--- fist pos.                      rrrrrrrrrrrrrrrr  
            mmmmmiiiiiiiiiiiii                     r+------------+r
                iiiiiiiiiiiiii                     r|rrrrrrrrrrrr|r  
                iiiiiiiiiiiiii                     r|rrrrrrrrrrrr|r
                iiiiiiiiiiiiii                     r|rrrrrrrrrrrr|r
                iiiiiiiiiiiiimmmmm                 r+------------+r
                             mmx<-- last pos.      rrrrrrrrrrrrrrrr
                             mmmmm
      same:                                      same result:
              mmmmm              
       first: mmxmmiiiiiiiiiii                     +------------+ 
              mmmmmiiiiiiiiiii                     |rrrrrrrrrrrr|
                iiiiiiiiiiiiii                     |rrrrrrrrrrrr|
                iiiiiiiiiiimmmmm                   |rrrrrrrrrrrr|
                iiiiiiiiiiimmxmm <-- last          +------------+
                           mmmmm

      valid:                                     valid result:
                mmmmmiiiiiiiii                     +------------+ 
         first: mmxmmiiiiiiiii                     | rrrrrrrrrr |
                mmmmmiiiimmmmm                     | rrrrrrrrrr |
                iiiiiiiiimmxmm  <-- last           | rrrrrrrrrr |
                iiiiiiiiimmmmm                     +------------+
                                 
      
      </pre>

      \section OP Operation Type
      This time three different metrics for the similarity measurements
      are implemented (IPP Only)
      
      The formulas can be found in the ippi-manual!
      
      optypes:
      - sqrDistance
      - crossCorr
      - crossCorrCoeff
     
      
  */
  class ProximityOp : public BinaryOp, public Uncopyable{
    public:
    
    /// enum to specify the current apply mode of a ProximityOp
    /** @see ProximityOp */
    enum applymode{
      full, /**< destination image has size (w1+w2-1)x(h1+h2-1) */
      same, /**< destination image has size (w1)x(h1)           */
      valid /**< destination image has size (w1-w2+1)x(h1-h2+1) */
    };

    /// enum to specify the current operation type of a ProximityOp
    /** @see ProximityOp */
    enum optype{
      sqrDistance,   /**< square distance metric               */
      crossCorr,     /**< cross correlation metric             */ 
      crossCorrCoeff /**< cross correlation coefficient metric */
    };

    /// Creates a new ProximityOp object with given apply mode and optype
    /** @param ot optype for the ProximityOp 
        @param am apply mode for the ProximityOp (default = "valid")
    **/
    ProximityOp(optype ot, applymode am=valid):
    m_eOpType(ot),m_eApplyMode(am),m_poImageBuffer(0),m_poTemplateBuffer(0){}

    /// Destructor
    virtual ~ProximityOp(){
      if(m_poImageBuffer) delete m_poImageBuffer;
      if(m_poTemplateBuffer) delete m_poTemplateBuffer;
    }

    /// applies the current op given source image, template and destination image
    /** allowed input image types are icl8u and icl32f other types are converted internally
        to float images. The destination image is adapted automatically; it depth becomes 
        depth32f.        
        @param poSrc1 source image 
        @param poSrc2 template 
        @param ppoDst destination image (apated automatically)        
    **/
    virtual void apply(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);

    /// import apply symbol from parent class
    BinaryOp::apply;

    /// sets the current optype
    /** @param ot new optype **/
    void setOpType(optype ot){ m_eOpType = ot; }
    
    /// sets the current applymode
    /** @param am new applymode value **/
    void setApplyMode(applymode am) { m_eApplyMode = am; }
    
    /// returns the current optype
    /** @return current optype **/
    optype getOpType() const { return m_eOpType; }

    /// returns the current applymode
    /** @return current applymode **/
    applymode getApplyMode() const { return m_eApplyMode; }

    private:
    
    /// internal storage for the current optype
    optype m_eOpType;
    
    /// internal storage for the current applymode
    applymode m_eApplyMode;   
    
    /// internal used buffer for handling unsupported formats
    Img32f *m_poImageBuffer;

    /// internal used buffer for handling unsupported formats
    Img32f *m_poTemplateBuffer;
  };
} // namespace icl
#endif


