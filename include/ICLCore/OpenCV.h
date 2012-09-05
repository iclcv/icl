/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCore/OpenCV.h                               **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Christian Groszewski              **
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

#pragma once

#ifndef HAVE_OPENCV
#warning "this header should not be included if HAVE_OPENCV is not defined"
#else
#include <opencv/cxcore.h>

#include <ICLCore/CCFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/ImgBase.h>

namespace icl{
  namespace core{
  
    /// Modes that define whether to prefer the source image's or the destination image's depth
    enum DepthPreference{
      PREFERE_SRC_DEPTH, //!< prefer source depth
      PREFERE_DST_DEPTH  //!< prefer destination depth
    };
  
    ///Convert OpenCV IplImage to ICLimage
    /**Converts IplImage to ICLimage. If dst is NULL, the sourceimagedepth
        will be used, else the destinationimagedepth will be used.
        @param *src pointer to sourceimage (IplImage)
        @param **dst pointer to pointer to destinationimage (ICLimage)
        @param e depthpreference*/
    ImgBase *ipl_to_img(CvArr *src,ImgBase **dst=0,DepthPreference e=PREFERE_SRC_DEPTH) throw (utils::ICLException);
  
    ///Convert ICLimage to OpenCV IplImage
    /**Converts ICLimage to IplImage. If dst is NULL, the sourceimagedepth
        will be used, else the destinationimagedepth will be used.
        @param *src pointer to sourceimage
        @param **dst pointer to pointer to destinationimage (IplImage)
        @param e depthpreference*/
    IplImage *img_to_ipl(const ImgBase *src, IplImage **dst=0,DepthPreference e=PREFERE_SRC_DEPTH)throw (utils::ICLException);
  
    ///Copy single ICLimage channel to OpenCV single channel CvMat
    /**Copy single ICLimage channel to single channel CvMat. If dst is NULL, the sourceimagedepth
        will be used, else the destinationmatrixdepth will be used.
        @param *src pointer to sourceimage
        @param **dst pointer to pointer to destinationmatrix
        @param channel channel to use*/
    CvMat* img_to_cvmat(const ImgBase *src, CvMat *dst=0,int channel=0) throw (utils::ICLException);
  
    ///Convert single channel ICLimage to OpenCV IplImage
    /**Converts single channel ICLimage to IplImage. Data is shared by source and destination.
        Using icl8u or icl16s the imagedata is not aligned, but OpenCV expects aligned data.
        In this case be careful using further OpenCV functions.
        Be careful when releasig (data)pointers.
        @param *src pointer to sourceimage
        @param *dst pointer to destinationimage (IplImage)*/
    IplImage *img_to_ipl_shallow(ImgBase *src,IplImage *dst=0)throw (utils::ICLException);
  
    ///Convert single channel ICLimage to OpenCV CvMat
    /**Converts single channel ICLimage to a single channel CvMat. Data is shared by
        source and destination.
        Be careful when releasig (data)pointers.
        @param *src pointer to sourceimage
        @param *dst pointer to destinationmatrix (IplImage)*/
    CvMat *img_to_cvmat_shallow(const ImgBase *src,CvMat *dst=0) throw (utils::ICLException);
  } // namespace core
}
#endif


