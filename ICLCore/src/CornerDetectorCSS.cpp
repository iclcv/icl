#include <ICLCore/CornerDetectorCSS.h>
#include <ICLCore/Mathematics.h>

using namespace std;

namespace icl{
  template<class T> inline static float sign(T x){
    return (x<0) ? -1 : (x>0) ? 1 : 0;
  }


  // just for beeing idependent from ipp, we have an inefficient implementation of 1D convolution here!
  void CornerDetectorCSS::convolute_1D(float *vec, int dim, float *kernel, int kernelDim, float *dst){
#ifdef HAVE_IPP
    ippsConv_32f(vec,dim,kernel,kernelDim,dst);
#else
    int dstLen = dim+kernelDim-1;
    for(int n=0;n<dstLen;++n){
      double buf = 0;
      for(int k=0;k<=n;++k){
        if(k<dim && (n-k)<kernelDim){
          buf += vec[k] * kernel[n-k];
        }
      }
      dst[n] = buf;
    }
#endif
  }



  int CornerDetectorCSS::gaussian(GaussianKernel &gauss, float sigma, float cutoff) {
    // do we need to recalculate the gaussian kernel?
    if ((sigma == gauss.sigma) && (cutoff == gauss.cutoff)) return gauss.width;
    // --yes, the parameters changed...
    gauss.sigma = sigma;
    gauss.cutoff = cutoff;
    float ssq = sigma*sigma;
    int width;
    for (width=1; exp(-(width*width)/(2*ssq)) > cutoff; width++);
    width = max(1, width-1);
    gauss.width = width;
    float sum = 0;
    // first free the old memory
    gauss.gau.resize(width*2+1);
    for (int i=-width; i<=width; i++) {
      gauss.gau[i+width] = exp(-(i*i)/(2*ssq))/(2*M_PI*ssq);
      sum += gauss.gau[i+width];
    }
    for (int i=0; i<width*2+1; i++) {
      gauss.gau[i] /= sum;
    }
    return width;
  }

  void CornerDetectorCSS::findExtrema(vector<int> &extrema, icl32f* x, int length) {
    extrema.clear();
    float search = 1; // 1...pos. slope becomes neg. slope, 0... neg. slope becomes pos. slope

    for (int i=0; i<length-1; i++) {
      if ((x[i+1]-x[i])*search > 0) {
        extrema.push_back(i); // even extrema indicies are minima, odd indicies are maxima
        search = -search; 
      }
    }
    if (extrema.size() % 2 == 0) extrema.push_back(length-1);
  }

  void CornerDetectorCSS::removeRoundCorners(float rc_coeff, icl32f* k, vector<int> &extrema) {
    vector<int> new_extrema;
    float mean;
    int n;
    for (unsigned i=1; i<extrema.size()-1; i+=2) {
      mean = 0; n = 0;
      // get mean curvature value over ROS (region of support)
      // here the region of support searches for first minimum value both left and right each maximum
      for (int j=extrema[i]-1; k[j]>k[extrema[i-1]]; j--) { mean += k[j]; n++; }
      for (int j=extrema[i]+1; k[j]>k[extrema[i+1]]; j++) { mean += k[j]; n++; }
      mean /= n;
      if (k[extrema[i]] >= rc_coeff*mean) {
        new_extrema.push_back(extrema[i]);
      }
    }
    extrema = new_extrema;
  }

 

  float modulo(float x, float v) {
    while (x < 0) x+= v;
    while (x >= v) x-= v;
    return x;
  }

