/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCV/OpenSurfDetector.h                       **
** Module : ICLCV                                                  **
** Authors: Christian Groszewski                                   **
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

#include <ICLUtils/VisualizationDescription.h>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLCore/OpenCV.h>
#include <opensurf/surflib.h>


namespace icl{
  namespace cv{
    /// This class is a wrapper for opensurf.
    /** Instances of this class detect surf. You can set some parameters to
        make it more stable. For more informations on opensurf visit
        http://www.chrisevansdev.com/computer-vision-opensurf.html*/
    class OpenSurfDetector{
      
      private:
      ///Forwarddeklaration.
      class Data;
      
      ///Class for internal params and buffers.
      Data *m_data;
      
      ///internal match function
      /** Finds matches between surf of object image and passed
          surf of given image.
          @param vector of surfpoints*/
      void match(std::vector<Ipoint> *ipts);
      
      ///computes surf
      void computeSURF();
      
      public:
      
      /// Creates a new OpenSurfDetector object with standard parameters.
      /** Creates a new OpenSurfDetector object. Standard parameters are
          are set, but one can change these later.
          @param obj the object image
          @param upright
          @param octaves
          @param intervals
          @param init_samples
          @param thresh*/
      OpenSurfDetector(const core::ImgBase *obj=0, bool upright=false, int octaves=4,
                       int intervals=4, int init_samples=2, float thresh=0.004f);
      
      ///Destructor
      ~OpenSurfDetector();
      
      /// sets a new image as reference image und  computes surf for it
      /** If the parameter objectImg is null an Exception is thrown.
          @param objectImg the image to be set as new reference image*/
      void setObjectImg(const core::ImgBase *objectImg) throw (utils::ICLException);
      
      /// returns a copy of the reference image
      /** Return a copy of the original reference image. If the reference image
  	  was not set, the method throws an Exception.
          @return copy reference image*/
      const core::ImgBase *getObjectImg() throw (utils::ICLException);
      
      /// returns the surf of the reference image.
      /** If the reference image was not set, the returned vector is empty.
          @return found features*/
      const std::vector<Ipoint> &getObjectImgFeatures();
      
      ///sets rotationinvariance
      /** Note due to compatibility with  opensurf setting
          this parameter false has the behavior you expect.
          @param upright rotationinvariance or not */
      void setRotationInvariant(bool upright);
  
      ///sets the octave count
      /** @param octaves the octavenumber*/
      void setOctaves(int octaves);
      
      ///sets the intervalls
      /** @param intervals the intervalnumber*/
      void setIntervals(int intervals);
      
      ///sets the initsamples
      /** @param init_samples the  samplesnumber*/
      void setInitSamples(int init_samples);
      
      ///sets the responsethresh
      /** @param thresh the thresh*/
      void setRespThresh(float thresh);
      
      ///returns wether rotationinvariance is set or not
      /** @return value for rotationinvariance*/
      bool getRotationInvariant();
      
      ///returns octave count
      /** @return the current octaves number*/
      int getOctaves();
      
      ///returns interval count
      /** @return the current intervals number*/
      int getIntervals();
      
      ///returns initsamples
      /**@return the current initsamples number*/
      int getInitSamples();
      
      ///returns responsethresh
      /** @return the current reponsethresh*/
      float getRespThresh();
      
      /// sets desired parameters
      /** Sets desired parameters and computes surf, but only if the
          parameters have really changed and if a reference image is
          available.
          @param upright rotationinvariance
          @param octaves
          @param intervals
          @param init_samples
          @param thresh*/
      void setParams(bool upright, int octaves, int intervals, int init_samples,
                     float thresh);
      
      /// extracs surf from given image
      /** Extracts surf from given image. If the image is null
          an empty vector is returned.
          @param src source image
          @return vector of computed features*/
      const std::vector<Ipoint> &extractFeatures(const core::ImgBase *src);
      
      /// computes matches between reference image and passed image.
      /** If the passed image or the reference image is null an
          exception is thrown. First point of the pair is the point for the image,
          second for the reference image.
          @param image
          @return matches vector of std::pair in vector*/
      const std::vector<std::pair<Ipoint, Ipoint> > &match(const core::ImgBase *image) throw (utils::ICLException);
      
      /// sets new newObjImage as new reference image and computes matches between passed reference image and passed image.
      /** If one of the passed images null an exception is thrown.
          First point of the pair is the point for the image, second for the reference image.
          @param image
          @param newObjImage the new reference Image
          @return matches vector of std::pair in vector*/
      const std::vector<std::pair<Ipoint, Ipoint> > &match(const core::ImgBase *currentImage,
                                                           const core::ImgBase *newObjImage) throw (utils::ICLException){
        setObjectImg(newObjImage);
        return match(currentImage);
      }
     
      /// for feature visualization
      static void visualizePoint(utils::VisualizationDescription &target, const Ipoint &p);

      /// for feature visualization
      static void visualizePoints(utils::VisualizationDescription &target, const std::vector<Ipoint> &ps);
      
      /// returns 2 visualization descriptions (first for the object, second for the result)
      static std::pair<utils::VisualizationDescription,utils::VisualizationDescription>
      visualizeMatches(const std::vector<std::pair<Ipoint,Ipoint> > &matches);
 
    } // namespace cv
  };
}

