/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/OpenSurfLib.cpp                        **
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


#include <ICLCV/OpenSurfLib.h>

#include <iostream>
#include <fstream>
#include <time.h>

#ifdef ICL_HAVE_OPENCV_OLD_STYLE
#include <opencv/highgui.h>
#else
#include <opencv2/highgui/highgui_c.h>
#endif


namespace icl{
  namespace cv{
    namespace opensurf{

       class FastHessian {
         public:

         /// Constructor without image
         FastHessian(std::vector<Ipoint> &ipts,
                     const int octaves = OCTAVES,
                     const int intervals = INTERVALS,
                     const int init_sample = INIT_SAMPLE,
                     const float thres = THRES);

         /// Constructor with image
         FastHessian(IplImage *img,
                     std::vector<Ipoint> &ipts,
                     const int octaves = OCTAVES,
                     const int intervals = INTERVALS,
                     const int init_sample = INIT_SAMPLE,
                     const float thres = THRES);

         /// Destructor
         ~FastHessian();

         /// Save the parameters
         void saveParameters(const int octaves,
                             const int intervals,
                             const int init_sample,
                             const float thres);

         /// Set or re-set the integral image source
         void setIntImage(IplImage *img);

         /// Find the image features and write into vector of features
         void getIpoints();

         private:

         /// Build map of DoH responses
         void buildResponseMap();

         /// Calculate DoH responses for supplied layer
         void buildResponseLayer(ResponseLayer *r);

         /// 3x3x3 Extrema test
         int isExtremum(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);

         /// Interpolation functions - adapted from Lowe's SIFT implementation
         void interpolateExtremum(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);
         void interpolateStep(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b,
                              double* xi, double* xr, double* xc );
         CvMat* deriv3D(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);
         CvMat* hessian3D(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b);

         /// Pointer to the integral Image, and its attributes
         IplImage *img;
         int i_width, i_height;

         /// Reference to vector of features passed from outside
         std::vector<Ipoint> &ipts;

         /// Response stack of determinant of hessian values
         std::vector<ResponseLayer *> responseMap;

         /// Number of Octaves
         int octaves;

         /// Number of Intervals per octave
         int intervals;

         /// Initial sampling step for Ipoint detection
         int init_sample;

         /// Threshold value for blob resonses
         float thresh;
       };



      //! Constructor without image
      FastHessian::FastHessian(std::vector<Ipoint> &ipts,
                               const int octaves, const int intervals, const int init_sample,
                               const float thresh)
        :i_width(0), i_height(0), ipts(ipts)
      {
        // Save parameter set
        saveParameters(octaves, intervals, init_sample, thresh);
      }



      //! Constructor with image
      FastHessian::FastHessian(IplImage *img, std::vector<Ipoint> &ipts,
                               const int octaves, const int intervals, const int init_sample,
                               const float thresh)
        : i_width(0), i_height(0),ipts(ipts)
      {
        // Save parameter set
        saveParameters(octaves, intervals, init_sample, thresh);

        // Set the current image
        setIntImage(img);
      }



      FastHessian::~FastHessian()
      {
        for (unsigned int i = 0; i < responseMap.size(); ++i)
          {
            delete responseMap[i];
          }
      }



      //! Save the parameters
      void FastHessian::saveParameters(const int octaves, const int intervals,
                                       const int init_sample, const float thresh)
      {
        // Initialise variables with bounds-checked values
        this->octaves =
        (octaves > 0 && octaves <= 4 ? octaves : OCTAVES);
        this->intervals =
        (intervals > 0 && intervals <= 4 ? intervals : INTERVALS);
        this->init_sample =
        (init_sample > 0 && init_sample <= 6 ? init_sample : INIT_SAMPLE);
        this->thresh = (thresh >= 0 ? thresh : THRES);
      }




      //! Set or re-set the integral image source
      void FastHessian::setIntImage(IplImage *img)
      {
        // Change the source image
        this->img = img;

        i_height = img->height;
        i_width = img->width;
      }



      //! Find the image features and write into vector of features
      void FastHessian::getIpoints()
      {
        // filter index map
        static const int filter_map [OCTAVES][INTERVALS] = {{0,1,2,3}, {1,3,4,5}, {3,5,6,7}, {5,7,8,9}, {7,9,10,11}};

        // Clear the vector of exisiting ipts
        ipts.clear();

        // Build the response map
        buildResponseMap();

        // Get the response layers
        ResponseLayer *b, *m, *t;
        for (int o = 0; o < octaves; ++o) for (int i = 0; i <= 1; ++i)
                                            {
                                              b = responseMap.at(filter_map[o][i]);
                                              m = responseMap.at(filter_map[o][i+1]);
                                              t = responseMap.at(filter_map[o][i+2]);

                                              // loop over middle response layer at density of the most
                                              // sparse layer (always top), to find maxima across scale and space
                                              for (int r = 0; r < t->height; ++r)
                                                {
                                                  for (int c = 0; c < t->width; ++c)
                                                    {
                                                      if (isExtremum(r, c, t, m, b))
                                                        {
                                                          interpolateExtremum(r, c, t, m, b);
                                                        }
                                                    }
                                                }
                                            }
      }



