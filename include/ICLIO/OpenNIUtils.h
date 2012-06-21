/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/OpenNIUtils.h                            **
** Module : ICLIO                                                  **
** Authors: Viktor Richter                                         **
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

#ifndef ICL_OPENNI_UTILS_H
#define ICL_OPENNI_UTILS_H

#include <XnOS.h>
#include <XnCppWrapper.h>

namespace icl {

  /// Utility Structure
  /**
   * This struct is used to initialize and release the OpenNI Context.
   * It intializes an xn::Context on creation and releases it on destruction.
   * Uses a static counter to ensure initialization only on first and
   * release on last call.
   */
  struct OpenNIAutoContext{
    public:
      /// Initializes OpenNI context if not already done.
      OpenNIAutoContext();

      /// Releases OpenNI context when (calls to term) == (calls to init).
      ~OpenNIAutoContext();

      /// Initializes the OpenNI context.
      /** @return whether Context.init() actually was called. */
      static bool initOpenNIContext();

      /// releases the OpenNI context.
      /** @return whether Context.release() actually was called. */
      static bool releaseOpenNIContext();

      /// returns a pointer to the OpenNI context.
      xn::Context* getContextPtr();
  };

  /// abstract super-class of all Image generators
  class OpenNIImageGenerator {
    public:

    /// an enum listing all supported data generators
    enum Generators {
      RGB,
      DEPTH,
      NOT_SPECIFIED = -1
    };

    /// grab function grabs an image
    virtual const ImgBase* acquireImage() = 0;
    /// tells the type of the Generator
    virtual Generators getType() = 0;

    ///  Creates the corresponding Generator.
    static OpenNIImageGenerator* createGenerator(xn::Context* context,
                                                 Generators type);
  };

  /// Depth Image Generator
  class OpenNIDepthGenerator : public OpenNIImageGenerator {
    public:
      /// Creates a DepthGenerator from Context
      OpenNIDepthGenerator(xn::Context* context);
      /// Destructor frees all resouurces
      ~OpenNIDepthGenerator();

      /// grab function grabs an image
      virtual const ImgBase* acquireImage();
      /// tells the type of the Generator
      virtual Generators getType();

    private:
      /// the OpenNI context
      xn::Context* m_Context;
      /// the underlying depth generator
      xn::DepthGenerator* m_DepthGenerator;
      /// a DepthMetaData object holding image information
      xn::DepthMetaData m_DepthMD;
      /// pointer to internally used image
      Img16s* m_Image;
  };

  /// RGB Image Generator
  class OpenNIRgbGenerator : public OpenNIImageGenerator {
    public:
      /// Creates a RgbGenerator from Context
      OpenNIRgbGenerator(xn::Context* context);
      /// Destructor frees all resouurces
      ~OpenNIRgbGenerator();

      /// grab function grabs an image
      virtual const ImgBase* acquireImage();
      /// tells the type of the Generator
      virtual Generators getType();

    private:
      /// the OpenNI context
      xn::Context* m_Context;
      /// A NodeInfo for the used device
      xn::NodeInfo* m_DeviceInfo;
      /// the underlying rgb-image generator
      xn::ImageGenerator* m_RgbGenerator;
      /// a ImagehMetaData object holding image information
      xn::ImageMetaData m_RgbMD;
      /// pointer to internally used image
      Img8u* m_Image;
  };

} //namespace icl

#endif

