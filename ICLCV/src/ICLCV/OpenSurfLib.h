/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/OpenSurfLib.h                          **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter (+see below)                       **
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

/*********************************************************** 
* Please note that this file contains the whole OpenSURF   *
* library. We decided to include the library by-source     *
* in order to be able to ship its functionality easier     *
*                                                          *
*  --- OpenSURF ---                                        *
*  This library is distributed under the GNU GPL. Please   *
*  use the contact form at http://www.chrisevansdev.com    *
*  for more information.                                   *
*                                                          *
*  C. Evans, Research Into Robust Visual Features,         *
*  MSc University of Bristol, 2008.                        *
*                                                          *
************************************************************/

#pragma once

#include <vector>
#include <opencv/cv.h>

#include <ICLCV/SurfFeature.h>

namespace icl{

   namespace cv{
     
     namespace opensurf{

       typedef SurfFeature Ipoint;
       typedef std::vector<SurfFeature> IpVec;
       typedef std::vector<SurfMatch> IpPairVec;
  
       /** \cond */
       class ResponseLayer;
       class FastHessian;
       /** \endcond */


       ICL_CV_API void getMatches(IpVec &ipts1, IpVec &ipts2, IpPairVec &matches);
  
       ICL_CV_API int translateCorners(IpPairVec &matches, const CvPoint src_corners[4], CvPoint dst_corners[4]);
  
       /// Computes the integral image of image img.  Assumes source image to be a 
       /** 32-bit floating point.  Returns IplImage in 32-bit float form. **/
       ICL_CV_API IplImage *Integral(IplImage *img);
  
  
       /// Computes the sum of pixels within the rectangle 
       /** specified by the top-left start co-ordinate and size */
       ICL_CV_API float BoxIntegral(IplImage *img, int row, int col, int rows, int cols);

  
       

       /// Kmeans clustering
       /** Kmeans clustering class (under development)
           - Can be used to cluster points based on their location.
           - Create Kmeans object and call Run with IpVec.
           - Planned improvements include clustering based on motion 
           and descriptor components. */
       class ICL_CV_API Kmeans {
         public:

         /// Destructor
         ~Kmeans() {};

         /// Constructor
         Kmeans() {};

         /// Do it all!
         void Run(IpVec *ipts, int clusters, bool init = false);

         /// Set the ipts to be used
         void SetIpoints(IpVec *ipts);

         /// Randomly distribute 'n' clusters
         void InitRandomClusters(int n);

         /// Assign Ipoints to clusters
         bool AssignToClusters();

         /// Calculate new cluster centers
         void RepositionClusters();

         /// Function to measure the distance between 2 ipoints
         float Distance(Ipoint &ip1, Ipoint &ip2);

         /// Vector stores ipoints for this run
         IpVec *ipts;

         /// Vector stores cluster centers
         IpVec clusters;

       };

       /// Response Layer class
       class ICL_CV_API ResponseLayer{
         public:

         int width, height, step, filter;
         float *responses;
         unsigned char *laplacian;

         inline ResponseLayer(int width, int height, int step, int filter){
           assert(width > 0 && height > 0);
      
           this->width = width;
           this->height = height;
           this->step = step;
           this->filter = filter;
      
           responses = new float[width*height];
           laplacian = new unsigned char[width*height];
      
           memset(responses,0,sizeof(float)*width*height);
           memset(laplacian,0,sizeof(unsigned char)*width*height);
         }

         inline ~ResponseLayer(){
           if (responses) delete [] responses;
           if (laplacian) delete [] laplacian;
         }
    
         inline unsigned char getLaplacian(unsigned int row, unsigned int column)
         {
           return laplacian[row * width + column];
         }

         inline unsigned char getLaplacian(unsigned int row, unsigned int column, ResponseLayer *src)
         {
           int scale = this->width / src->width;

#ifdef RL_DEBUG
           assert(src->getCoords(row, column) == this->getCoords(scale * row, scale * column));
#endif

           return laplacian[(scale * row) * width + (scale * column)];
         }

         inline float getResponse(unsigned int row, unsigned int column)
         {
           return responses[row * width + column];
         }

         inline float getResponse(unsigned int row, unsigned int column, ResponseLayer *src)
         {
           int scale = this->width / src->width;

#ifdef RL_DEBUG
           assert(src->getCoords(row, column) == this->getCoords(scale * row, scale * column));
#endif

           return responses[(scale * row) * width + (scale * column)];
         }

#ifdef RL_DEBUG
         std::vector<std::pair<int, int>> coords;