      //! Build map of DoH responses
      void FastHessian::buildResponseMap()
      {
        // Calculate responses for the first 4 octaves:
        // Oct1: 9,  15, 21, 27
        // Oct2: 15, 27, 39, 51
        // Oct3: 27, 51, 75, 99
        // Oct4: 51, 99, 147,195
        // Oct5: 99, 195,291,387

        // Deallocate memory and clear any existing response layers
        for(unsigned int i = 0; i < responseMap.size(); ++i)
          delete responseMap[i];
        responseMap.clear();

        // Get image attributes
        int w = (i_width / init_sample);
        int h = (i_height / init_sample);
        int s = (init_sample);

        // Calculate approximated determinant of hessian values
        if (octaves >= 1)
          {
            responseMap.push_back(new ResponseLayer(w,   h,   s,   9));
            responseMap.push_back(new ResponseLayer(w,   h,   s,   15));
            responseMap.push_back(new ResponseLayer(w,   h,   s,   21));
            responseMap.push_back(new ResponseLayer(w,   h,   s,   27));
          }

        if (octaves >= 2)
          {
            responseMap.push_back(new ResponseLayer(w/2, h/2, s*2, 39));
            responseMap.push_back(new ResponseLayer(w/2, h/2, s*2, 51));
          }

        if (octaves >= 3)
          {
            responseMap.push_back(new ResponseLayer(w/4, h/4, s*4, 75));
            responseMap.push_back(new ResponseLayer(w/4, h/4, s*4, 99));
          }

        if (octaves >= 4)
          {
            responseMap.push_back(new ResponseLayer(w/8, h/8, s*8, 147));
            responseMap.push_back(new ResponseLayer(w/8, h/8, s*8, 195));
          }

        if (octaves >= 5)
          {
            responseMap.push_back(new ResponseLayer(w/16, h/16, s*16, 291));
            responseMap.push_back(new ResponseLayer(w/16, h/16, s*16, 387));
          }

        // Extract responses from the image
        for (unsigned int i = 0; i < responseMap.size(); ++i)
          {
            buildResponseLayer(responseMap[i]);
          }
      }



      //! Calculate DoH responses for supplied layer
      void FastHessian::buildResponseLayer(ResponseLayer *rl)
      {
        float *responses = rl->responses;         // response storage
        unsigned char *laplacian = rl->laplacian; // laplacian sign storage
        int step = rl->step;                      // step size for this filter
        int b = (rl->filter - 1) / 2 + 1;         // border for this filter
        int l = rl->filter / 3;                   // lobe for this filter (filter size / 3)
        int w = rl->filter;                       // filter size
        float inverse_area = 1.f/(w*w);           // normalisation factor
        float Dxx, Dyy, Dxy;

        for(int r, c, ar = 0, index = 0; ar < rl->height; ++ar)
          {
            for(int ac = 0; ac < rl->width; ++ac, index++)
              {
                // get the image coordinates
                r = ar * step;
                c = ac * step;

                // Compute response components
                Dxx = BoxIntegral(img, r - l + 1, c - b, 2*l - 1, w)
                - BoxIntegral(img, r - l + 1, c - l / 2, 2*l - 1, l)*3;
                Dyy = BoxIntegral(img, r - b, c - l + 1, w, 2*l - 1)
                - BoxIntegral(img, r - l / 2, c - l + 1, l, 2*l - 1)*3;
                Dxy = + BoxIntegral(img, r - l, c + 1, l, l)
                + BoxIntegral(img, r + 1, c - l, l, l)
                - BoxIntegral(img, r - l, c - l, l, l)
                - BoxIntegral(img, r + 1, c + 1, l, l);

                // Normalise the filter responses with respect to their size
                Dxx *= inverse_area;
                Dyy *= inverse_area;
                Dxy *= inverse_area;

                // Get the determinant of hessian response & laplacian sign
                responses[index] = (Dxx * Dyy - 0.81f * Dxy * Dxy);
                laplacian[index] = (Dxx + Dyy >= 0 ? 1 : 0);

#ifdef RL_DEBUG
                // create list of the image coords for each response
                rl->coords.push_back(std::make_pair<int,int>(r,c));
#endif
              }
          }
      }



      //! Non Maximal Suppression function
      int FastHessian::isExtremum(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b)
      {
        // bounds check
        int layerBorder = (t->filter + 1) / (2 * t->step);
        if (r <= layerBorder || r >= t->height - layerBorder || c <= layerBorder || c >= t->width - layerBorder)
          return 0;

        // check the candidate point in the middle layer is above thresh
        float candidate = m->getResponse(r, c, t);
        if (candidate < thresh)
          return 0;

        for (int rr = -1; rr <=1; ++rr)
          {
            for (int cc = -1; cc <=1; ++cc)
              {
                // if any response in 3x3x3 is greater candidate not maximum
                if (
                    t->getResponse(r+rr, c+cc) >= candidate ||
                    ((rr != 0 && cc != 0) && m->getResponse(r+rr, c+cc, t) >= candidate) ||
                    b->getResponse(r+rr, c+cc, t) >= candidate
                    )
                  return 0;
              }
          }

        return 1;
      }



      //! Interpolate scale-space extrema to subpixel accuracy to form an image feature.
      void FastHessian::interpolateExtremum(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b)
      {
        // get the step distance between filters
        // check the middle filter is mid way between top and bottom
        int filterStep = (m->filter - b->filter);
        assert(filterStep > 0 && t->filter - m->filter == m->filter - b->filter);

        // Get the offsets to the actual location of the extremum
        double xi = 0, xr = 0, xc = 0;
        interpolateStep(r, c, t, m, b, &xi, &xr, &xc );

        // If point is sufficiently close to the actual extremum
        if( fabs( xi ) < 0.5f  &&  fabs( xr ) < 0.5f  &&  fabs( xc ) < 0.5f )
          {
            Ipoint ipt;
            ipt.x = static_cast<float>((c + xc) * t->step);
            ipt.y = static_cast<float>((r + xr) * t->step);
            ipt.scale = static_cast<float>((0.1333f) * (m->filter + xi * filterStep));
            ipt.laplacian = static_cast<int>(m->getLaplacian(r,c,t));
            ipts.push_back(ipt);
          }
      }



