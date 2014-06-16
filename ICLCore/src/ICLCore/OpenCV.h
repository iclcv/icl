/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/OpenCV.h                           **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Christian Groszewski              **
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

#include <opencv/cxcore.h>

#include <ICLCore/CCFunctions.h>
#include <ICLCore/Img.h>
#include <ICLCore/ImgBase.h>

#include <ICLUtils/Uncopyable.h>

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
    ICLCore_API ImgBase *ipl_to_img(CvArr *src,ImgBase **dst=0,DepthPreference e=PREFERE_SRC_DEPTH) throw (utils::ICLException);
  
    ///Convert ICLimage to OpenCV IplImage
    /**Converts ICLimage to IplImage. If dst is NULL, the sourceimagedepth
        will be used, else the destinationimagedepth will be used.
        @param *src pointer to sourceimage
        @param **dst pointer to pointer to destinationimage (IplImage)
        @param e depthpreference*/
    ICLCore_API IplImage *img_to_ipl(const ImgBase *src, IplImage **dst = 0, DepthPreference e = PREFERE_SRC_DEPTH)throw (utils::ICLException);
  
    ///Copy single ICLimage channel to OpenCV single channel CvMat
    /**Copy single ICLimage channel to single channel CvMat. If dst is NULL, the sourceimagedepth
        will be used, else the destinationmatrixdepth will be used.
        @param *src pointer to sourceimage
        @param **dst pointer to pointer to destinationmatrix
        @param channel channel to use*/
    ICLCore_API CvMat* img_to_cvmat(const ImgBase *src, CvMat *dst = 0, int channel = 0) throw (utils::ICLException);
  
    ///Convert single channel ICLimage to OpenCV IplImage
    /**Converts single channel ICLimage to IplImage. Data is shared by source and destination.
        Using icl8u or icl16s the imagedata is not aligned, but OpenCV expects aligned data.
        In this case be careful using further OpenCV functions.
        Be careful when releasig (data)pointers.
        @param *src pointer to sourceimage
        @param *dst pointer to destinationimage (IplImage)*/
    ICLCore_API IplImage *img_to_ipl_shallow(ImgBase *src, IplImage *dst = 0)throw (utils::ICLException);
  
    ///Convert single channel ICLimage to OpenCV CvMat
    /**Converts single channel ICLimage to a single channel CvMat. Data is shared by
        source and destination.
        Be careful when releasig (data)pointers.
        @param *src pointer to sourceimage
        @param *dst pointer to destinationmatrix (IplImage)*/
    ICLCore_API CvMat *img_to_cvmat_shallow(const ImgBase *src, CvMat *dst = 0) throw (utils::ICLException);


    
    /** \cond */
    /// internally used templated selector
    template<class T>
    inline int icl_get_cv_mat_type() { 
      throw utils::ICLException("icl_get_cv_mat_type: invalid type");
      return 0; 
    }
   
    /// specialization of this for ints
    template<> inline int icl_get_cv_mat_type<int>() { return CV_32SC1; }

    /// specialization of this for floats
    template<> inline int icl_get_cv_mat_type<float>() { return CV_32FC1; }
    /** \endcond */    
    
    /// Utility Delete Operator used within CvMatWrapper
    struct CvMatDelOp : public utils::DelOpBase{ 
      static void delete_func(CvMat *m){ 
        if(m) cvReleaseMat(&m);
      } 
    };

    /// Utility class that wraps around a CvMat of type CV_32FC1 
    /** The wrapper simply handles construction, x/y- indexing and 
        deletion of the internally used CvMat pointer in its destructor. */
    template<class T>
    class CvMatWrapper : public utils::Uncopyable{
      utils::Size size; //!< current size
      utils::SmartPtrBase<CvMat,CvMatDelOp> m;

      public:
      /// Constructor, creates a CvMat with given dimensions
      CvMatWrapper(int nrows=0, int ncols=0):
        size(ncols, nrows),
        m( nrows * ncols > 0 ? cvCreateMat(nrows, ncols, icl_get_cv_mat_type<T>()) : 0, true){}

      CvMatWrapper(const utils::Size &size):
        size(size),
        m( size.getDim() > 0 ? cvCreateMat(size.height, size.width, icl_get_cv_mat_type<T>()) : 0, true){}
      
      /// Constructor with given source matrix (wrappes around that)
      /** Optionally, ownership is taken!*/
      CvMatWrapper(CvMat *other, bool takeOwnerShip=false):
        size(0,0), m(other, takeOwnerShip){

        if(other){
          CvSize size = cvGetSize(other);
          this->size = utils::Size(size.width,size.height);
        }
      }
        
      /// returns the current wrapped CvMat pointer
      CvMat *get() { return m.get(); }

      /// returns the current wrapped CvMat pointer (const)
      const CvMat *get() const { return m.get(); }
      

      /// adapts the size (if necessary)
      /** If the size is adapted, the newly created instace's ownership is taken over */
      inline void setSize(const utils::Size &size){
        if(size == this->size) return;
        this->size = size;
        if(size.getDim() > 0){
          CvMat *m = cvCreateMat(size.height, size.width, icl_get_cv_mat_type<T>());
          this->m = utils::SmartPtrBase<CvMat,CvMatDelOp>(m,true);
        }else{
          this->m = utils::SmartPtrBase<CvMat,CvMatDelOp>();
        }
      }
      
      /// returns whether a non-null pointer is wrapped
      inline bool isNull() const { return m.get() == 0; }

      /// returns the current size
      inline const utils::Size &getSize() const {
        return size;
      }
      
      /// index operator
      T &operator()(int y, int x) {
        return CV_MAT_ELEM(*m,T,y,x);
      }

      /// index operator (const)
      const T &operator()(int y, int x) const{
        return CV_MAT_ELEM((const_cast<CvMat&>(*m)),T,y,x);
      }

    };
      
    /// Overloaded ostream operator for the CvMat32fWrapper
    template<class T>
    inline std::ostream &operator<<(std::ostream &str, const CvMatWrapper<T> &m){
      utils::Size s = m.getSize();
      std::cout << "CvMatWrapper of a " <<  s.height << " x " << s.width << " matrix:" << "\n";

      for(int y=0;y<s.height;++y){
        for(int x=0;x<s.width;++x){
          str << m(y,x) << ((x == s.width-1) ? "\n" : "  ");
        }
      }
      return str;
    }

  } // namespace core
}


