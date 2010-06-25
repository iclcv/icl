/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLFilter/WarpOp.h                             **
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

#ifndef ICL_WARP_OP_H
#define ICL_WARP_OP_H

#include <ICLFilter/UnaryOp.h>
#include <ICLCore/Img.h>

namespace icl{

  /// Operator that remaps an image with given look-up map
  /** \section OV Overview
      A 'Warping' operation on images is any operation, that works
      on the local domain of the image i.e. it moves the images
      pixel locations. Special warping routines like affine operations
      (see AffineOp) can be performed using a functional rule that 
      is applied on each destination pixel to determine it's corresponding
      source pixels (using the AffineOp example again, this function
      might be e.g. an affine matrix multiplication).
      If the mapping function gets more complex (e.g. in case of camera
      lens distortion), computation might become too slow, however
      computational performance even for most complex mappings can be 
      limited by pre-calculating a 
      so-called warp-table (a 2-channel 2D-look-up-table, that contains
      the result of the mapping for each pixel). Once having obtained 
      such a LUT, a WarpOp will help to apply the table lookup operation
      conveniently and safely.
      
      \section ROI ROI-Support
      Currently this Op does not provide ROI handling (although used
      IPP-functions do)
      
      \section IPP IPP-Support
      Support is purely optional and only defined in case of depth8u
      or depth32f input images
      
      \section PERF Performance
      As already mentioned, the operation performance does not depend
      on the mapping function at all. Hence there're only few parameters,
      that influence the apply time of a WarpOp instance:
      - image depth (we expect icl8u's to be a bit faster then the other types)
      - image size (warping is linear wrt the number of image pixels)
      - interpolation method

      Here's a short list of benchmarks. For benchmarking your system, 
      you can use the icl-warp-op-test application.
      <pre>
      System: 2.0 GHz Core2Duo
      Size:   640x480 (VGA)
      Build-flags: -O4 -march=native -funroll-loops
      
       interpolation:   NN             LINEAR
      --------------------------------------------
      - depth8u:       8ms(IPP)       13ms(IPP)
      - depth16s:      42ms           79ms
      - depth32s:      46ms           79ms
      - depth32f:      13ms(IPP)      20ms(IPP)
      - depth64f:      54ms           92ms

      </pre>

   */
  class WarpOp : public UnaryOp{
    public:
    
    /// create a new WarpOp instance
    /** This constructor has been made explicit to avoid ambiguity in case of
        calling WarpOp(ImgQ()) or something like that.
        @param warpMap map that contains new x-coortinates in the first channel
                           and new y-coordinates in the 2nd one
        @param mode interpolation mode either interpolateLIN or interpolateNN
        @param allowWarpMapScaling if set to true, the WarpOp instance will
               internally create a scaled warp map in case of differing warp map
               and image size
    **/
    explicit WarpOp(const Img32f &warpMap=Img32f(), 
                    scalemode mode=interpolateLIN,
                    bool allowWarpMapScaling=true);
    
    /// Sets a new scalemode (either interpolateLIN or interpolateNN)    
    void setScaleMode(scalemode scaleMode);

    /// Sets a new warp map
    void setWarpMap(const Img32f &warpMap);

    /// Sets the allow warp-map-scaling features
    /** @see WarpOp(const Img32f&,scalemode,bool)*/
    void setAllowWarpMapScaling(bool allow);

    /// returns the current scalemode
    scalemode getScaleMode() const { return m_scaleMode; }

    /// returns the current warp map
    const Img32f &getWarpMap() const { return m_warpMap; }

    /// returns whether warp map scaling is allowed
    bool getAllowWarpMapScaling() const { return m_allowWarpMapScaling; }
    
    /// virtual apply function
    virtual void apply(const ImgBase *src, ImgBase **dst);
    
    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    private:
    bool m_allowWarpMapScaling;
    Img32f m_warpMap;
    Img32f m_scaledWarpMap;
    scalemode m_scaleMode;
  };


}

#endif