      //! Performs one step of extremum interpolation.
      void FastHessian::interpolateStep(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b,
                                        double* xi, double* xr, double* xc )
      {
        CvMat* dD, * H, * H_inv, X;
        double x[3] = { 0 };

        dD = deriv3D( r, c, t, m, b );
        H = hessian3D( r, c, t, m, b );
        H_inv = cvCreateMat( 3, 3, CV_64FC1 );
        cvInvert( H, H_inv, CV_SVD );
        cvInitMatHeader( &X, 3, 1, CV_64FC1, x, CV_AUTOSTEP );
        cvGEMM( H_inv, dD, -1, NULL, 0, &X, 0 );

        cvReleaseMat( &dD );
        cvReleaseMat( &H );
        cvReleaseMat( &H_inv );

        *xi = x[2];
        *xr = x[1];
        *xc = x[0];
      }



      //! Computes the partial derivatives in x, y, and scale of a pixel.
      CvMat* FastHessian::deriv3D(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b)
      {
        CvMat* dI;
        double dx, dy, ds;

        dx = (m->getResponse(r, c + 1, t) - m->getResponse(r, c - 1, t)) / 2.0;
        dy = (m->getResponse(r + 1, c, t) - m->getResponse(r - 1, c, t)) / 2.0;
        ds = (t->getResponse(r, c) - b->getResponse(r, c, t)) / 2.0;

        dI = cvCreateMat( 3, 1, CV_64FC1 );
        cvmSet( dI, 0, 0, dx );
        cvmSet( dI, 1, 0, dy );
        cvmSet( dI, 2, 0, ds );

        return dI;
      }



      //! Computes the 3D Hessian matrix for a pixel.
      CvMat* FastHessian::hessian3D(int r, int c, ResponseLayer *t, ResponseLayer *m, ResponseLayer *b)
      {
        CvMat* H;
        double v, dxx, dyy, dss, dxy, dxs, dys;

        v = m->getResponse(r, c, t);
        dxx = m->getResponse(r, c + 1, t) + m->getResponse(r, c - 1, t) - 2 * v;
        dyy = m->getResponse(r + 1, c, t) + m->getResponse(r - 1, c, t) - 2 * v;
        dss = t->getResponse(r, c) + b->getResponse(r, c, t) - 2 * v;
        dxy = ( m->getResponse(r + 1, c + 1, t) - m->getResponse(r + 1, c - 1, t) -
                m->getResponse(r - 1, c + 1, t) + m->getResponse(r - 1, c - 1, t) ) / 4.0;
        dxs = ( t->getResponse(r, c + 1) - t->getResponse(r, c - 1) -
                b->getResponse(r, c + 1, t) + b->getResponse(r, c - 1, t) ) / 4.0;
        dys = ( t->getResponse(r + 1, c) - t->getResponse(r - 1, c) -
                b->getResponse(r + 1, c, t) + b->getResponse(r - 1, c, t) ) / 4.0;

        H = cvCreateMat( 3, 3, CV_64FC1 );
        cvmSet( H, 0, 0, dxx );
        cvmSet( H, 0, 1, dxy );
        cvmSet( H, 0, 2, dxs );
        cvmSet( H, 1, 0, dxy );
        cvmSet( H, 1, 1, dyy );
        cvmSet( H, 1, 2, dys );
        cvmSet( H, 2, 0, dxs );
        cvmSet( H, 2, 1, dys );
        cvmSet( H, 2, 2, dss );

        return H;
      }


      //! Computes the integral image of image img.  Assumes source image to be a
      //! 32-bit floating point.  Returns IplImage of 32-bit float form.
      IplImage *Integral(IplImage *source)
      {
        // convert the image to single channel 32f
        IplImage *img = getGray(source);
        IplImage *int_img = cvCreateImage(cvGetSize(img), IPL_DEPTH_32F, 1);

        // set up variables for data access
        int height = img->height;
        int width = img->width;
        int step = img->widthStep/sizeof(float);
        float *data   = (float *) img->imageData;
        float *i_data = (float *) int_img->imageData;

        // first row only
        float rs = 0.0f;
        for(int j=0; j<width; j++)
          {
            rs += data[j];
            i_data[j] = rs;
          }

        // remaining cells are sum above and to the left
        for(int i=1; i<height; ++i)
          {
            rs = 0.0f;
            for(int j=0; j<width; ++j)
              {
                rs += data[i*step+j];
                i_data[i*step+j] = rs + i_data[(i-1)*step+j];
              }
          }

        // release the gray image
        cvReleaseImage(&img);

        // return the integral image
        return int_img;
      }

      float BoxIntegral(IplImage *img, int row, int col, int rows, int cols){
        float *data = (float *) img->imageData;
        int step = img->widthStep/sizeof(float);

        // The subtraction by one for row/col is because row/col is inclusive.
        int r1 = std::min(row,          img->height) - 1;
        int c1 = std::min(col,          img->width)  - 1;
        int r2 = std::min(row + rows,   img->height) - 1;
        int c2 = std::min(col + cols,   img->width)  - 1;

        float A(0.0f), B(0.0f), C(0.0f), D(0.0f);
        if (r1 >= 0 && c1 >= 0) A = data[r1 * step + c1];
        if (r1 >= 0 && c2 >= 0) B = data[r1 * step + c2];
        if (r2 >= 0 && c1 >= 0) C = data[r2 * step + c1];
        if (r2 >= 0 && c2 >= 0) D = data[r2 * step + c2];

        return std::max(0.f, A - B - C + D);
      }



      //! Populate IpPairVec with matched ipts
      void getMatches(IpVec &ipts1, IpVec &ipts2, IpPairVec &matches)
      {
        float dist, d1, d2;
        Ipoint *match = 0;

        matches.clear();

        for(unsigned int i = 0; i < ipts1.size(); i++)
          {
            d1 = d2 = FLT_MAX;

            for(unsigned int j = 0; j < ipts2.size(); j++)
              {
                dist = ipts1[i] - ipts2[j];

                if(dist<d1) // if this feature matches better than current best
                  {
                    d2 = d1;
                    d1 = dist;
                    match = &ipts2[j];
                  }
                else if(dist<d2) // this feature matches better than second best
                  {
                    d2 = dist;
                  }
              }

            // If match has a d1:d2 ratio < 0.65 ipoints are a match
            if(d1/d2 < 0.65)
              {
                // Store the change in position
                ipts1[i].dx = match->x - ipts1[i].x;
                ipts1[i].dy = match->y - ipts1[i].y;
                matches.push_back(std::make_pair(ipts1[i], *match));
              }
          }
      }