         inline std::pair<int,int> getCoords(unsigned int row, unsigned int column)
         {
           return coords[row * width + column];
         }

         inline std::pair<int,int> getCoords(unsigned int row, unsigned int column, ResponseLayer *src)
         {
           int scale = this->width / src->width;
           return coords[(scale * row) * width + (scale * column)];
         }
#endif
       };

       /// Surf Feation class
       class ICL_CV_API Surf {
  
         public:
    
         /// Standard Constructor (img is an integral image)
         Surf(IplImage *img, std::vector<Ipoint> &ipts);

         /// Describe all features in the supplied vector
         void getDescriptors(bool bUpright = false);
  
         private:

         /// Assign the current Ipoint an orientation
         void getOrientation();
    
         /// Get the descriptor. See Agrawal ECCV 08
         void getDescriptor(bool bUpright = false);

         /// Calculate the value of the 2d gaussian at x,y
         inline float gaussian(int x, int y, float sig);
         inline float gaussian(float x, float y, float sig);

         /// Calculate Haar wavelet responses in x and y directions
         inline float haarX(int row, int column, int size);
         inline float haarY(int row, int column, int size);

         /// Get the angle from the +ve x-axis of the vector given by [X Y]
         float getAngle(float X, float Y);


         /// Integral image where Ipoints have been detected
         IplImage *img;

         /// Ipoints vector
         IpVec &ipts;

         /// Index of current Ipoint in the vector
         int index;
       };

       static const int OCTAVES = 5;
       static const int INTERVALS = 4;
       static const float THRES = 0.0004f;
       static const int INIT_SAMPLE = 2;

       /// Library function builds vector of described interest points
       ICL_CV_API void surfDetDes(IplImage *img,  /* image to find Ipoints in */
                       std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
                       bool upright = false, /* run in rotation invariant mode? */
                       int octaves = OCTAVES, /* number of octaves to calculate */
                       int intervals = INTERVALS, /* number of intervals per octave */
                       int init_sample = INIT_SAMPLE, /* initial sampling step */
                       float thres = THRES /* blob response threshold */);

       /// Library function builds vector of interest points
       ICL_CV_API void surfDet(IplImage *img,  /* image to find Ipoints in */
                    std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
                    int octaves = OCTAVES, /* number of octaves to calculate */
                    int intervals = INTERVALS, /* number of intervals per octave */
                    int init_sample = INIT_SAMPLE, /* initial sampling step */
                    float thres = THRES /* blob response threshold */);

       /// Library function describes interest points in vector
       ICL_CV_API void surfDes(IplImage *img,  /* image to find Ipoints in */
                    std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
                    bool upright = false); /* run in rotation invariant mode? */


       /// Display error message and terminate program
       ICL_CV_API void error(const char *msg);

       /// Show the provided image and wait for keypress
       ICL_CV_API void showImage(const IplImage *img);

       /// Show the provided image in titled window and wait for keypress
       ICL_CV_API void showImage(char *title, const IplImage *img);

       // Convert image to single channel 32F
       ICL_CV_API IplImage* getGray(const IplImage *img);

       /// Draw a single feature on the image
       ICL_CV_API void drawIpoint(IplImage *img, Ipoint &ipt, int tailSize = 0);

       /// Draw all the Ipoints in the provided vector
       ICL_CV_API void drawIpoints(IplImage *img, std::vector<Ipoint> &ipts, int tailSize = 0);

       /// Draw descriptor windows around Ipoints in the provided vector
       ICL_CV_API void drawWindows(IplImage *img, std::vector<Ipoint> &ipts);

       // Draw the FPS figure on the image (requires at least 2 calls)
       ICL_CV_API void drawFPS(IplImage *img);

       /// Draw a Point at feature location
       ICL_CV_API void drawPoint(IplImage *img, Ipoint &ipt);

       /// Draw a Point at all features
       ICL_CV_API void drawPoints(IplImage *img, std::vector<Ipoint> &ipts);

       /// Save the SURF features to file
       ICL_CV_API void saveSurf(char *filename, std::vector<Ipoint> &ipts);

       /// Load the SURF features from file
       ICL_CV_API void loadSurf(char *filename, std::vector<Ipoint> &ipts);

       /// Round float to nearest integer
       inline int fRound(float flt) {  return (int) floor(flt+0.5f); }
 
    } // end of namespace opensurf

  } // end of namespace cv

} // end of namespace icl
