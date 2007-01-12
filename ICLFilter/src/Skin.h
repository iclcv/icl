/*
  Skin.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef Skin_H
#define Skin_H

#include <Filter.h>

namespace icl {

/// This class implements a Skin color detection algorithm
/**
\section class
The skin class provides access to a skin color segmentation algorithm.
Therefor the class provide a training procedure and a skin detection algorithm.
In a first step the skin parabola parameter have to be trained or if the parameter set is allready known the can be set directly. After this the detection algorithm is well prepared for detection the skin color.
author Michael Götting (mgoettin@techfak.uni-bielefeld.de)
**/

class Skin : public Filter
{
 public:
  Skin () : m_poChromaApply(0), m_poChromaTrain(0) {}
  Skin (std::vector<float> skinParams) : m_poChromaApply(0), 
    m_poChromaTrain(0),
    m_vecSkinParams(skinParams) {}
  ~Skin () {};
  
  ///Start the detection of skin color in the given image.
  /**
     @param poSrc The src image
     @param poDst The final skin color mask (binarized)
  **/
  void apply(icl::ImgBase *poSrc, icl::ImgBase **ppoDst);

  ///Start the training procedure for the skin parabla parameter
  /**
     param poSrc The tarining image. Usually a small skin color patttern
     param poMask Mask the input image with this mask
  **/
  void train(icl::ImgBase *poSrc, icl::ImgBase *poMask = 0);

  ///Set the skin parabola parameter directly 
  /**
     @params The skin color parabola parameter
  **/
  void setParameter(const std::vector<float>& params) {
    m_vecSkinParams = params;}
  
  ///Get the current skin parabola parameter
  /**
     @retrun A vector with the skin parabola parameter
  **/
  const std::vector<float>& getParameter() { return m_vecSkinParams; }

  ///Get the reference Image in chromaticity color space
  /**
     @retrun The chromaticity image
  **/
  ImgBase* getRefChromImg() { return m_poChromaTrain; }

  ///Get the reference Image in chromaticity color space
  /**
     @retrun The chromaticty image
  **/
  ImgBase* getChromImg() { return m_poChromaApply; }
  
 private:
  ImgBase *m_poChromaApply;
  ImgBase *m_poChromaTrain;
  std::vector<float> m_vecSkinParams;
  
}; //class Skin

} //namespace icl

#endif //Skin_H