      //
      // This function uses homography with CV_RANSAC (OpenCV 1.1)
      // Won't compile on most linux distributions
      //



      //! Find homography between matched points and translate src_corners to dst_corners
      int translateCorners(IpPairVec &matches, const CvPoint src_corners[4], CvPoint dst_corners[4])
      {
        double h[9];
        CvMat _h = cvMat(3, 3, CV_64F, h);
        std::vector<CvPoint2D32f> pt1, pt2;
        CvMat _pt1, _pt2;

        int n = (int)matches.size();
        if( n < 4 ) return 0;

        // Set vectors to correct size
        pt1.resize(n);
        pt2.resize(n);

        // Copy Ipoints from match vector into cvPoint vectors
        for(int i = 0; i < n; i++ )
          {
            pt1[i] = cvPoint2D32f(matches[i].second.x, matches[i].second.y);
            pt2[i] = cvPoint2D32f(matches[i].first.x, matches[i].first.y);
          }
        _pt1 = cvMat(1, n, CV_32FC2, &pt1[0] );
        _pt2 = cvMat(1, n, CV_32FC2, &pt2[0] );

        // Find the homography (transformation) between the two sets of points
        if(!cvFindHomography(&_pt1, &_pt2, &_h, CV_RANSAC, 5))  // this line requires opencv 1.1
          return 0;

        // Translate src_corners to dst_corners using homography
        for(int i = 0; i < 4; i++ )
          {
            double x = src_corners[i].x, y = src_corners[i].y;
            double Z = 1./(h[6]*x + h[7]*y + h[8]);
            double X = (h[0]*x + h[1]*y + h[2])*Z;
            double Y = (h[3]*x + h[4]*y + h[5])*Z;
            dst_corners[i] = cvPoint(cvRound(X), cvRound(Y));
          }
        return 1;
      }



      //! SURF priors (these need not be done at runtime)
      const float pi = 3.14159f;

      const double gauss25 [7][7] = {
        {0.02350693969273,0.01849121369071,0.01239503121241,0.00708015417522,0.00344628101733,0.00142945847484,0.00050524879060},
        {0.02169964028389,0.01706954162243,0.01144205592615,0.00653580605408,0.00318131834134,0.00131955648461,0.00046640341759},
        {0.01706954162243,0.01342737701584,0.00900063997939,0.00514124713667,0.00250251364222,0.00103799989504,0.00036688592278},
        {0.01144205592615,0.00900063997939,0.00603330940534,0.00344628101733,0.00167748505986,0.00069579213743,0.00024593098864},
        {0.00653580605408,0.00514124713667,0.00344628101733,0.00196854695367,0.00095819467066,0.00039744277546,0.00014047800980},
        {0.00318131834134,0.00250251364222,0.00167748505986,0.00095819467066,0.00046640341759,0.00019345616757,0.00006837798818},
        {0.00131955648461,0.00103799989504,0.00069579213743,0.00039744277546,0.00019345616757,0.00008024231247,0.00002836202103}
      };

      const double gauss33 [11][11] = {
        {0.014614763,0.013958917,0.012162744,0.00966788,0.00701053,0.004637568,0.002798657,0.001540738,0.000773799,0.000354525,0.000148179},
        {0.013958917,0.013332502,0.011616933,0.009234028,0.006695928,0.004429455,0.002673066,0.001471597,0.000739074,0.000338616,0.000141529},
        {0.012162744,0.011616933,0.010122116,0.008045833,0.005834325,0.003859491,0.002329107,0.001282238,0.000643973,0.000295044,0.000123318},
        {0.00966788,0.009234028,0.008045833,0.006395444,0.004637568,0.003067819,0.001851353,0.001019221,0.000511879,0.000234524,9.80224E-05},
        {0.00701053,0.006695928,0.005834325,0.004637568,0.003362869,0.002224587,0.001342483,0.000739074,0.000371182,0.000170062,7.10796E-05},
        {0.004637568,0.004429455,0.003859491,0.003067819,0.002224587,0.001471597,0.000888072,0.000488908,0.000245542,0.000112498,4.70202E-05},
        {0.002798657,0.002673066,0.002329107,0.001851353,0.001342483,0.000888072,0.000535929,0.000295044,0.000148179,6.78899E-05,2.83755E-05},
        {0.001540738,0.001471597,0.001282238,0.001019221,0.000739074,0.000488908,0.000295044,0.00016243,8.15765E-05,3.73753E-05,1.56215E-05},
        {0.000773799,0.000739074,0.000643973,0.000511879,0.000371182,0.000245542,0.000148179,8.15765E-05,4.09698E-05,1.87708E-05,7.84553E-06},
        {0.000354525,0.000338616,0.000295044,0.000234524,0.000170062,0.000112498,6.78899E-05,3.73753E-05,1.87708E-05,8.60008E-06,3.59452E-06},
        {0.000148179,0.000141529,0.000123318,9.80224E-05,7.10796E-05,4.70202E-05,2.83755E-05,1.56215E-05,7.84553E-06,3.59452E-06,1.50238E-06}
      };





      //! Constructor
      Surf::Surf(IplImage *img, IpVec &ipts)
        : ipts(ipts)
      {
        this->img = img;
      }



