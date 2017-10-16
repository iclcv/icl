/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/ImgBorder.h                        **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>


namespace icl{
  namespace core{

    /// Class to setup an images border pixels
    /** Border pixels are all pixels, that are not within the images ROI.
        The ImgBorder class provides three different strategies to setup
        these pixels with defined values:

        <h3>fixed</h3>
        This strategy sets all pixels to a fixed value, that is given by
        a 2nd argument to the "fixed"-method. To optimize performace,
        this function does only allow to set up <em>Img<T>-borders</em>
        with <em>T-values</em>. Hence it is realized as a template function.
        <pre>
        ..................        vvvvvvvvvvvvvvvvvv
        ..+-------------+.        vv...............v
        ..|.............|.        vv...............v
        ..|.............|.   -->  vv...............v
        ..|.............|.        vv...............v
        ..+-------------+.        vv...............v
        ..................        vvvvvvvvvvvvvvvvvv
        </pre>

        <h3>copy</h3>
        The copy strategy is an analogue to the IPPs "ippCopyReplicateBorder_.."
        function. It will extrude the images ROI edges towards the images borders.

        <pre>
               ROI                     Img                  Result
        ..................      .......++++++++##.     .....++++++++#####
        ..+-------------+.      .....++++++++####.     .....++++++++#####
        ..|.............|.      ....+++++++######.     ....+++++++#######
        ..|.............|. -->  ....-------######. --> ....-------#######
        ..|.............|.      ....-------######.     ....-------#######
        ..+-------------+.      ....-------####...     ....-------####...
        ..................      ....-------##.....     ....-------####...
        </pre>

        <h3>fromOther</h3>
        The last strategy is to copy the border pixels from another given
        Img.

    */
    class ICLCore_API ImgBorder{
      public:

      /// sets up an images border with a fixed value
      /** @param poImage destination image (with channel count C)
          @param ptVal destination value (for each of the C channels)
      */
      template<class T> ICLCore_API
      static void fixed(Img<T> *poImage, T* ptVal);

      /// sets up an images border by extruding the images ROI pixels towards the image border
      /** @param poImage destination image */
      static void copy(ImgBase *poImage);

      /// copies an images border from another image
      /** @param poImage destination image
          @param poOther source image for the border data. This image must have
                              the same size and channel count as poImage. The function
                              will print a warning and return immediately without any
                              changings otherwise.*/
      static void fromOther(ImgBase *poImage, ImgBase* poOther);

    };
  } // namespace core
}

