// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>

namespace icl::core {
    /// Singelton class that provides access to reusable temporary images
    /** \section GEN General Information
        For global functions as those, provided by the ICLQuick module,
        it is a common problem that functions that return whole images
        always have to allocation new memory for those images. To avoid this,
        a globel function can <em>ask</em> the singelton ImgBuffer instance
        for a temporary image (using one of the ImgBuffer::get functions).
        This will return a pointer to an image whose memory is managed by the
        ImgBuffer class.

        \section HOW How Appropriate Images are Found
        ICL Images internally use std::shared_ptr instances to manage their channel
        data pointers. ImgBase instances can be <em>asked</em> whether they
        are <em>independent</em>, i.e. no other image does currently share
        the image data.

        The ImgBuffer does internally manage a list potentially reusable
        Img-Instances for each possible image depth.

        \subsection GET_1 get(void)
        If ImgBuffer::get<T>(void) is called, the ImgBuffer searches in the
        current buffer-list for depth-T for an image that is currently
        independent, i.e, the ImgBuffer itself holds the only reference to the
        images memory. If an independent image can be found, it is returned,
        otherwise a new image is added to the ImgBuffers internal buffer list
        for depth-T.

        \subsection GET_2 get(params)
        If ImgBuffer::get<T>(const ImgParams&) or
        ImgBuffer::get<T>(const Size&,int) is called, the ImgBuffer will first
        search for an image that is not only independent but that already has
        the desired parameters. If a compatible image is found, it is returned.
        Otherwise, an independent image is preferred. If an independent image
        can be found, it's parameters are adapted to the desired parameters.
        Otherwise, a new image with the desired parameters is created, stored
        and returned.
    */
    class ICLCore_API ImgBuffer {
      struct Data; //!< internal data storage class
      Data *data;  //!< internal data storage pointer
      ImgBuffer(); //!< private contructor -> use static function instance to get the singelton instance
      public:
      ImgBuffer(const ImgBuffer&) = delete;
      ImgBuffer& operator=(const ImgBuffer&) = delete;


      /// Destructor
      ~ImgBuffer();

      /// Obtain the singelton instance of ImgBuffer
      static ImgBuffer *instance();

      /// returns an independent image if available, otherwise, a new one is added
      template<class T> ICLCore_API
      Img<T> *get();

      /// returns an independent image with preferably correct parameters or a new one
      template<class T> ICLCore_API
      Img<T> *get(const utils::Size &size, int channels);

      /// returns an independent image with preferably correct parameters or a new one
      template<class T> ICLCore_API
      Img<T> *get(const ImgParams &params);

      /// non template version for get(void)
      ImgBase *get(depth d);

      /// non template version for get(const Size&,channels)
      ImgBase *get(depth d, const utils::Size &size, int channels);

      /// non template version for get(const ImgParams&)
      ImgBase *get(depth d, const ImgParams &p);
    };

  } // namespace icl::core