      //! Describe all features in the supplied vector
      void Surf::getDescriptors(bool upright)
      {
        // Check there are Ipoints to be described
        if (!ipts.size()) return;

        // Get the size of the vector for fixed loop bounds
        int ipts_size = (int)ipts.size();

        if (upright)
          {
            // U-SURF loop just gets descriptors
            for (int i = 0; i < ipts_size; ++i)
              {
                // Set the Ipoint to be described
                index = i;

                // Extract upright (i.e. not rotation invariant) descriptors
                getDescriptor(true);
              }
          }
        else
          {
            // Main SURF-64 loop assigns orientations and gets descriptors
            for (int i = 0; i < ipts_size; ++i)
              {
                // Set the Ipoint to be described
                index = i;

                // Assign Orientations and extract rotation invariant descriptors
                getOrientation();
                getDescriptor(false);
              }
          }
      }



      //! Assign the supplied Ipoint an orientation
      void Surf::getOrientation()
      {
        Ipoint *ipt = &ipts[index];
        float gauss = 0.f, scale = ipt->scale;
        const int s = fRound(scale), r = fRound(ipt->y), c = fRound(ipt->x);
        std::vector<float> resX(109), resY(109), Ang(109);
        const int id[] = {6,5,4,3,2,1,0,1,2,3,4,5,6};

        int idx = 0;
        // calculate haar responses for points within radius of 6*scale
        for(int i = -6; i <= 6; ++i)
          {
            for(int j = -6; j <= 6; ++j)
              {
                if(i*i + j*j < 36)
                  {
                    gauss = static_cast<float>(gauss25[id[i+6]][id[j+6]]);
                    resX[idx] = gauss * haarX(r+j*s, c+i*s, 4*s);
                    resY[idx] = gauss * haarY(r+j*s, c+i*s, 4*s);
                    Ang[idx] = getAngle(resX[idx], resY[idx]);
                    ++idx;
                  }
              }
          }

        // calculate the dominant direction
        float sumX=0.f, sumY=0.f;
        float max=0.f, orientation = 0.f;
        float ang1=0.f, ang2=0.f;

        // loop slides pi/3 window around feature point
        for(ang1 = 0; ang1 < 2*pi;  ang1+=0.15f) {
          ang2 = ( ang1+pi/3.0f > 2*pi ? ang1-5.0f*pi/3.0f : ang1+pi/3.0f);
          sumX = sumY = 0.f;
          for(unsigned int k = 0; k < Ang.size(); ++k)
            {
              // get angle from the x-axis of the sample point
              const float & ang = Ang[k];

              // determine whether the point is within the window
              if (ang1 < ang2 && ang1 < ang && ang < ang2)
                {
                  sumX+=resX[k];
                  sumY+=resY[k];
                }
              else if (ang2 < ang1 &&
                       ((ang > 0 && ang < ang2) || (ang > ang1 && ang < 2*pi) ))
                {
                  sumX+=resX[k];
                  sumY+=resY[k];
                }
            }

          // if the vector produced from this window is longer than all
          // previous vectors then this forms the new dominant direction
          if (sumX*sumX + sumY*sumY > max)
            {
              // store largest orientation
              max = sumX*sumX + sumY*sumY;
              orientation = getAngle(sumX, sumY);
            }
        }

        // assign orientation of the dominant response vector
        ipt->orientation = orientation;
      }



      //! Get the modified descriptor. See Agrawal ECCV 08
      //! Modified descriptor contributed by Pablo Fernandez
      void Surf::getDescriptor(bool bUpright)
      {
        int y, x, sample_x, sample_y, count=0;
        int i = 0, ix = 0, j = 0, jx = 0, xs = 0, ys = 0;
        float scale, *desc, dx, dy, mdx, mdy, co, si;
        float gauss_s1 = 0.f, gauss_s2 = 0.f;
        float rx = 0.f, ry = 0.f, rrx = 0.f, rry = 0.f, len = 0.f;
        float cx = -0.5f, cy = 0.f; //Subregion centers for the 4x4 gaussian weighting

        Ipoint *ipt = &ipts[index];
        scale = ipt->scale;
        x = fRound(ipt->x);
        y = fRound(ipt->y);
        desc = ipt->descriptor;

        if (bUpright)
          {
            co = 1;
            si = 0;
          }
        else
          {
            co = cos(ipt->orientation);
            si = sin(ipt->orientation);
          }

        i = -8;

        //Calculate descriptor for this interest point
        while(i < 12)
          {
            j = -8;
            i = i-4;

            cx += 1.f;
            cy = -0.5f;

            while(j < 12)
              {
                dx=dy=mdx=mdy=0.f;
                cy += 1.f;

                j = j - 4;

                ix = i + 5;
                jx = j + 5;

                xs = fRound(x + ( -jx*scale*si + ix*scale*co));
                ys = fRound(y + ( jx*scale*co + ix*scale*si));

                for (int k = i; k < i + 9; ++k)
                  {
                    for (int l = j; l < j + 9; ++l)
                      {
                        //Get coords of sample point on the rotated axis
                        sample_x = fRound(x + (-l*scale*si + k*scale*co));
                        sample_y = fRound(y + ( l*scale*co + k*scale*si));

                        //Get the gaussian weighted x and y responses
                        gauss_s1 = gaussian(xs-sample_x,ys-sample_y,2.5f*scale);
                        rx = haarX(sample_y, sample_x, 2*fRound(scale));
                        ry = haarY(sample_y, sample_x, 2*fRound(scale));

                        //Get the gaussian weighted x and y responses on rotated axis
                        rrx = gauss_s1*(-rx*si + ry*co);
                        rry = gauss_s1*(rx*co + ry*si);

                        dx += rrx;
                        dy += rry;
                        mdx += fabs(rrx);
                        mdy += fabs(rry);

                      }
                  }

                //Add the values to the descriptor vector
                gauss_s2 = gaussian(cx-2.0f,cy-2.0f,1.5f);

                desc[count++] = dx*gauss_s2;
                desc[count++] = dy*gauss_s2;
                desc[count++] = mdx*gauss_s2;
                desc[count++] = mdy*gauss_s2;

                len += (dx*dx + dy*dy + mdx*mdx + mdy*mdy) * gauss_s2*gauss_s2;

                j += 9;
              }
            i += 9;
          }

        //Convert to Unit Vector
        len = sqrt(len);
        for(int i = 0; i < 64; ++i)
          desc[i] /= len;

      }




