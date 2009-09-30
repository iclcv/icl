#include "intrinsic-camera-calibration-tools.h"
#include <iclRandom.h>
#include <iclStochasticOptimizer.h>

namespace icl{
 

  static P64f distort_coords(const double dist_factor[4], 
                             const double ox, const double oy){

    double  z02, z0, p, q, z, px, py;
    int     i;

    px = ox - dist_factor[0];
    py = oy - dist_factor[1];
    p = dist_factor[2]/100000000.0;
    z02 = px*px+ py*py;
    q = z0 = sqrt(px*px+ py*py);

    for( i = 1; ; i++ ) {
      if( z0 != 0.0 ) {
        z = z0 - ((1.0 - p*z02)*z0 - q) / (1.0 - 3.0*p*z02);
        px = px * z / z0;
        py = py * z / z0;
      }
      else {
        px = 0.0;
        py = 0.0;
        break;
      }
#define  PD_LOOP   3
      if( i == PD_LOOP ) break;

      z02 = px*px+ py*py;
      z0 = sqrt(px*px+ py*py);
    }

    return P64f(px / dist_factor[3] + dist_factor[0],
                py / dist_factor[3] + dist_factor[1]);
  }

  static void distort_coords(const double dist_factor[4], 
                             const double ox, const double oy,
                             double *ix, double *iy){
    P64f p = distort_coords(dist_factor,ox,oy);
    *ix = p[0];
    *iy = p[1];
  }


  void calc_distortion(const CalibrationData &data, int xsize, int ysize, double dist_factor[4]){
    int     i, j;
    double  bx, by;
    double  bf[4];
    double  error, min;
    double  factor[4];

    bx = xsize / 2;
    by = ysize / 2;
    factor[0] = bx;
    factor[1] = by;
    factor[3] = 1.0;
    min = calc_distortion2(data,factor);
    bf[0] = factor[0];
    bf[1] = factor[1];
    bf[2] = factor[2];
    bf[3] = 1.0;
    printf("[%5.1f, %5.1f, %5.1f] %f\n", bf[0], bf[1], bf[2], min);

    // CE: changed ranges
    
    static const int MINV = -20;
    static const int MAXV = 20;
    for( j = MINV; j <= MAXV; j++ ) {
      factor[1] = by + j*5;
      for( i = MINV; i <= MAXV; i++ ) {
        factor[0] = bx + i*5;
        error = calc_distortion2( data, factor );
        if( error < min ) { 
          bf[0] = factor[0]; 
          bf[1] = factor[1];
          bf[2] = factor[2]; 
          min = error; 
        }
      }
      printf("[%5.1f, %5.1f, %5.1f] %f\n", bf[0], bf[1], bf[2], min);
    }

    bx = bf[0];
    by = bf[1];
    for( j = MINV; j <= MAXV; j++ ) {
      factor[1] = by + 0.5 * j;
      for( i = MINV; i <= MAXV; i++ ) {
        factor[0] = bx + 0.5 * i;
        error = calc_distortion2( data, factor );
        if( error < min ) { 
          bf[0] = factor[0]; 
          bf[1] = factor[1];
          bf[2] = factor[2]; 
          min = error; 
        }
      }
      printf("[%5.1f, %5.1f, %5.1f] %f\n", bf[0], bf[1], bf[2], min);
    }
    
    dist_factor[0] = bf[0];
    dist_factor[1] = bf[1];
    dist_factor[2] = bf[2];
    dist_factor[3] = get_size_factor( bf, xsize, ysize );
  }

