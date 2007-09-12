#ifndef ICL_UNARY_OP_PIPE_H
#define ICL_UNARY_OP_PIPE_H
#include <vector>
#include <iclUnaryOp.h>
namespace icl{
  
  /** \cond */
  class ImgBase;
  /** \endcond */
  
  /// Utility class that helps applying UnaryOps one after another \ingroup UNARY
  /** Consider a default computer vision system, which has some
      preprocessing steps in the following operation order:
      -# resize the image to a given size
      -# calculate a feature map (e.g. a weighted sum of the color channels)
      -# binarize the feature map
      -# apply some morphological operation on this feature map
      
      To facilitate this, you can easily use a UnaryOpPipe, which
      internally creates a queue of UnaryOp instances and their
      individual result images (as ImgBase*). Once you have created
      this Pipe under a certain name (e.g. "MyProprocessor") you
      can easily add or remove specific preprocessing steps, without
      having to consider fall-outs elsewhere in your code.
      
      Take a look on the following example:
      \code
      #include <iclQuick.h>
      #include <iclUnaryOpPipe.h>
      #include <iclScaleOp.h>
      #include <iclWeightedSumOp.h>
      #include <iclUnaryCompareOp.h>
      #include <iclMorphologicalOp.h>
      
      int main(){
        // create an input image (the nice parrot here!)
        ImgQ inputImage = create("parrot");
      
        // create a weight vector (used later on by an instance of the WeightedSumOp class)
        vector<icl64f> weights(3); weights[0] = 0.2; weights[1] = 0.5; weights[0] = 0.3;
      
        // create a dilation mask (used later on by an instance of the MorphologicalOp class)
        char mask[9] = {1,1,1,1,1,1,1,1,1};
      
        // create the empty pipe
        UnaryOpPipe pipe;
      
        // add the UnaryOp's in the correct order
        pipe.add(new ScaleOp(0.5,0.5));
        pipe.add(new WeightedSumOp(weights));
        pipe.add(new UnaryCompareOp(UnaryCompareOp::lt,128));
        pipe.add(new MorphologicalOp(Size(3,3),mask,MorphologicalOp::dilate3x3));
      
        // apply this pipe on the source image (use the last image as destination)
        pipe.apply(&inputImage,&pipe.getLastImage());
      
        // show the result
        show(cvt(pipe.getLastImage()));
      
        return 0;
      }
      \endcode
  **/
  class UnaryOpPipe : public UnaryOp{
    public:
    /// create an empty pipe
    UnaryOpPipe();
    
    /// Destructor
    ~UnaryOpPipe();
    
    /// add a new op to the end of this pipe
    void add(UnaryOp *op, ImgBase*im=0);
    
    /// applies all ops sequentially 
    virtual void apply(const ImgBase *src, ImgBase **dst);
    
    /// returns the number of contained ops
    int getLength() const;
    
    /// returns the op at given index
    UnaryOp *&getOp(int i);
    
    /// returns the result buffer image at given index
    ImgBase *&getImage(int i);
    
    /// returns the last image (which is not used by default)
    /** This image is only used, if it is given a 2nd parameter to
        the apply function
        \code
        MyPipe.apply(mySourceImagePtr,&(MyPipe.getLastImage());
        \endcode
    **/
    ImgBase *&getLastImage();  
    
    private:
    /// Internal buffer of ops
    std::vector<UnaryOp*> ops;
    
    /// Internal buffer of result images
    std::vector<ImgBase*> ims;
  };
}
#endif