      //! Calculate the value of the 2d gaussian at x,y
      inline float Surf::gaussian(int x, int y, float sig)
      {
        return (1.0f/(2.0f*pi*sig*sig)) * exp( -(x*x+y*y)/(2.0f*sig*sig));
      }



      //! Calculate the value of the 2d gaussian at x,y
      inline float Surf::gaussian(float x, float y, float sig)
      {
        return 1.0f/(2.0f*pi*sig*sig) * exp( -(x*x+y*y)/(2.0f*sig*sig));
      }



      //! Calculate Haar wavelet responses in x direction
      inline float Surf::haarX(int row, int column, int s)
      {
        return BoxIntegral(img, row-s/2, column, s, s/2)
        -1 * BoxIntegral(img, row-s/2, column-s/2, s, s/2);
      }



      //! Calculate Haar wavelet responses in y direction
      inline float Surf::haarY(int row, int column, int s)
      {
        return BoxIntegral(img, row, column-s/2, s/2, s)
        -1 * BoxIntegral(img, row-s/2, column-s/2, s/2, s);
      }



      //! Get the angle from the +ve x-axis of the vector given by (X Y)
      float Surf::getAngle(float X, float Y)
      {
        if(X > 0 && Y >= 0)
          return atan(Y/X);

        if(X < 0 && Y >= 0)
          return pi - atan(-Y/X);

        if(X < 0 && Y < 0)
          return pi + atan(Y/X);

        if(X > 0 && Y < 0)
          return 2*pi - atan(-Y/X);

        return 0;
      }



      static const int NCOLOURS = 8;
      static const CvScalar COLOURS [] = {cvScalar(255,0,0), cvScalar(0,255,0),
                                          cvScalar(0,0,255), cvScalar(255,255,0),
                                          cvScalar(0,255,255), cvScalar(255,0,255),
                                          cvScalar(255,255,255), cvScalar(0,0,0)};



      //! Display error message and terminate program
      void error(const char *msg)
      {
        std::cout << "\nError: " << msg;
        getchar();
        exit(0);
      }



      //! Show the provided image and wait for keypress
      void showImage(const IplImage *img)
      {
        cvNamedWindow("Surf", CV_WINDOW_AUTOSIZE);
        cvShowImage("Surf", img);
        cvWaitKey(0);
      }



      //! Show the provided image in titled window and wait for keypress
      void showImage(char *title,const IplImage *img)
      {
        cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
        cvShowImage(title, img);
        cvWaitKey(0);
      }



      // Convert image to single channel 32F
      IplImage *getGray(const IplImage *img)
      {
        // Check we have been supplied a non-null img pointer
        if (!img) error("Unable to create grayscale image.  No image supplied");

        IplImage* gray8, * gray32;

        gray32 = cvCreateImage( cvGetSize(img), IPL_DEPTH_32F, 1 );

        if( img->nChannels == 1 )
          gray8 = (IplImage *) cvClone( img );
        else {
          gray8 = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );
          cvCvtColor( img, gray8, CV_BGR2GRAY );
        }

        cvConvertScale( gray8, gray32, 1.0 / 255.0, 0 );

        cvReleaseImage( &gray8 );
        return gray32;
      }



      //! Draw all the Ipoints in the provided vector
      void drawIpoints(IplImage *img, std::vector<Ipoint> &ipts, int tailSize)
      {
        Ipoint *ipt;
        float s, o;
        int r1, c1, r2, c2, lap;

        for(unsigned int i = 0; i < ipts.size(); i++)
          {
            ipt = &ipts.at(i);
            s = (2.5f * ipt->scale);
            o = ipt->orientation;
            lap = ipt->laplacian;
            r1 = fRound(ipt->y);
            c1 = fRound(ipt->x);
            c2 = fRound(s * cos(o)) + c1;
            r2 = fRound(s * sin(o)) + r1;

            if (o) // Green line indicates orientation
              cvLine(img, cvPoint(c1, r1), cvPoint(c2, r2), cvScalar(0, 255, 0));
            else  // Green dot if using upright version
              cvCircle(img, cvPoint(c1,r1), 1, cvScalar(0, 255, 0),-1);

            if (lap == 1)
              { // Blue circles indicate dark blobs on light backgrounds
                cvCircle(img, cvPoint(c1,r1), fRound(s), cvScalar(255, 0, 0),1);
              }
            else if (lap == 0)
              { // Red circles indicate light blobs on dark backgrounds
                cvCircle(img, cvPoint(c1,r1), fRound(s), cvScalar(0, 0, 255),1);
              }
            else if (lap == 9)
              { // Red circles indicate light blobs on dark backgrounds
                cvCircle(img, cvPoint(c1,r1), fRound(s), cvScalar(0, 255, 0),1);
              }

            // Draw motion from ipoint dx and dy
            if (tailSize)
              {
                cvLine(img, cvPoint(c1,r1),
                       cvPoint(int(c1+ipt->dx*tailSize), int(r1+ipt->dy*tailSize)),
                       cvScalar(255,255,255), 1);
              }
          }
      }



