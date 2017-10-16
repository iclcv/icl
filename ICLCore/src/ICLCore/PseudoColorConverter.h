/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/PseudoColorConverter.h             **
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
#include <ICLCore/Color.h>
#include <ICLCore/Img.h>

namespace icl{
  namespace core{

    /// Utility class for speudocolor conversion
    /** The PseudoColorConverter converts a given 1-channel image into an RGB pseudocolor image.
        It can be set up to use a default color-table or it can be set up using a Stop-based
        piecewise linear color table as internal convert lookup table */
    struct ICLCore_API PseudoColorConverter{
      /// mode internally used
      enum ColorTable{
        Default,         //!< default mode using default gradient
        Custom           //!< custom mode using a custom gradient
      };

      /// This is for creation of color gradients
      struct ICLCore_API Stop{
        Stop(float relPos=0, const Color &color=Color(0,0,0)):
        relPos(relPos),color(color){}
        float relPos; //!< relative pos [0,1]
        Color color;  //!< color
      };

      /// creates instance with default color table
      /** The maxValue parameter defines the maximum expected value found in input images */
      PseudoColorConverter(int maxValue=255);

      /// creates instance with custom mode
      /** @see setColorTable */
      PseudoColorConverter(const std::vector<Stop> &stops, int maxValue=255) throw (utils::ICLException);

      /// sets the color table mode
      /** if mode is default, stops must be empty
          If the custom mode is used, stops is used and internally stored. Stops are
          sorted automatically by there relative position. The minimum  allowed relative
          position is 0, the maximum allowed relative position is 1. If these bounds are
          exceeded, an ICLException is thrown. If the first sorted stop has not relative
          position 0.0, an extra stop is prepended with relative position 0.0 and black
          color (r=g=b=0). If the last sorted stop's position is not 1.0, an extra stop
          is appended with absolute position 1.0 and white color (r=g=b=255).
          The stops list must at least contain one stop. If only one stop is contained,
          all pixels are set to the given color. However, there are simpler functions that
          have the same result and are less complex (like Img<T>::clear(channelIndx,color))
      */
      void setColorTable(ColorTable t, const std::vector<Stop> &stops=std::vector<Stop>(), int maxValue=255)
        throw (utils::ICLException);

      /// create a speudo color image from given source image
      void apply(const ImgBase *src, ImgBase **dst) throw (utils::ICLException);

      /// create a speudo color image from given source image
      void apply(const Img8u &src, Img8u &dst);

      /// create a speudo color image from given source image (using an internal buffer)
      const Img8u &apply(const Img8u &src);

      /// writes current stop configuration to xml-configuration file with given name
      void save(const std::string &filename);

      /// loads new stop configuration from given xml-configuration file
      void load(const std::string &filename);

      private:

      /// Internal data structure
      struct Data;

      /// Internally used data pointer
      Data *m_data;
    };

  } // namespace core
}

