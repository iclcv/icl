/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CornerDetectorCSS.cpp                  **
** Module : ICLCV                                                  **
** Authors: Erik Weitnauer                                         **
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

#include <ICLCV/CornerDetectorCSS.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Point32f.h>
#include <cstring>

#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLProgram.h>
#endif
#include <ICLUtils/CLIncludes.h>

using namespace std;
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{
#ifdef ICL_HAVE_OPENCL
    struct CornerDetectorCSS::CLCurvature{
      CLProgram deriveProgram;
      CLKernel deriveAllAndCurvatureKernel;
      CLCurvature(){
        // source code
        static const char *code = (
                                    "__kernel void deriveAllAndCurvature(float curvature_cutoff,       	\n"
                                    "                        const __global float *x,                	\n"
                                    "                        const __global float *y,                	\n"
                                    "                        __global float *curvature){                \n"
                                    "    	unsigned int i = get_global_id(0);		        \n"
                                    "    float x_1 = (x[i+2]-x[i])/2.f;					\n"
                                    "    float x_0 = (x[i]-x[i-2])/2.f;					\n"
                                    "    float xu = (x[i+1]-x[i-1])/2.f;				\n"
                                    "    float y_1 = (y[i+2]-y[i])/2.f;					\n"
                                    "    float y_0 = (y[i]-y[i-2])/2.f;					\n"
                                    "    float yu = (y[i+1]-y[i-1])/2.f;				\n"
                                    "    float xuu = (x_1-x_0)/2.f;					\n"
                                    "    float yuu = (y_1-y_0)/2.f;					\n"
                                    "    float k = fabs((xu*yuu - xuu*yu) / pow((xu*xu + yu*yu),1.5f)); \n"
                                    "    curvature[i] = ((int)(k*curvature_cutoff+0.5f))/curvature_cutoff; \n"
                                    "}\n"
                                   );

        deriveProgram = CLProgram("gpu",code);
        deriveAllAndCurvatureKernel = deriveProgram.createKernel("deriveAllAndCurvature");
      }

      void operator()(const icl32f *in_x, const icl32f *in_y,
                      float curvature_cutoff, unsigned int length, icl32f *curvature_out){
        // make sure length is dividable by 2 to avoid prime numbers which can cause
        // weird issues witht the workgroup size and slow down the kernel
        unsigned int save_length = length + length % 2;
        unsigned int dim = length * sizeof(icl32f);
        unsigned int save_dim = save_length * sizeof(icl32f);

        CLBuffer x = deriveProgram.createBuffer("r",save_dim);
        CLBuffer y = deriveProgram.createBuffer("r",save_dim);
        CLBuffer curvature = deriveProgram.createBuffer("w",save_dim);
        x.write(in_x,dim);
        y.write(in_y,dim);
        deriveAllAndCurvatureKernel.setArgs(curvature_cutoff, x, y, curvature);
        deriveAllAndCurvatureKernel.apply(save_length,0,0);
        curvature.read(curvature_out,dim);
      }
    };
#else
    struct CornerDetectorCSS::CLCurvature{
      // dummy
    };
#endif



    inline int wrap(int index, int length) {
      index = index%length;
      return index<0 ? index+length : index;
    }

    CornerDetectorCSS::CornerDetectorCSS(float angle_thresh,
                                         float rc_coeff,
                                         float sigma,
                                         float curvature_cutoff,
                                         float straight_line_thresh,
                                         bool accurate):
      angle_thresh(angle_thresh), rc_coeff(rc_coeff), sigma(sigma),
      curvature_cutoff(curvature_cutoff),
      straight_line_thresh(straight_line_thresh),
      accurate(accurate), clcurvature(0), useOpenCL(false){
    }

    int CornerDetectorCSS::gauss_radius(float sigma, float cutoff) {
      float ssq2 = sigma*sigma*2.f;
      float norm_inv = sqrt(float(M_PI)*ssq2);
      return ceil(sqrt(-ssq2*log(cutoff * norm_inv)));
    }

    void CornerDetectorCSS::fill_gauss(float *mask, float sigma, int width) {
      float ssq2 = sigma*sigma*2.f;
      float norm_inv = sqrt(float(M_PI)*ssq2);
      float log_norm_inv = log(norm_inv);
      float sum = 0;
      for (int i=-width; i<=width; i++) {
        mask[i+width] = exp(-(i*i)/ssq2-log_norm_inv);
        sum += mask[i+width];
      }
      for (int i=0; i<width*2+1; i++) {
        mask[i] /= sum;
      }
    }

    CornerDetectorCSS::~CornerDetectorCSS(){
      ICL_DELETE(clcurvature);
    }

    void CornerDetectorCSS::convolute(const float *data, int data_length, const float *mask , int mask_length, float *convoluted) {
#ifdef ICL_HAVE_IPP
      int radius = mask_length / 2;
      float *val = new float[data_length + 2 * radius];
      memcpy(val, data + data_length - radius, sizeof(float) * radius);
      memcpy(val + radius, data, sizeof(float) * data_length);
      memcpy(val + radius + data_length, data, sizeof(float) * radius);
      float *con = new float[data_length + 2 * radius + mask_length - 1];
      //ippsConv_32f(val, data_length + 2 * radius, mask, mask_length, con);

      IppStatus status=ippStsNoErr;
      const int src1Len = data_length + 2 * radius, src2Len = mask_length;// dstLen = src1Len+src2Len-1;
      IppEnum funCfg = (IppEnum)(ippAlgAuto);
      int bufSize = 0;
      Ipp8u *pBuffer;
      status = ippsConvolveGetBufferSize(src1Len, src2Len, ipp32f, funCfg, &bufSize);
      if(status != ippStsNoErr) WARNING_LOG("IPP Error");
      pBuffer = ippsMalloc_8u( bufSize );
      status = ippsConvolve_32f(val, src1Len, mask, src2Len, con, funCfg, pBuffer);
      if(status != ippStsNoErr) WARNING_LOG("IPP Error");
      ippsFree( pBuffer );

      memcpy(convoluted,con+radius * 2,data_length * sizeof(float));
      delete val;
      delete con;
#else
      int radius = mask_length / 2;
      for(int i = 0; i < data_length; i++) {
        float val = 0;
        int offset = i - radius;
        for(int j = 0; j < mask_length; j++) {
          val += data[wrap(offset+j,data_length)] * mask[j];
        }
        convoluted[i] = val;
      }
#endif
    }

    //this functions expects a minimum length of 3
    void CornerDetectorCSS::calculate_curvatures(const float *smoothed_x, const float *smoothed_y,
                                                 int length, float curvature_cutoff, float *curvatures) {
      //calculate special cases
      //i = 0
      float xu_0 = (smoothed_x[0] - smoothed_x[length-2]) * 0.5f;
      float xu = (smoothed_x[1] - smoothed_x[length-1]) * 0.5f;
      float xu_1 = (smoothed_x[2] - smoothed_x[0]) * 0.5f;
      float xuu = (xu_1 - xu_0) * 0.5f;

      float yu_0 = (smoothed_y[0] - smoothed_y[length-2]) * 0.5f;
      float yu = (smoothed_y[0+1] - smoothed_y[length-1]) * 0.5f;
      float yu_1 = (smoothed_y[0+2] - smoothed_y[0]) * 0.5f;
      float yuu = (yu_1 - yu_0) * 0.5f;

      float k = fabs((xu*yuu - xuu*yu) / pow((xu*xu + yu*yu),1.5f));
      curvatures[0] = round(k*curvature_cutoff)/curvature_cutoff;

      for(int i = 1; i < length-2; i++) {
        xu_0 = xu;
        xu = xu_1;
        xu_1 = (smoothed_x[i+2] - smoothed_x[i]) * 0.5f;
        xuu = (xu_1 - xu_0) * 0.5f;

        yu_0 = yu;
        yu = yu_1;
        yu_1 = (smoothed_y[i+2] - smoothed_y[i]) * 0.5f;
        yuu = (yu_1 - yu_0) * 0.5f;

        k = fabs((xu*yuu - xuu*yu) / pow((xu*xu + yu*yu),1.5f));
        curvatures[i] = round(k*curvature_cutoff)/curvature_cutoff;
      }

      //calculate special cases
      //i = length-2
      xu_0 = xu;
      xu = xu_1;
      xu_1 = (smoothed_x[0] - smoothed_x[length-2]) * 0.5f;
      xuu = (xu_1 - xu_0) * 0.5f;

      yu_0 = yu;
      yu = yu_1;
      yu_1 = (smoothed_y[0] - smoothed_y[length-2]) * 0.5f;
      yuu = (yu_1 - yu_0) * 0.5f;

      k = fabs((xu*yuu - xuu*yu) / pow((xu*xu + yu*yu),1.5f));
      curvatures[length-2] = round(k*curvature_cutoff)/curvature_cutoff;

      //i = length-1
      xu_0 = xu;
      xu = xu_1;
      xu_1 = (smoothed_x[1] - smoothed_x[length-1]) * 0.5f;
      xuu = (xu_1 - xu_0) * 0.5f;

      yu_0 = yu;
      yu = yu_1;
      yu_1 = (smoothed_y[1] - smoothed_y[length-1]) * 0.5f;
      yuu = (yu_1 - yu_0) * 0.5f;

      k = fabs((xu*yuu - xuu*yu) / pow((xu*xu + yu*yu),1.5f));
      curvatures[length-1] = round(k*curvature_cutoff)/curvature_cutoff;
    }

    void CornerDetectorCSS::calculate_curvatures_bulk(int array_length, int num_boundaries,
                                                      const int *lengths, const int *indices,
                                                      const int *indices_padded, const float *smoothed_x,
                                                      const float *smoothed_y, float curvature_cutoff,
                                                      float *curvature) {
      float *padded_x = new float[array_length + num_boundaries * 4];
      float *padded_y = new float[array_length + num_boundaries * 4];
      for(int i = 0; i < num_boundaries; i++) {
        memcpy(&padded_x[indices_padded[i]],&smoothed_x[indices[i]+lengths[i]-2],2 * sizeof(float));
        memcpy(&padded_x[indices_padded[i]+2+lengths[i]],&smoothed_x[indices[i]],2 * sizeof(float));

        memcpy(&padded_y[indices_padded[i]],&smoothed_y[indices[i]+lengths[i]-2],2 * sizeof(float));
        memcpy(&padded_y[indices_padded[i]+2+lengths[i]],&smoothed_y[indices[i]],2 * sizeof(float));

        //copy data
        memcpy(&padded_x[indices_padded[i] + 2], &smoothed_x[indices[i]],sizeof(float) * lengths[i]);
        memcpy(&padded_y[indices_padded[i] + 2], &smoothed_y[indices[i]],sizeof(float) * lengths[i]);
      }
      //calculate curvatures
      bool done = false;
#ifdef ICL_HAVE_OPENCL
      if(useOpenCL){
        if(!clcurvature) clcurvature = new CLCurvature;
        (*clcurvature)(padded_x,padded_y,curvature_cutoff,
                       array_length + num_boundaries * 4, curvature);
        done = true;
      }
#endif
      if(!done){
        calculate_curvatures(padded_x, padded_y,
                             array_length + num_boundaries * 4,
                             curvature_cutoff, curvature);
      }
      delete padded_x;
      delete padded_y;
    }

    //returns the offset of the first maxima
    int CornerDetectorCSS::findExtrema(int *extrema, int *num_extrema_out, float* k, int length) {
      int num_extrema = 0;
      int maxima_offset;
      float search;
      float current = k[length-1];
      float next = k[0];
      if(next-current > 0.f) {
        //-1 meaning that we search for a maximum, because we currently have a positive gradient
        search = -1.f;
        //if we search for a maximum that means the offset to the next maxima will be 0
        maxima_offset = 0;
      } else {
        //1 meaning that we search for a minimum, because we currently have a negative gradient
        search = 1.f;
        //if we search for a minimum that means the offset to the next maxima will be 1
        maxima_offset = 1;
      }
      for(int i = 0; i < length -1; i++) {
        current = next;
        next = k[i+1];
        if((next - current) * search > 0) {
          extrema[num_extrema++] = i;
          search *= -1.f;
        }
      }
      if(num_extrema % 2 != 0) extrema[num_extrema++] = length - 1;
      *num_extrema_out = num_extrema;
      return maxima_offset;
    }

    void CornerDetectorCSS::removeRoundCorners(float rc_coeff, int maxima_offset, float* k,
                                               int length, int *extrema, int num_extrema,
                                               int *new_extrema, int *num_new_extrema_out) {
      int num_new_extrema = 0;
      for (int i=maxima_offset; i<num_extrema; i+=2) {
        float kl = k[extrema[wrap(i-1,num_extrema)]];
        float ki = k[extrema[i]];
        float kr = k[extrema[wrap(i+1,num_extrema)]];
        if (ki >= (kl+kr)){
          new_extrema[num_new_extrema++] = extrema[i];
        }
      }
      *num_new_extrema_out = num_new_extrema;
    }

    void CornerDetectorCSS::removeRoundCornersAccurate(float rc_coeff, int maxima_offset,
                                                       float* k, int length, int *extrema,
                                                       int num_extrema, int *extrema_out,
                                                       int *num_extrema_out) {
      int num_new_extrema = 0;
      float mean;
      int n;
      for (int i=maxima_offset; i<num_extrema-1; i+=2) {
        mean = 0; n = 0;
        // get mean curvature value over ROS (region of support)
        // here the region of support searches for first minimum value both left and right each maximum
        float minimum_left = k[extrema[wrap(i-1,num_extrema)]];
        float minimum_right = k[extrema[wrap(i+1,num_extrema)]];
        for (int j=wrap(extrema[i]-1,length); k[j]>minimum_left; j = wrap(j-1,length)) { mean += k[j]; n++; }
        for (int j=wrap(extrema[i]+1,length); k[j]>minimum_right; j = wrap(j+1,length)) { mean += k[j]; n++; }
        mean /= n;
        if (k[extrema[i]] >= rc_coeff*mean) {
          extrema_out[num_new_extrema++] = extrema[i];
        }
      }
      *num_extrema_out = num_new_extrema;
    }

    float CornerDetectorCSS::cornerAngle(float *x, float *y, int prev, int current,
                                         int next, int length, float straight_line_thresh) {
      float xr = x[next] - x[current];
      float yr = y[next] - y[current];
      float xl = x[prev] - x[current];
      float yl = y[prev] - y[current];
      float len_left = sqrt(xl*xl+yl*yl);
      float len_right = sqrt(xr*xr+yr*yr);
      float cos_angle = (xr*xl + yr * yl) / (len_left * len_right);
      float angle = acos(cos_angle);
      angle *= 180.f / M_PI;
      return  angle;

    }

    template <typename T> int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }

    float CornerDetectorCSS::cornerAngleAccurate(float *x, float *y, int prev, int current,
                                                 int next, int array_length,
                                                 float straight_line_thresh) {
      int last, first, middle;
      float x0,y0,x1,y1,x2,y2,x3,y3;
      float tangent_direction;
      float diff_angle;
      float direction[2];
      for (int leftright=0; leftright<2; leftright++) {
        first = current;
        if (leftright == 0) last = prev; // left tangent
        else last = next; // right tangent
        int dist = abs(first-last);
        if (dist>3) {
          middle = (first + last);
          if(dist >= array_length/2)middle -= array_length/2;
          x1 = x[first]; y1 = y[first];
          x2 = x[middle]; y2 = y[middle];
          x3 = x[last]; y3 = y[last];

          diff_angle = fmod((atan2(y3-y1,x3-x1) - atan2(y2-y1,x2-x1)), float(2.f*M_PI));
          if (diff_angle>float(M_PI)) diff_angle = float(2*M_PI)-diff_angle;
          if (diff_angle*180/float(M_PI) < max(straight_line_thresh, 0.01f)) { // fit straight line
            tangent_direction = atan2(y[last]-y[first], x[last]-x[first]);
          } else { // fit a circle
            const float sum1 = (x1*x1+y1*y1);
            const float sum2 = (x2*x2+y2*y2);
            const float sum3 = (x3*x3+y3*y3);
            x0 = 0.5f*(sum1*(y2-y3) + sum2*(y3-y1) + sum3*(y1-y2)) /
                 (x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2));
            y0 = 0.5f*(sum1*(x2-x3) + sum2*(x3-x1) + sum3*(x1-x2)) /
                 (y1*(x2-x3) + y2*(x3-x1) + y3*(x1-x2));
            float radius_dir = atan2(y0-y1, x0-x1);
            float adjacent_dir = atan2(y2-y1, x2-x1);
            tangent_direction = sign(sin(adjacent_dir-radius_dir))*float(M_PI/2.f) + radius_dir;
          }
        } else { // very short line
          tangent_direction = atan2(y[last]-y[first], x[last]-x[first]);
        }
        direction[leftright] = tangent_direction*float(180.f/M_PI);
      }
      float angle = direction[0]-direction[1];
      return (angle < 0) ? angle+360 : (angle > 360) ? angle-360 : angle;
    }

    void CornerDetectorCSS::removeFalseCorners(float angle_thresh, float* x,
                                               float* y, float* k, int length,
                                               int *maxima, int num_maxima,
                                               int *maxima_out, int *num_maxima_out) {

      int *new_maxima = maxima_out;
      int num_new_maxima;
      bool has_changed;
      do {
        num_new_maxima = 0;
        has_changed = false;

        for (int i=0; i<num_maxima; i++) {
          int i_left = maxima[wrap(i-1,num_maxima)];
          int i_right = maxima[wrap(i+1,num_maxima)];
          int i_corner = maxima[i];
          float angle;
          if(accurate) angle = cornerAngleAccurate(x,y,i_left,i_corner,i_right,length,straight_line_thresh);
          else angle = cornerAngle(x,y,i_left,i_corner,i_right,length,straight_line_thresh);
          if ((angle<=angle_thresh) || (angle>=360.f-angle_thresh)) {
            new_maxima[num_new_maxima++] = maxima[i];
          } else {
            has_changed = true;
          }
        }

        //swap arrays
        int* temp = maxima;
        int num_temp = num_maxima;

        maxima = new_maxima;
        num_maxima = num_new_maxima;

        new_maxima = temp;
        num_new_maxima = num_temp;

      } while (has_changed);
      //copy the final values to the final destination
      for(int i = 0; i < num_maxima; i++) {
        maxima_out[i] = maxima[i];
      }
      *num_maxima_out = num_maxima;
    }

    template<class T>
    const vector<Point32f> &CornerDetectorCSS::detectCorners(const vector<T> &boundary) {
      corners.clear();
      //calculate length of the data
      int length = boundary.size();
      int gauss_length = gauss_radius(sigma, 0.0001) * 2 + 1;
      if(gauss_length < length) {
        //create needed arrays
        float *x = new float[length], *y = new float[length];
        float *smoothed_x = new float[length], *smoothed_y = new float[length];
        float *curvature = new float[length];
        float *gauss = new float[gauss_length];

        int *extrema0 = new int[length];
        int extrema0_sizes;
        int *extrema1 = new int[length];
        int extrema1_sizes;

        //copy data into arrays
        for(unsigned int i = 0; i < boundary.size(); i++) {
          x[i] = boundary[i].x;
          y[i] = boundary[i].y;
        }
        fill_gauss(gauss,sigma,gauss_length / 2);

        //smooth arrays
        convolute(x,length,gauss,gauss_length,smoothed_x);
        convolute(y,length,gauss,gauss_length,smoothed_y);

        //calculate curvature
        calculate_curvatures(smoothed_x, smoothed_y, length, curvature_cutoff, curvature);

        //find extrema
        int maxima_offset = findExtrema(extrema0,&extrema0_sizes,curvature,length);
        //remove round corners
        if(accurate)removeRoundCornersAccurate(rc_coeff, maxima_offset, curvature, length, extrema0, extrema0_sizes, extrema1, &extrema1_sizes);
        else removeRoundCorners(rc_coeff, maxima_offset, curvature, length, extrema0, extrema0_sizes, extrema1, &extrema1_sizes);
        //remove false corners
        removeFalseCorners(angle_thresh,smoothed_x,smoothed_y,curvature,length,extrema1,extrema1_sizes,extrema0,&extrema0_sizes);
        //the final corner indices reside in extrema0

        //extract the corners
        for(int i = 0; i < extrema0_sizes; i++) {
          int maximum = extrema0[i];
          corners.push_back(Point32f(x[maximum], y[maximum]));
        }

        delete x;
        delete y;
        delete smoothed_x;
        delete smoothed_y;
        delete curvature;
        delete gauss;
        delete extrema0;
        delete extrema1;
      }
      return corners;
    }
    template ICLCV_API const vector<Point32f> &CornerDetectorCSS::detectCorners(const vector<Point32f> &boundary);
    template ICLCV_API const vector<Point32f> &CornerDetectorCSS::detectCorners(const vector<Point> &boundary);

    template<class T>
    const vector<vector<utils::Point32f> > &CornerDetectorCSS::detectCorners(const vector<vector<T> > &boundaries, const vector<icl32f> &sigmas) {
      corners_list.clear();
      //calculate length of the data
      int array_length = 0;
      int *lengths = new int[boundaries.size()];
      int *indices = new int[boundaries.size()];
      int *indices_padded = new int[boundaries.size()];

      int gauss_length = 0;
      int *gauss_lengths = new int[boundaries.size()];
      int *gauss_indices = new int[boundaries.size()];

      for(unsigned int i = 0; i < boundaries.size(); i++) {
        indices[i] = array_length;
        indices_padded[i] = array_length + i * 4;
        lengths[i] = boundaries[i].size();
        array_length += lengths[i];

        gauss_indices[i] = gauss_length;
        gauss_lengths[i] = gauss_radius(sigmas[i], 0.0001) * 2 + 1;
        gauss_length += gauss_lengths[i];
      }
      //create needed arrays
      float *x = new float[array_length], *y = new float[array_length];
      float *smoothed_x = new float[array_length], *smoothed_y = new float[array_length];
      float *curvature = new float[array_length + boundaries.size() * 4];

      float *gauss = new float[gauss_length];

      int *extrema0 = new int[array_length];
      int *extrema0_sizes = new int[boundaries.size()];
      int *extrema1 = new int[array_length];
      int *extrema1_sizes = new int[boundaries.size()];

      //copy data into arrays
      for(unsigned int i = 0; i < boundaries.size(); i++) {
        if(gauss_lengths[i] < lengths[i]) {
          fill_gauss(&gauss[gauss_indices[i]],sigmas[i],gauss_lengths[i] / 2);
          for(unsigned int j = 0; j < boundaries[i].size();j++) {
            x[indices[i] + j] = boundaries[i][j].x;
            y[indices[i] + j] = boundaries[i][j].y;
          }
        }
      }
      for(unsigned int i = 0; i < boundaries.size(); i++) {
        if(gauss_lengths[i] < lengths[i]) {
          //copy values into local memory
          int length = lengths[i];
          int index = indices[i];
          int gauss_length = gauss_lengths[i];
          int gauss_index = gauss_indices[i];
          //smooth arrays
          convolute(x+index,length,gauss+gauss_index,gauss_length,smoothed_x+index);
          convolute(y+index,length,gauss+gauss_index,gauss_length,smoothed_y+index);
        }
      }
      //calculate curvature
      calculate_curvatures_bulk(array_length,boundaries.size(),lengths,indices,indices_padded,smoothed_x,smoothed_y,curvature_cutoff,curvature);
      for(unsigned int i = 0; i < boundaries.size(); i++) {
        if(gauss_lengths[i] < lengths[i]) {
          //copy values into local memory
          int length = lengths[i];
          int index = indices[i];
          int index_padded = indices_padded[i] + 2;
          int maxima_offset;
          //find extrema
          maxima_offset = findExtrema(extrema0+index,extrema0_sizes+i,curvature+index_padded,length);
          //remove round corners
          if(accurate)removeRoundCornersAccurate(rc_coeff, maxima_offset, curvature+index_padded, length, extrema0+index, extrema0_sizes[i], extrema1+index, extrema1_sizes+i);
          else removeRoundCorners(rc_coeff, maxima_offset, curvature+index_padded, length, extrema0+index, extrema0_sizes[i], extrema1+index, extrema1_sizes+i);
          //remove false corners
          removeFalseCorners(angle_thresh,smoothed_x+index,smoothed_y+index,curvature+index_padded,length,extrema1+index,extrema1_sizes[i],extrema0+index,extrema0_sizes+i);
        }else {
          extrema0_sizes[i] = 0;
        }
      }
      //the final corner indices reside in extrema0
      //extract the corners
      corners_list.reserve(boundaries.size());
      for(unsigned int i = 0; i < boundaries.size();i++) {
        corners_list.push_back(vector<Point32f>());
        corners_list.back().reserve(extrema0_sizes[i]);
        if(gauss_lengths[i] < lengths[i]) {
          for(int j = 0; j < extrema0_sizes[i]; j++) {
            int index = indices[i];
            int maximum = extrema0[index+j];
            corners_list.back().push_back(Point32f(x[index+maximum], y[index+maximum]));
          }
        }
      }

      delete x;
      delete y;
      delete smoothed_x;
      delete smoothed_y;
      delete curvature;
      delete gauss;
      delete extrema0;
      delete extrema0_sizes;
      delete extrema1;
      delete extrema1_sizes;

      delete lengths;
      delete indices;
      delete indices_padded;
      delete gauss_indices;

      return corners_list;
    }

    template ICLCV_API const vector<vector<utils::Point32f> > &CornerDetectorCSS::detectCorners(const vector<vector<Point32f> > &boundaries, const vector<icl32f> &sigmas);
    template ICLCV_API const vector<vector<utils::Point32f> > &CornerDetectorCSS::detectCorners(const vector<vector<Point> > &boundaries, const vector<icl32f> &sigmas);


    void CornerDetectorCSS::setPropertyValue(const std::string &propertyName, const Any &value) throw (ICLException){
      if(propertyName == "angle-threshold") angle_thresh = parse<float>(value);
      else if(propertyName == "rc-coefficient") rc_coeff = parse<float>(value);
      else if(propertyName == "sigma") sigma = parse<float>(value);
      else if(propertyName == "curvature-cutoff") curvature_cutoff = parse<float>(value);
      else if(propertyName == "straight-line-threshold") straight_line_thresh = parse<float>(value);
      else if(propertyName == "accurate") accurate = value == "on";
      else if(propertyName == "use opencl") useOpenCL = value == "on";
      else {
        ERROR_LOG("invalid property name " << propertyName);
      }
    }

    std::vector<std::string> CornerDetectorCSS::getPropertyList() const{
      static const std::vector<std::string> l = tok("angle-threshold,rc-coefficient,sigma,"
                                                    "curvature-cutoff,straight-line-threshold,"
                                                    "accurate,use opencl",",");
      return l;
    }

    std::string CornerDetectorCSS::getPropertyType(const std::string &propertyName) const{
      if(propertyName == "accurate" || propertyName == "use opencl") return "flag";
      return "range";
    }

    std::string CornerDetectorCSS::getPropertyInfo(const std::string &propertyName) const{
      if(propertyName == "angle-threshold") return "[0,180]";
      else if(propertyName == "rc-coefficient") return "[0,10]";
      else if(propertyName == "sigma") return "[1,20]";
      else if(propertyName == "curvature-cutoff") return "[0,1000]";
      else if(propertyName == "straight-line-threshold") return "[0,180]";
      else if(propertyName == "accurate" || propertyName == "use opencl") return "off,on";
      else return "undefined";
    }

    Any CornerDetectorCSS::getPropertyValue(const std::string &propertyName) const{
      if(propertyName == "angle-threshold") return str(angle_thresh);
      else if(propertyName == "rc-coefficient") return str(rc_coeff);
      else if(propertyName == "sigma") return str(sigma);
      else if(propertyName == "curvature-cutoff") return str(curvature_cutoff);
      else if(propertyName == "straight-line-threshold") return str(straight_line_thresh);
      else if(propertyName == "accurate") return accurate ? "on" : "off";
      else if(propertyName == "use opencl") return useOpenCL ? "on" : "off";
      else return "undefined";
    }

    std::string CornerDetectorCSS::getPropertyToolTip(const std::string &propertyName) const{
      if(propertyName == "angle-threshold"){
        return str( "denotes the maximum obtuse angle that a corner\n"
                    "can have when it is detected as a true corner,\n"
                    "default value is 162." );
      }else if(propertyName == "rc-coefficient"){
        return str( "denotes the minimum ratio of major axis to minor\n"
                    "axis of an ellipse, whose vertex could be detected\n"
                    "as a corner by proposed detector. The default\n"
                    "value is 1.5." );
      }else if(propertyName == "sigma"){
        return str( "denotes the standard deviation of the Gaussian\n"
                    "filter when computeing curvature. The default sig is 3");
      }else if(propertyName == "curvature-cutoff") {
        return str("cutoff for curvature values");
      }else if(propertyName == "straight-line-threshold"){
        return str("In order to estimate the angle of a corner, either a\n"
                   "circle or a straight line approximation of the left and\n"
                   "right surrounding is used. The straight line\n"
                   "approximation is used, if the angle between the\n"
                   "left neigbour, corner candidate and  and the point\n"
                   "on the contour half way between them is smaller than\n"
                   "straight_line_thresh. A value of 0 leads to circle \n"
                   "approximation only, 180 to straight line approximation\n"
                   "only.");
      }else if(propertyName == "accurate"){
        return str("Choose between using more complex sampling of the \n"
                   "boundary or using fast approximations.");
      }else if(propertyName == "use opencl"){
        return str("Choose whether to use opencl to compute curvature \n"
                   "(if opencl is not supported, this has no effect");
      }

      return "";
    }

    REGISTER_CONFIGURABLE_DEFAULT(CornerDetectorCSS);
  }
}
