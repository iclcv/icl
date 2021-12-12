/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/QImageConverter.h                      **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Robert Haschke                    **
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
#include <ICLCore/Types.h>

// forward declared QImage class
class QImage;

namespace icl{
  namespace qt{

    /// class for conversion between QImage and core::ImgBase/Img\<T\> \ingroup COMMON
    /** The QImageConverter class provides functionality for conversion
        between the QImage class and the core::Img\<T\> classes.
        It provides an internal buffer handling for the destination images,
        so that the user does not have to care about memory handling. The
        user must only take care, that the given image is persistent.

        <h2>Use cases</h2>
        The basic use case is just to convert one Image into another:
        <pre>
        core::ImgBase *i = imgNew(...);
        QImage *q = QImageConverter(i).getImage();
        </pre>
        This will temporarily create a converter object on the stack,
        that converts the given image <em>i</em> into a qimage.
        The opposite direction (QImage to core::ImgBase) behaves identically.
        <b>Note:</b> that the converted image is only persistent as long
        as the QImageConverter object is.

        Another use-case is to optimize performance in a working loop,
        by reusing the same instance of QImageConverter. By writing
        <pre>
        QImageConverter c;
        while(true){
           ...
           core::ImgBase *i = ...
           c.setImage(i);
           QImage *q = q.getQImage();
           ...
        }
        </pre>
        The converter will internally adapt itself to this use-case
        (getting pointers to core::ImgBase objects and returning pointers to
        QImages) that no memory allocation must be performed during the
        iteration. Only if several use cases are performed alternating, it
        might be necessary to allocate and release memory during lifetime.

        <b>Note:</b> If you call setImage(core::Img8u* xxx) before calling
        getImage8u() you will get a <em> copy of the pointer xxx</em>. This
        is essentially, as you will not have a 2nd instance of the image.
    */

    class ICLQt_API QImageConverter{
      public:
      /// creates an empty QImageConverter object
      QImageConverter();

      /// creates a QImageConverter object with given ImgBase
      QImageConverter(const core::ImgBase *image);

      /// creates a QImageConverter object with given QImage
      QImageConverter(const QImage *qimage);

      /// Destructor
      /** if the released object was the last QImageConverter object,
          all static buffers are freed, to avoid large unused memory
      */
      ~QImageConverter();

      /// returns a converted QImage
      /** This function will cause an error if no images were set before.
          Images can be set by calling setImage, setQImage, or by using
          one of the not empty constructors.
      */
      const QImage *getQImage();

      /// returns converted core::ImgBase (of given core::depth")
      /** This function will cause an error if no images were set before.
          Images can be set by calling setImage, setQImage, or by using
          one of the not empty constructors.
      */
      const core::ImgBase *getImgBase(core::depth d=core::depth8u);


      /// template returing an image of given datatype
      template<class T>
      const core::Img<T> *getImg();

      /// sets the current source image of type core::Img8u or Img32f
      /** All further set images get the state "outdated". Hence all later
          <em>getImg[Base]-calls</em> must perform a deep conversion first
      */
      void setImage(const core::ImgBase *image);

      /// sets the current source image of type QImage
      /** All further set images get the state "outdated". Hence all later
          <em>getImg[Base]-calls</em> must perform a deep conversion first
      */
      void setQImage(const QImage *qimage);

      /// sets whether to use speudo colors for grayscale image (default is false)
      /** Right now, speudo colors are only used for Img to QImage conversion
          and for single channel input images */
      void setUseSpeudoColors(bool use);

      private:

      /// internal used state struct
      enum State{ given=0,  /**< this image was given calling setImage  */
                  uptodate=1, /**< this image has already been converted */
                  undefined=2 /**< this image is not defined or <em>outdated</em> */
      };

      /// internal buffer for Imgs of all depths
      core::ImgBase *m_apoBuf[5];

      /// internal qimage buffer
      QImage *m_poQBuf;

      /// internal state buffer (states indicate if images are uptodate, given or outdated
      State m_aeStates[5];

      /// internal state buffer for the QImage buffer
      State m_eQImageState;

      /// use pseudo colors
      bool m_usePC;
    };
  } // namespace qt
}
