#include <iclCornerDetectorCSS.h>
#include <iclMathematics.h>

using namespace std;

namespace icl{
  template<class T> inline static float sign(T x){
    return (x<0) ? -1 : (x>0) ? 1 : 0;
  }

  int CornerDetectorCSS::gaussian(icl32f **gau, float sigma, float cutoff) {
    float ssq = sigma*sigma;
    int width;
    for (width=1; exp(-(width*width)/(2*ssq)) > cutoff; width++);
    width = max(1, width-1);

    float sum = 0;
    *gau = (icl32f*)malloc(sizeof(icl32f)*(width*2+1));
    for (int i=-width; i<=width; i++) {
      (*gau)[i+width] = exp(-(i*i)/(2*ssq))/(2*M_PI*ssq);
      sum += (*gau)[i+width];
    }
    for (int i=0; i<width*2+1; i++) {
      (*gau)[i] /= sum;
    }
    return width;
  }

  void CornerDetectorCSS::findExtrema(vector<int> &extrema, icl32f* x, int length) {
    extrema.clear();
    float search = 1; // 1...pos. slope becomes neg. slope, 0... neg. slope becomes pos. slope

    for (int i=0; i<length-1; i++) {
      if ((x[i+1]-x[i])*search > 0) {
        extrema.push_back(i); // eextremaen indicies are minima, odd indicies are maxima
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

  const vector<Point32f> &CornerDetectorCSS::detectCorners(const vector<Point32f> &boundary) {
    //debug_output = true;
    corners.clear();
    int L=boundary.size();
    icl32f x_in[L], y_in[L];
    for (int i=0; i<L; i++) { x_in[i] = boundary[i].x; y_in[i] = boundary[i].y; }

    icl32f* h;
    int W = gaussian(&h, sigma, 0.0001); // W ... size of gauss kernel
    const int l2w = L+2*W;
    const int l4w = L+4*W;
    if (L <= W) return corners;

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
    ippsConv_32f(x, l2w, h, 2*W+1, xx_big);
    ippsConv_32f(y, l2w, h, 2*W+1, yy_big);
    icl32f *xx = &xx_big[W], *yy = &yy_big[W];

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
    for (int i=0; i<l2w; i++) {
      k[i] = abs((xu[i]*yuu[i] - xuu[i]*yu[i]) / pow((xu[i]*xu[i] + yu[i]*yu[i]),1.5f));
      k[i] = float(round(k[i]*curvature_cutoff))/curvature_cutoff; // very important to suppess noise
    }

    // get the maxima of the curvature
    findExtrema(extrema, k, l2w);

    // remove round corners
    removeRoundCorners(rc_coeff, k, extrema);

    // remove false corners due to boundary noise and trivial details
    removeFalseCorners(angle_thresh, xx, yy, k, l2w, extrema, corner_angles, straight_line_thresh);

    // extract the coordinates of the detected corners
    corners.clear();
    vector<float> angles_tmp;
    for (unsigned i=0; i<extrema.size(); i++) {
      if (extrema[i] >= W and extrema[i] < L+W) {
        corners.push_back(Point32f(x_in[extrema[i]-W], y_in[extrema[i]-W]));
        angles_tmp.push_back(corner_angles[i]);
      }
    }
    corner_angles = angles_tmp;
    return corners;
  }
}
