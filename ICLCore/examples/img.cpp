/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/examples/img.cpp                               **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCore/Img.h>
#include <algorithm>

using namespace icl::utils;
using namespace icl::core;

int main(){
   // create an empty image with byte pixels
   Img8u byteImage;

   // create float image with VGA size with 3-channel RGB-format
   Img32f floatImage(Size(640,480),formatRGB);

   // set all pixels of channel 0 to 255
   std::fill(floatImage.begin(0),floatImage.end(0),255);

   // scale down the float image
   floatImage.scale(Size(320,240));

   // convert the float image into the former byteImage
   // the byte image is adapted automatically
   floatImage.convert(&byteImage);
   
   // create a shallow copy of 'byteImage'
   Img8u byteImage2 = byteImage;

   // byteImage2 and byteImage now share their image data, so
   // changes on byteImage2s pixel-data will also effect the pixel-data
   // of byteImage. The image can be made independent by doing this:
   byteImage.detach();
   
   // now lets access the image pixels for a simple thresholding application
   // note: the image pixel data is organized channel-wise as independend 
   // row-major ordered data blocks 
   for(int c=0;c<byteImage.getChannels();++c){
      for(int x=0;x<byteImage.getWidth();++x){
         for(int y=0;y<byteImage.getHeight();++y){
            byteImage(x,y,c) = 255 * (byteImage(x,y,c)>128);
         } 
      }
   }
   
   /// copy an image ROI
   Img8u byteImage3(Size(1000,1000),formatRGB);
   byteImage3.setROI(Rect(100,100,300,200));
   byteImage2.setROI(Rect(0,0,300,200));
   byteImage2.deepCopyROI(&byteImage3);
}