  void calc_distortion_stochastic_search(const CalibrationData &data,
                                         int xsize, int ysize, double dist_factor[4]){
    struct StochasticIntrinsicCameraParameterOptimizer : public StochasticOptimizer<double>{
      double *dist_factor;
      const CalibrationData &data;
      Size size;
      double noise[4];
      StochasticIntrinsicCameraParameterOptimizer(double *dist_factor, const CalibrationData &data, int w, int h):
        StochasticOptimizer<double>(4),dist_factor(dist_factor),data(data),size(w,h){
        dist_factor[0] = size.width/2;
        dist_factor[1] = size.height/2;
        dist_factor[2] = 0.0;
        dist_factor[3] = 1.0;
      }
      virtual double *getData(){
        return dist_factor;
      }
    
      virtual double getError(const double *dist){
        return get_fitting_error(data, const_cast<double*>(dist));
      }
      
      virtual const double *getNoise(int currentTime, int endTime){
        std::fill(noise,noise+3,GRand(0,0.1));
        noise[3]=0;
        return noise;
      }
      virtual void reinitialize(){
        dist_factor[0] = size.width/2;
        dist_factor[1] = size.height/2;
        dist_factor[2] = 0.0;
        dist_factor[3] = 1.0;
      }
      /// a pure utility function, which can be implemented in derived classes to notify optization progress (somehow)
      virtual void notifyProgress(int t, int numSteps, int startError, 
                                  int currBestError, int currError,const  double *data, int dataDim){
        if(!(t%100)){
          
          std::cout << "t:" << t << " (" << 100*(float(t)/numSteps) << "%  error:" << currBestError << " dist-factor:{"
                    << dist_factor[0] << "," << dist_factor[1] << ","
                    << dist_factor[2] << "," << dist_factor[3] << "}" << std::endl;
        }
      }
    };
  
    StochasticIntrinsicCameraParameterOptimizer opt(dist_factor,data,xsize,ysize);
    StochasticOptimizer<double>::Result res = opt.optimize(100000);
    std::cout << "FINAL RESULT error:" << res.error << " dist-factor:{"
                    << dist_factor[0] << "," << dist_factor[1] << ","
                    << dist_factor[2] << "," << dist_factor[3] << "}" << std::endl;

    dist_factor[3] = get_size_factor( dist_factor, xsize, ysize );
  }


  double get_size_factor( double dist_factor[4], int xsize, int ysize){
    double  ox, oy, ix, iy;
    double  olen, ilen;
    double  sf, sf1;

    sf = 100.0;

    ox = 0.0;
    oy = dist_factor[1];
    olen = dist_factor[0];
    distort_coords( dist_factor, ox, oy, &ix, &iy );
    ilen = dist_factor[0] - ix;
    printf("Olen = %f, Ilen = %f\n", olen, ilen);
    if( ilen > 0 ) {
      sf1 = ilen / olen;
      if( sf1 < sf ) sf = sf1;
    }

    ox = xsize;
    oy = dist_factor[1];
    olen = xsize - dist_factor[0];
    distort_coords( dist_factor, ox, oy, &ix, &iy );
    ilen = ix - dist_factor[0];
    printf("Olen = %f, Ilen = %f\n", olen, ilen);
    if( ilen > 0 ) {
      sf1 = ilen / olen;
      if( sf1 < sf ) sf = sf1;
    }

    ox = dist_factor[0];
    oy = 0.0;
    olen = dist_factor[1];
    distort_coords(dist_factor, ox, oy, &ix, &iy );
    ilen = dist_factor[1] - iy;
    printf("Olen = %f, Ilen = %f\n", olen, ilen);
    if( ilen > 0 ) {
      sf1 = ilen / olen;
      if( sf1 < sf ) sf = sf1;
    }

    ox = dist_factor[0];
    oy = ysize;
    olen = ysize - dist_factor[1];
    distort_coords( dist_factor, ox, oy, &ix, &iy );
    ilen = iy - dist_factor[1];
    printf("Olen = %f, Ilen = %f\n", olen, ilen);
    if( ilen > 0 ) {
      sf1 = ilen / olen;
      if( sf1 < sf ) sf = sf1;
    }

    if( sf == 0.0 ) sf = 1.0;

    return sf;
  }