  float CornerDetectorCSS::tangentAngle(icl32f* x, icl32f* y, int length, int candidate, float straight_line_thresh) {
    int last, first, middle, middle2;
    float x0,y0,x1,x2,x3,y1,y2,y3;
    float tangent_direction;
    float diff_angle;
    float direction[2];
    for (int leftright=0; leftright<2; leftright++) {
      first = candidate;
      if (leftright == 0) last = 0; // left tangent
      else last = length-1; // right tangent
    
      if (abs(first-last)>3) {
        if ((x[first] != x[last]) || (y[first] != y[last])) {
          middle = (last == 0) ? candidate/2 : candidate + (length-candidate+1)/2;
          middle2 = -1;
        } else {
          middle = (last == 0) ? candidate/3 : candidate + (length-candidate+1)/3;
          middle2 = (last == 0) ? 2*candidate/3 : candidate + 2*(length-candidate+1)/3;
        }
        x1 = x[first]; y1 = y[first];
        x2 = x[middle]; y2 = y[middle];
        if (middle2 == -1) { x3 = x[last]; y3 = y[last]; }
        else { x3 = x[middle2]; y3 = y[middle2]; }

        diff_angle = modulo((atan2(y3-y1,x3-x1) - atan2(y2-y1,x2-x1)), 2*M_PI);
        if (diff_angle>M_PI) diff_angle = 2*M_PI-diff_angle;
        if (diff_angle*180/M_PI < max(straight_line_thresh, 0.01f)) { // fit straight line
          tangent_direction = atan2(y[last]-y[first], x[last]-x[first]);
        } else { // fit a circle
          x0 = 0.5*((x1*x1+y1*y1)*(y2-y3) + (x2*x2+y2*y2)*(y3-y1) + (x3*x3+y3*y3)*(y1-y2)) /
          (x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2));
          y0 = 0.5*((x1*x1+y1*y1)*(x2-x3) + (x2*x2+y2*y2)*(x3-x1) + (x3*x3+y3*y3)*(x1-x2)) /
          (y1*(x2-x3) + y2*(x3-x1) + y3*(x1-x2));
          float radius_dir = atan2(y0-y1, x0-x1);
          float adjacent_dir = atan2(y2-y1, x2-x1);
          tangent_direction = sign(sin(adjacent_dir-radius_dir))*M_PI/2. + radius_dir;
        }
      } else { // very short line
        tangent_direction = atan2(y[last]-y[first], x[last]-x[first]);
      }
      direction[leftright] = tangent_direction*180./M_PI;
    }
    float angle = direction[0]-direction[1];
    return (angle < 0) ? angle+360 : (angle > 360) ? angle-360 : angle;
  }

  void CornerDetectorCSS::removeFalseCorners(float angle_thresh, icl32f* x, icl32f* y, icl32f* k, 
                                             int length, vector<int> &maxima, vector<float> &corner_angles, 
                                             float straight_line_thresh) {
    vector<int> new_maxima;
    bool has_changed;
    float angle;
    int i_0; // start index of boundary segment
    int l;   // length of boundary segment
    int i_ext; // postion of maxima in boundary segment
    do {
      new_maxima.clear();
      corner_angles.clear();
      has_changed = false;
      for (unsigned i=0; i<maxima.size(); i++) {
        if ((i==0) && (maxima.size() == 1)) {
          i_0 = 0;
          l = length;
          i_ext = maxima[i];
        } else if (i==0) {
          i_0 = 0;
          l = maxima[i+1]+1;
          i_ext = maxima[i];
        } else if (i==maxima.size()-1) {
          i_0 = maxima[i-1];
          l = length-maxima[i-1];
          i_ext = maxima[i]-maxima[i-1];
        } else {
          i_0 = maxima[i-1];
          l = maxima[i+1]-maxima[i-1]+1;
          i_ext = maxima[i] - maxima[i-1];
        }
        angle = tangentAngle(&x[i_0], &y[i_0], l, i_ext, straight_line_thresh);
        if ((angle<=angle_thresh) || (angle>=360.-angle_thresh)) {
          new_maxima.push_back(maxima[i]);
          corner_angles.push_back(angle);
        } else has_changed = true;
      }
      maxima = new_maxima;
    } while (has_changed);
  }

  const std::vector<Point32f> &CornerDetectorCSS::detectCorners(const std::vector<Point> &boundary){
    inputBuffer.resize(boundary.size());
    std::copy(boundary.begin(),boundary.end(),inputBuffer.begin());
    return detectCorners(inputBuffer);
  }
  
  void atov(vector<Point32f> &v, icl32f *x, icl32f *y, int length) {
  	v.clear();
  	for (int i=0; i < length; i++) v.push_back(Point32f(x[i],y[i]));
  }

  const vector<Point32f> &CornerDetectorCSS::detectCorners(const vector<Point32f> &boundary) {
    //debug_output = true;
    if (debug_mode) {
    	debug_inf.boundary = boundary;
    	debug_inf.angle_thresh = angle_thresh;
     	debug_inf.rc_coeff = rc_coeff;
    	debug_inf.straight_line_thresh = straight_line_thresh;
    }
    corners.clear();
    
    // Copy first half to the end and second half to the beginning of the
    // boundary, so we become independent from where the start/end point of the
    // boundary is.
    int n = boundary.size();
    int offset = n/2;
    int L = n + 2*offset;
    icl32f x_in[L], y_in[L];
    for (int i=0; i<offset; i++) { x_in[i] = boundary[n-offset+i].x; y_in[i] = boundary[n-offset+i].y; }
    for (int i=0; i<n; i++) { x_in[offset+i] = boundary[i].x; y_in[offset+i] = boundary[i].y; }
    for (int i=0; i<offset; i++) { x_in[offset+n+i] = boundary[i].x; y_in[offset+n+i] = boundary[i].y; }
    

    int W = gaussian(m_gauss, sigma, 0.0001); // W ... size of gauss kernel
    if (debug_mode) {
    	debug_inf.gk = m_gauss;
    	debug_inf.offset = W+offset;
    }
    if (L <= W) return corners;
    icl32f* h = m_gauss.gau.data();
    const int l2w = L+2*W;
    const int l4w = L+4*W;
    
    // closed curve -> copy end points to begin and begin points to end
    icl32f x[l2w], y[l2w];
    memcpy(x, &x_in[L-W], W*sizeof(icl32f));
    memcpy(&x[W], x_in, L*sizeof(icl32f));
    memcpy(&x[L+W], x_in, W*sizeof(icl32f));
    memcpy(y, &y_in[L-W], W*sizeof(icl32f));
    memcpy(&y[W], y_in, L*sizeof(icl32f));
    memcpy(&y[L+W], y_in, W*sizeof(icl32f));

    // do the convolution
    icl32f xx_big[l4w], yy_big[l4w];
    convolute_1D(x, l2w, h, 2*W+1, xx_big);
    convolute_1D(y, l2w, h, 2*W+1, yy_big);
    icl32f *xx = &xx_big[W], *yy = &yy_big[W];
    if (debug_mode) atov(debug_inf.smoothed_boundary, xx, yy, l2w);

    // calculate the first derivation
    icl32f xu[l2w], yu[l2w];
    xu[0] = xx[1]-xx[0];
    for (int i=1; i<l2w-1; i++) xu[i] = (xx[i+1]-xx[i-1])/2;
    xu[l2w-1] = xx[l2w-1]-xx[l2w-2];
    yu[0] = yy[1]-yy[0];
    for (int i=1; i<l2w-1; i++) yu[i] = (yy[i+1]-yy[i-1])/2;
    yu[l2w-1] = yy[l2w-1]-yy[l2w-2];

    // calculate the second derivation
    icl32f xuu[l2w], yuu[l2w];
    xuu[0] = xu[1]-xu[0];
    for (int i=1; i<l2w-1; i++) xuu[i] = (xu[i+1]-xu[i-1])/2;
    xuu[l2w-1] = xu[l2w-1]-xu[l2w-2];
    yuu[0] = yu[1]-yu[0];
    for (int i=1; i<l2w-1; i++) yuu[i] = (yu[i+1]-yu[i-1])/2;
    yuu[l2w-1] = yu[l2w-1]-yu[l2w-2];

    // calculate the curvature values
    icl32f k[l2w];
    if (debug_mode) debug_inf.kurvature.clear();
    for (int i=0; i<l2w; i++) {
      k[i] = abs((xu[i]*yuu[i] - xuu[i]*yu[i]) / pow((xu[i]*xu[i] + yu[i]*yu[i]),1.5f));
      k[i] = float(round(k[i]*curvature_cutoff))/curvature_cutoff; // very important to suppess noise
      if (debug_mode) debug_inf.kurvature.push_back(k[i]);
    }

    // get the maxima of the curvature
    findExtrema(extrema, k, l2w);
    if (debug_mode) {
    	debug_inf.extrema.clear();
	    for (unsigned i=0; i<extrema.size(); i++) {
	      if (extrema[i] >= offset+W and extrema[i] < L-offset+W)
	        debug_inf.extrema.push_back(extrema[i]);
	    }
	    debug_inf.maxima.clear();
	    for (unsigned i=1; i<extrema.size()-1; i+=2) {
	      if (extrema[i] >= offset+W and extrema[i] < L-offset+W)
	        debug_inf.maxima.push_back(extrema[i]);
	    }
    }

    // remove round corners
    removeRoundCorners(rc_coeff, k, extrema);
    if (debug_mode) {
    	debug_inf.maxima_without_round_corners.clear();
	    for (unsigned i=0; i<extrema.size(); i++) {
	      if (extrema[i] >= offset+W and extrema[i] < L-offset+W)
	        debug_inf.maxima_without_round_corners.push_back(extrema[i]);
	    }
    }

    // remove false corners due to boundary noise and trivial details
    removeFalseCorners(angle_thresh, xx, yy, k, l2w, extrema, corner_angles, straight_line_thresh);
    if (debug_mode) {
    	debug_inf.maxima_without_false_corners.clear();
	    for (unsigned i=0; i<extrema.size(); i++) {
	      if (extrema[i] >= offset+W and extrema[i] < L-offset+W)
	        debug_inf.maxima_without_false_corners.push_back(extrema[i]);
	    }
    }
    
    // extract the coordinates of the detected corners
    corners.clear();
    vector<float> angles_tmp;
    for (unsigned i=0; i<extrema.size(); i++) {
      if (extrema[i] >= offset+W and extrema[i] < L-offset+W) {
        corners.push_back(Point32f(x_in[extrema[i]-W], y_in[extrema[i]-W]));
        angles_tmp.push_back(corner_angles[i]);
      }
    }
    corner_angles = angles_tmp;
    if (debug_mode) {
    	debug_inf.corners = corners;
    	debug_inf.angles = corner_angles;
    }
    return corners;
  }
}
