/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/WeightedSumOp.h                      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus       **
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

#ifndef WEIGHTEDSUMOP_H
#define WEIGHTEDSUMOP_H

#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>

namespace icl {

   /// Accumulate weighted pixel values of all image channels \ingroup UNARY
   /** Pixels of all channels in source image are weighted 
       by a channel-wise weight and accumulated to the destination 
       image:
       \f[
       D(x,y,0) = \sum\limits_{c=0}^C S(x,y,c)*w(c) 
       \f]
       where \f$D\f$ is the destination image, \f$S\f$ is the source
       image, \f$w\f$ is the weights vector and \f$C\f$ is the source 
       images channel count.
       The result image is created with depth32f by default. Only if
       the source image has a 64Bit floating point depth (depth64f),
       the destination image is adapted to depth64f to avoid loss of
       precession.
       
       
       Performance: 1000x1000 image with 10 channels (1400Mhz Centrino)
       - icl8u: 82ms
       - icl32f: 96ms
       - icl64f: 170ms
       
       Performance: 1000x1000 image with 3 channels (1400Mhz Centrino)
       - icl8u: 21ms
       - icl32f: 25ms
       - icl64f: 47ms
       
  **/
  class WeightedSumOp : public UnaryOp {
    public:
    /// creates a new WeightedSumOp object
    WeightedSumOp(){}
    
    /// creates an new WeightedSumOp object  with a given weights vector
    /** @param weights channel weights vector 
    **/
    WeightedSumOp(const std::vector<icl64f> &weights):
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
    void setWeights(const std::vector<icl64f> &weights){ m_vecWeights = weights; }

    private:
    /// internal storage for the channel weights
    std::vector<icl64f> m_vecWeights;
  };
  
} // namespace icl

#endif