  double calc_distortion2(const CalibrationData &data,double dist_factor[4]){
    double    min, err, f, fb;
    int       i;

    dist_factor[2] = 0.0;
    min = get_fitting_error(data,dist_factor);

    static const int MINV = -10;
    static const int MAXV = 800; // orig: 200
    f = dist_factor[2];
    for( i = MINV; i < MAXV; i+=10 ) {
      dist_factor[2] = i;
      err = get_fitting_error( data, dist_factor );
      if( err < min ) { min = err; f = dist_factor[2]; }
    }

    
    fb = f;
    for( i = -10; i <= 10; i++ ) {
      dist_factor[2] = fb + i;
      if( dist_factor[2] < 0 ) continue;
      err = get_fitting_error( data, dist_factor );
      if( err < min ) { min = err; f = dist_factor[2]; }
    }

    fb = f;
    for( i = -10; i <= 10; i++ ) {
      dist_factor[2] = fb + 0.1 * i;
      if( dist_factor[2] < 0 ) continue;
      err = get_fitting_error( data, dist_factor );
      if( err < min ) { min = err; f = dist_factor[2]; }
    }

    dist_factor[2] = f;
    return min;
  }

  double get_fitting_error(const CalibrationData &data, double dist_factor[4]){
  
    int max = iclMax(data.nx,data.ny);
    std::vector<double> x(max),y(max);
  
    double error = 0.0;
    for(int i=0;i<data.nloops();i++){
      for(int j=0;j<data.ny;j++) {
        for(int k=0;k<data.nx;k++) {
          x[k] = data[i][j*data.nx+k].x;
          y[k] = data[i][j*data.nx+k].y;
        }
        error += check_error( x, y, data.nx, dist_factor );
      }
    
      for(int j=0;j<data.nx;j++){
        for(int k=0;k<data.ny;k++){
          x[k] = data[i][k*data.nx+j].x;
          y[k] = data[i][k*data.nx+j].y;
        }
        error += check_error(x,y,data.ny,dist_factor);
      }
    
      for(int j=3-data.ny;j<data.nx-2;j++){
        int p=0;
        for(int k=0;k<data.ny;k++){
          int l=j+k;
          if(l<0||l>=data.nx) continue;
          x[p] = data[i][k*data.nx+l].x;
          y[p] = data[i][k*data.nx+l].y;
          p++;
        }
        error += check_error(x,y,p,dist_factor);
      }
    
      for(int j=2;j<data.nx+data.ny-3;j++){
        int p=0;
        for(int k=0;k<data.ny;k++){
          int l=j-k;
          if(l<0||l>=data.nx)continue;
          x[p] = data[i][k*data.nx+l].x;
          y[p] = data[i][k*data.nx+l].y;
          p++;
        }
        error += check_error( x, y, p, dist_factor );
      }
    }
    return error;
  }


  void apply_pca(const std::vector<P64f> &input, M64f &evec, P64f &eval, P64f &mean){

    int num = (int)input.size();
    mean = 0.0;
    for(int i=0;i<num;++i){
      mean += input[i];
    }
    mean /= num;
  
    M64f C = 0.0;
    for(int i=0;i<num;++i){
      C += (input[i]-mean) * (input[i]-mean).transp();
    }
  
    int s2 = sizeof(icl64f);
    int s1 = 2 * s2;

    double buf[4];
    IppStatus s = ippmEigenValuesVectorsSym_m_64f(C.begin(),s1,s2,buf,evec.begin(),s1,s2,eval.begin(),2);
    if(s != ippStsNoErr){
      throw 0;
    }
 
    eval[0]/=(eval[0]+eval[1]);
  }

  double check_error(const std::vector<double> &x, const std::vector<double> &y, int num, double dist_factor[4]){
    std::vector<P64f> input;
    M64f evec;
    P64f ev,mean;


    for(int i=0;i<num;i++){
      input.push_back(distort_coords(dist_factor,x[i], y[i]));
    }
    try{
      apply_pca(input, evec, ev, mean);
      
      double a,b;
      if(ev[0] < ev[1]){
        a =  evec(0,0);
        b = evec(0,1);
      }else{
        a = evec(1,0);
        b = evec(1,1);
      }
      double c = -(a*mean[0] + b*mean[1]);
      
      double error = 0.0;
      for(int i=0;i<num;i++){
        error += ::pow(a*input[i][0] + b*input[i][1] + c,2);
      }
      error /= (a*a + b*b);
      
      return error;
    }catch(int i){
      return 0;
    }
  }
}
