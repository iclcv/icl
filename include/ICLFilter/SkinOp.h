/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/SkinOp.h                             **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Michael GÃ¶tting, Robert Haschke,  **
**          Andre Justus                                           **
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

#ifndef ICL_SkinOp_H
#define ICL_SkinOp_H

#include <ICLFilter/UnaryOp.h>
#include <ICLUtils/Uncopyable.h>
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
  ///Empty Constructor
  SkinOp () : m_poChromaApply(0), m_poChromaTrain(0) {}
  
  ///Destructor
  ~SkinOp () {};
  
  ///Start the detection of skin color in the given image.
  /**
     @param poSrc The src image
     @param ppoDst The final skin color mask (binarized)
  **/
  void apply(const icl::ImgBase *poSrc, icl::ImgBase **ppoDst);

  /// Import unaryOps apply function without destination image
  UnaryOp::apply;


???
































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