      //! Draw a single feature on the image
      void drawIpoint(IplImage *img, Ipoint &ipt, int tailSize)
      {
        float s, o;
        int r1, c1, r2, c2, lap;

        s = (2.5f * ipt.scale);
        o = ipt.orientation;
        lap = ipt.laplacian;
        r1 = fRound(ipt.y);
        c1 = fRound(ipt.x);

        // Green line indicates orientation
        if (o) // Green line indicates orientation
          {
            c2 = fRound(s * cos(o)) + c1;
            r2 = fRound(s * sin(o)) + r1;
            cvLine(img, cvPoint(c1, r1), cvPoint(c2, r2), cvScalar(0, 255, 0));
          }
        else  // Green dot if using upright version
          cvCircle(img, cvPoint(c1,r1), 1, cvScalar(0, 255, 0),-1);

        if (lap >= 0)
          { // Blue circles indicate light blobs on dark backgrounds
            cvCircle(img, cvPoint(c1,r1), fRound(s), cvScalar(255, 0, 0),1);
          }
        else
          { // Red circles indicate light blobs on dark backgrounds
            cvCircle(img, cvPoint(c1,r1), fRound(s), cvScalar(0, 0, 255),1);
          }

        // Draw motion from ipoint dx and dy
        if (tailSize)
          {
            cvLine(img, cvPoint(c1,r1),
                   cvPoint(int(c1+ipt.dx*tailSize), int(r1+ipt.dy*tailSize)),
                   cvScalar(255,255,255), 1);
          }
      }



      //! Draw a single feature on the image
      void drawPoint(IplImage *img, Ipoint &ipt)
      {
        float s;
        int r1, c1;

        s = 3;

        r1 = fRound(ipt.y);
        c1 = fRound(ipt.x);

        cvCircle(img, cvPoint(c1,r1), fRound(s), COLOURS[ipt.clusterIndex%NCOLOURS], -1);
        cvCircle(img, cvPoint(c1,r1), fRound(s+1), COLOURS[(ipt.clusterIndex+1)%NCOLOURS], 2);

      }



      //! Draw a single feature on the image
      void drawPoints(IplImage *img, std::vector<Ipoint> &ipts)
      {
        float s;
        int r1, c1;

        for(unsigned int i = 0; i < ipts.size(); i++)
          {
            s = 3;
            r1 = fRound(ipts[i].y);
            c1 = fRound(ipts[i].x);

            cvCircle(img, cvPoint(c1,r1), fRound(s), COLOURS[ipts[i].clusterIndex%NCOLOURS], -1);
            cvCircle(img, cvPoint(c1,r1), fRound(s+1), COLOURS[(ipts[i].clusterIndex+1)%NCOLOURS], 2);
          }
      }



      //! Draw descriptor windows around Ipoints in the provided vector
      void drawWindows(IplImage *img, std::vector<Ipoint> &ipts)
      {
        Ipoint *ipt;
        float s, o, cd, sd;
        int x, y;
        CvPoint2D32f src[4];

        for(unsigned int i = 0; i < ipts.size(); i++)
          {
            ipt = &ipts.at(i);
            s = (10 * ipt->scale);
            o = ipt->orientation;
            y = fRound(ipt->y);
            x = fRound(ipt->x);
            cd = cos(o);
            sd = sin(o);

            src[0].x=sd*s+cd*s+x;   src[0].y=-cd*s+sd*s+y;
            src[1].x=sd*s+cd*-s+x;  src[1].y=-cd*s+sd*-s+y;
            src[2].x=sd*-s+cd*-s+x; src[2].y=-cd*-s+sd*-s+y;
            src[3].x=sd*-s+cd*s+x;  src[3].y=-cd*-s+sd*s+y;

            if (o) // Draw orientation line
              cvLine(img, cvPoint(x, y),
                     cvPoint(fRound(s*cd + x), fRound(s*sd + y)), cvScalar(0, 255, 0),1);
            else  // Green dot if using upright version
              cvCircle(img, cvPoint(x,y), 1, cvScalar(0, 255, 0),-1);


            // Draw box window around the point
            cvLine(img, cvPoint(fRound(src[0].x), fRound(src[0].y)),
                   cvPoint(fRound(src[1].x), fRound(src[1].y)), cvScalar(255, 0, 0),2);
            cvLine(img, cvPoint(fRound(src[1].x), fRound(src[1].y)),
                   cvPoint(fRound(src[2].x), fRound(src[2].y)), cvScalar(255, 0, 0),2);
            cvLine(img, cvPoint(fRound(src[2].x), fRound(src[2].y)),
                   cvPoint(fRound(src[3].x), fRound(src[3].y)), cvScalar(255, 0, 0),2);
            cvLine(img, cvPoint(fRound(src[3].x), fRound(src[3].y)),
                   cvPoint(fRound(src[0].x), fRound(src[0].y)), cvScalar(255, 0, 0),2);

          }
      }



      // Draw the FPS figure on the image (requires at least 2 calls)
      void drawFPS(IplImage *img)
      {
        static int counter = 0;
        static clock_t t;
        static float fps;
        char fps_text[20];
        CvFont font;
        cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, 1.0,1.0,0,2);

        // Add fps figure (every 10 frames)
        if (counter > 10)
          {
            fps = (10.0f/(clock()-t) * CLOCKS_PER_SEC);
            t=clock();
            counter = 0;
          }

        // Increment counter
        ++counter;

        // Get the figure as a string
        sprintf(fps_text,"FPS: %.2f",fps);

        // Draw the string on the image
        cvPutText (img,fps_text,cvPoint(10,25), &font, cvScalar(255,255,0));
      }



      //! Save the SURF features to file
      void saveSurf(char *filename, std::vector<Ipoint> &ipts)
      {
        std::ofstream outfile(filename);

        // output descriptor length
        outfile << "64\n";
        outfile << ipts.size() << "\n";

        // create output line as:  scale  x  y  des
        for(unsigned int i=0; i < ipts.size(); i++)
          {
            outfile << ipts.at(i).scale << "  ";
            outfile << ipts.at(i).x << " ";
            outfile << ipts.at(i).y << " ";
            outfile << ipts.at(i).orientation << " ";
            outfile << ipts.at(i).laplacian << " ";
            outfile << ipts.at(i).scale << " ";
            for(int j=0; j<64; j++)
              outfile << ipts.at(i).descriptor[j] << " ";

            outfile << "\n";
          }

        outfile.close();
      }

