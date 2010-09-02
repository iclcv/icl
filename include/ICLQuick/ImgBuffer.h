/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQuick/ImgBuffer.h                           **
** Module : ICLQuick                                               **
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

#ifndef ICL_IMG_BUFFER_H
#define ICL_IMG_BUFFER_H

#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>

namespace icl{
  
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
      ICL Images internally use SmartPtr instances to manage their channel
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
  class ImgBuffer : public Uncopyable{
    struct Data; //!< internal data storage class
    Data *data;  //!< internal data storage pointer
    ImgBuffer(); //!< private contructor -> use static function instance to get the singelton instance
    public:

    /// Destructor
    ~ImgBuffer(); 
    
    /// Obtain the singelton instance of ImgBuffer
    static ImgBuffer *instance();
    
    /// returns an independent image if available, otherwise, a new one is added
    template<class T>
    Img<T> *get();
    
    /// returns an independent image with preferably correct parameters or a new one
    template<class T>
    Img<T> *get(const Size &size, int channels);

    /// returns an independent image with preferably correct parameters or a new one
    template<class T>
    Img<T> *get(const ImgParams &params);

    /// non template version for get(void)
    ImgBase *get(depth d);

    /// non template version for get(const Size&,channels)
    ImgBase *get(depth d, const Size &size, int channels);

    /// non template version for get(const ImgParams&)
    ImgBase *get(depth d, const ImgParams &p);
  };
  
}

#endif
