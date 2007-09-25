#ifndef ICL_SkinOp_H
#define ICL_SkinOp_H

#include <iclUnaryOp.h>
#include <iclUncopyable.h>
/*
  Skin.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


namespace icl {

/// This class implements a Skin color detection algorithm \ingroup UNARY
/**
    \section Overview
    The skin class provides access to a skin color segmentation algorithm.
    Therefor the class provide a training procedure and a skin detection algorithm.
    In a first step the skin parabola parameter have to be trained or if the parameter set 
    is allready known the can be set directly. After this the detection algorithm is well 
    prepared for detection the skin color.
    
    author Michael Götting (mgoettin@techfak.uni-bielefeld.de)
**/
class SkinOp : public UnaryOp, public Uncopyable{
  public:
  ///Constructor
  SkinOp () : m_poChromaApply(0), m_poChromaTrain(0) {}
    
  ///Constructor
  SkinOp (std::vector<float> skinParams) : m_poChromaApply(0), 
    m_poChromaTrain(0),
    m_vecSkinParams(skinParams) {}
  ///Destructor
  ~SkinOp () {};
  
  ///Start the detection of skin color in the given image.
  /**
     @param poSrc The src image
     @param ppoDst The final skin color mask (binarized)
  **/
  void apply(const icl::ImgBase *poSrc, icl::ImgBase **ppoDst);

  ///Same function as apply() but image values written as csv file
  /**
     @param poSrc The src image
     @param ppoDst The final skin color mask (binarized)
     @param ppoPara1 The final skin color mask (prepared for function plot)
     @param ppoPara2 The final skin color mask (prepared for function plot)
     @param ppoChromaVal The final skin color mask (prepared for function plot)
     @param ppoChromaSkinMask The final skin color mask (prepared for function plot)
  **/
  void apply(const icl::ImgBase *poSrc, icl::ImgBase **ppoDst,
             icl::ImgBase **ppoPara1, icl::ImgBase **ppoPara2,
             icl::ImgBase **ppoChromaVal, icl::ImgBase **ppoChromaSkinMask);

  ///Start the training procedure for the skin parabla parameter
  /**
     param poSrc The tarining image. Usually a small skin color patttern
     param poMask Mask the input image with this mask
  **/
  void train(const icl::ImgBase *poSrc, const icl::ImgBase *poMask = 0);

  ///Set the skin parabola parameter directly 
  /**
     @param params The skin color parabola parameter
  **/
  void setParameter(const std::vector<float>& params) {
    m_vecSkinParams = params;}
  
  ///Get the current skin parabola parameter
  /**
     @return A vector with the skin parabola parameter
  **/
  const std::vector<float>& getParameter() { return m_vecSkinParams; }

  ///Get the reference Image in chromaticity color space
  /**
     @return The chromaticity image
  **/
  ImgBase* getRefChromImg() { return m_poChromaTrain; }

  ///Get the reference Image in chromaticity color space
  /**
     @return The chromaticty image
  **/
  ImgBase* getChromImg() { return m_poChromaApply; }
  
 private:
  ImgBase *m_poChromaApply;
  ImgBase *m_poChromaTrain;
  std::vector<float> m_vecSkinParams;
  
}; //class Skin

} //namespace icl

#endif //Skin_H