      //! Load the SURF features from file
      void loadSurf(char *filename, std::vector<Ipoint> &ipts)
      {
        int descriptorLength, count;
        std::ifstream infile(filename);

        // clear the ipts vector first
        ipts.clear();

        // read descriptor length/number of ipoints
        infile >> descriptorLength;
        infile >> count;

        // for each ipoint
        for (int i = 0; i < count; i++)
          {
            Ipoint ipt;

            // read vals
            infile >> ipt.scale;
            infile >> ipt.x;
            infile >> ipt.y;
            infile >> ipt.orientation;
            infile >> ipt.laplacian;
            infile >> ipt.scale;

            // read descriptor components
            for (int j = 0; j < 64; j++)
              infile >> ipt.descriptor[j];

            ipts.push_back(ipt);

          }
      }

      //! Library function builds vector of described interest points
      void surfDetDes(IplImage *img,  /* image to find Ipoints in */
                      std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
                      bool upright, /* run in rotation invariant mode? */
                      int octaves, /* number of octaves to calculate */
                      int intervals, /* number of intervals per octave */
                      int init_sample, /* initial sampling step */
                      float thres /* blob response threshold */)
      {
        // Create integral-image representation of the image
        IplImage *int_img = Integral(img);

        // Create Fast Hessian Object
        FastHessian fh(int_img, ipts, octaves, intervals, init_sample, thres);

        // Extract interest points and store in vector ipts
        fh.getIpoints();

        // Create Surf Descriptor Object
        Surf des(int_img, ipts);

        // Extract the descriptors for the ipts
        des.getDescriptors(upright);

        // Deallocate the integral image
        cvReleaseImage(&int_img);
      }


      //! Library function builds vector of interest points
      void surfDet(IplImage *img,  /* image to find Ipoints in */
                   std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
                   int octaves, /* number of octaves to calculate */
                   int intervals, /* number of intervals per octave */
                   int init_sample, /* initial sampling step */
                   float thres /* blob response threshold */)
      {
        // Create integral image representation of the image
        IplImage *int_img = Integral(img);

        // Create Fast Hessian Object
        FastHessian fh(int_img, ipts, octaves, intervals, init_sample, thres);

        // Extract interest points and store in vector ipts
        fh.getIpoints();

        // Deallocate the integral image
        cvReleaseImage(&int_img);
      }




      //! Library function describes interest points in vector
      void surfDes(IplImage *img,  /* image to find Ipoints in */
                   std::vector<Ipoint> &ipts, /* reference to vector of Ipoints */
                   bool upright) /* run in rotation invariant mode? */
      {
        // Create integral image representation of the image
        IplImage *int_img = Integral(img);

        // Create Surf Descriptor Object
        Surf des(int_img, ipts);

        // Extract the descriptors for the ipts
        des.getDescriptors(upright);

        // Deallocate the integral image
        cvReleaseImage(&int_img);
      }


      void Kmeans::Run(IpVec *ipts, int clusters, bool init)
      {
        if (!ipts->size()) return;

        SetIpoints(ipts);

        if (init) InitRandomClusters(clusters);

        while (AssignToClusters()) {
          RepositionClusters();
        }
      }


      void Kmeans::SetIpoints(IpVec *ipts)
      {
        this->ipts = ipts;
      }

      void Kmeans::InitRandomClusters(int n)
      {
        // clear the cluster vector
        clusters.clear();

        // Seed the random number generator
        srand((int)time(NULL));

        // add 'n' random ipoints to clusters list as initial centers
        for (int i = 0; i < n; ++i)
          {
            clusters.push_back(ipts->at(rand() % ipts->size()));
          }
      }

      bool Kmeans::AssignToClusters()
      {
        bool Updated = false;

        // loop over all Ipoints and assign each to closest cluster
        for (unsigned int i = 0; i < ipts->size(); ++i)
          {
            float bestDist = FLT_MAX;
            int oldIndex = ipts->at(i).clusterIndex;

            for (unsigned int j = 0; j < clusters.size(); ++j)
              {
                float currentDist = Distance(ipts->at(i), clusters[j]);
                if (currentDist < bestDist)
                  {
                    bestDist = currentDist;
                    ipts->at(i).clusterIndex = j;
                  }
              }

            // determine whether point has changed cluster
            if (ipts->at(i).clusterIndex != oldIndex) Updated = true;
          }

        return Updated;
      }

      void Kmeans::RepositionClusters()
      {
        float x, y, dx, dy, count;

        for (unsigned int i = 0; i < clusters.size(); ++i)
          {
            x = y = dx = dy = 0;
            count = 1;

            for (unsigned int j = 0; j < ipts->size(); ++j)
              {
                if (ipts->at(j).clusterIndex == (int)i)
                  {
                    Ipoint ip = ipts->at(j);
                    x += ip.x;
                    y += ip.y;
                    dx += ip.dx;
                    dy += ip.dy;
                    ++count;
                  }
              }

            clusters[i].x = x/count;
            clusters[i].y = y/count;
            clusters[i].dx = dx/count;
            clusters[i].dy = dy/count;
          }
      }

      float Kmeans::Distance(Ipoint &ip1, Ipoint &ip2)
      {
        return sqrt(pow(ip1.x - ip2.x, 2)
                    + pow(ip1.y - ip2.y, 2)
                    /*+ pow(ip1.dx - ip2.dx, 2)
                        + pow(ip1.dy - ip2.dy, 2)*/);
      }


    } // end of namespace opensurf

  } // end of namespace cv

} // end of namespace icl
