/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/WeightChannelsOp.h                   **
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
*********************************************************************/

#ifndef WEIGHTCHANNELSOP_H
#define WEIGHTCHANNELSOP_H

#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>

namespace icl {

   /// Weight pixel values of all image channels \ingroup UNARY
   /** Pixels of all channels in source image are weighted 
       by a channel-wise weight:
       \f[
       D(x,y,c) = S(x,y,c)*w(c) 
       \f]
       where \f$D\f$ is the destination image, \f$S\f$ is the source
       image, \f$w\f$ is the weight vector and \f$C\f$ is the source 
       images channel count.

       Performance: 1000x1000 image with 10 channels averaged over 100 runs (1,83 GHz Core Duo)
       - icl8u: 34ms
       - icl32f: 37ms
       - icl64f: 69ms
       
       Performance: 1000x1000 image with 3 channels averaged over 100 runs (1,83 GHz Core Duo)
       - icl8u: 10.9ms
       - icl32f: 10.9ms
       - icl64f: 21ms
   **/
  class WeightChannelsOp : public UnaryOp {
  public:
    /// creates a new WeightChannelsOp object
    WeightChannelsOp(){}
    
    /// creates an new WeightChannelsOp object  with a given weights vector
    /** @param weights channel weights vector 
     **/
    WeightChannelsOp(const std::vector<icl64f> &weights):
      m_vecWeights(weights){}
    
    /// applies this operation on the source image
    /** @param poSrc source image
        @param ppoDst destination image (adapted to icl32f by default,
        if the source image has depth64f, ppoDst is adapted to
        icl64f too.
        
    **/
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    

    /// returns the current weight vector
    /** @return reference to the current weight vector **/
    const std::vector<icl64f> &getWeights() const { return m_vecWeights; }
    
    /// sets up the current weights vector
    /** @param weights new weight vector **/
    void setWeights(const std::vector<icl64f> &weights) { 
      m_vecWeights = weights; }
    
  private:
    /// internal storage for the channel weights
    std::vector<icl64f> m_vecWeights;
  };
  
} // namespace icl

#endif
