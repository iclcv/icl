#include <iclCommon.h>
#include <iclCamera.h>
#include <iclFixedVector.h>
#include <fstream>

void get_angles(const FixedMatrix<float,3,3> &M, float &rx, float &ry, float &rz){
  rx = ::asin(M(1,2));
  float cosx = ::cos(rx);
  ry = ::acos(M(2,2)/cosx);
  rz = ::acos(M(1,1)/cosx);
}

const char *inifile[116] = {
  "# SVS Engine v 3.2 Stereo Camera Parameter File",
  "",
  "[image]",
  "max_linelen 640", 
  "max_lines 480", 
  "max_decimation 1", 
  "max_binning 1", 
  "max_framediv 1", 
  "gamma 0",
  "color_right 0", 
  "color 1", 
  "ix 0", 
  "iy 0", 
  "vergence 0.000000", 
  "rectified 1", 
  "width 640", 
  "height 480", 
  "linelen 640", 
  "lines 480", 
  "decimation 1", 
  "binning 1", 
  "framediv 0", 
  "subwindow 0", 
  "have_rect 1", 
  "autogain 0", 
  "autoexposure 0", 
  "autowhite 0", 
  "autobrightness 0", 
  "gain 50", 
  "exposure 50", 
  "contrast 50", 
  "brightness 50", 
  "saturation 50",
  "red 0", 
  "blue 0", 
  "",
  "[stereo]",
  "convx 11", 
  "convy 11",
  "corrxsize 15", 
  "corrysize 15",
  "thresh 7", 
  "unique 10", 
  "lr 1", 
  "ndisp 32", 
  "dpp 16", 
  "offx 0", 
  "offy 0", 
  "frame 1.050000", 
  "",
  "[external]",
  "Tx -88.915959",
  "Ty -0.157128", 
  "Tz 4.306600", 
  "Rx 0.000422", 
  "Ry 0.005006", 
  "Rz 0.002866", 
  "",
  "[left camera]",
  "pwidth 640", 
  "pheight 480", 
  "dpx 0.007000", 
  "dpy 0.007000", 
  "sx 1.000000", 
  "Cx 306.526995", 
  "Cy 286.809716", 
  "f 511.363092", 
  "fy 513.712339", 
  "alpha 0.000000", 
  "kappa1 -0.159408", 
  "kappa2 0.161214", 
  "kappa3 0.000000", 
  "tau1 0.000000", 
  "tau2 0.000000", 
  "proj", 
  "5.140000e+002 0.000000e+000 3.065270e+002 0.000000e+000", 
  "0.000000e+000 5.140000e+002 2.757325e+002 0.000000e+000", 
  "0.000000e+000 0.000000e+000 1.000000e+000 0.000000e+000", 
  "rect", 
  "9.990557e-001 -1.115309e-003 -4.343369e-002", 
  "1.107551e-003 9.999993e-001 -2.026861e-004", 
  "4.343389e-002 1.543897e-004 9.990563e-001", 
  "",
  "[right camera]",
  "pwidth 640", 
  "pheight 480", 
  "dpx 0.007000", 
  "dpy 0.007000", 
  "sx 1.000000", 
  "Cx 323.916494", 
  "Cy 264.655358", 
  "f 521.225593", 
  "fy 523.525160", 
  "alpha 0.000000", 
  "kappa1 -0.152761", 
  "kappa2 0.142905", 
  "kappa3 0.000000", 
  "tau1 0.000000", 
  "tau2 0.000000", 
"proj", 
  "5.140000e+002 0.000000e+000 3.239165e+002 -4.570280e+004", 
  "0.000000e+000 5.140000e+002 2.757325e+002 0.000000e+000", 
  "0.000000e+000 0.000000e+000 1.000000e+000 0.000000e+000", 
"rect", 
  "9.988248e-001 1.767150e-003 -4.843450e-002", 
  "-1.758498e-003 9.999985e-001 2.212436e-004", 
  "4.843481e-002 -1.358116e-004 9.988263e-001", 
  "",
  "[global]",
  "GTx 0.000000", 
  "GTy 0.000000", 
  "GTz 0.000000", 
  "GRx 0.000000", 
  "GRy 0.000000", 
  "GRz 0.000000",
  0 
};

int main(int n, char **ppc){
  pa_explain("-GRx","global x-rotation");
  pa_explain("-GRy","global y-rotation");
  pa_explain("-GRz","global z-rotation");
  pa_explain("-GTx","global x-translation");
  pa_explain("-GTy","global y-translation");
  pa_explain("-GTz","global z-translation");

  pa_explain("-convx","stereo matching prefilter kernel size (in x-direction)");
  pa_explain("-convy","stereo matching prefilter kernel size (in y-direction)");
  pa_explain("-corrxsize","width of correlation search window");
  pa_explain("-corrysize","height of correlation search window");
  pa_explain("-dpp","subpixel interpolation (do not change!)");
  pa_explain("-frame","frame expansion scaling factor");
  pa_explain("-left","defined filename for left cameras xml-config file");
  pa_explain("-right","defined filename for rightcameras xml-config file");
  pa_explain("-left-Cx","distorsion center for left camera (x)");
  pa_explain("-right-Cx","distorsion center for right camera (x)");
  pa_explain("-left-Cy","distorsion center for left camera (y)");
  pa_explain("-right-Cy","distorsion center for right camera (y)");

  
  pa_explain("-left-alpha","intrinsic skew parameter (only for analog cams)");
  pa_explain("-right-alpha","intrinsic skew parameter (only for analog cams)");
  pa_explain("-left-dpx","effective pixel size in x-direction for current resolution");
  pa_explain("-right-dpx","effective pixel size in x-direction for current resolution");

  pa_explain("-left-dpy","effective pixel size in y-direction for current resolution");
  pa_explain("-right-dpy","effective pixel size in y-direction for current resolution");

  pa_explain("-left-kappa1","first radial distortion parameter");
  pa_explain("-right-kappa1","first radial distortion parameter");

  pa_explain("-left-kappa2","2nd radial distortion parameter");
  pa_explain("-right-kappa2","2nd radial distortion parameter");

  pa_explain("-left-kappa3","3rd radial distortion parameter");
  pa_explain("-right-kappa3","3rd radial distortion parameter");

  pa_explain("-left-sx","aspect ratio (analog cameras only)");
  pa_explain("-right-sx","aspect ratio (analog cameras only)");

  pa_explain("-left-tau1","first tangential distortion pamrameter");
  pa_explain("-right-tau1","first tangential distortion pamrameter");

  pa_explain("-left-tau2","2nd tangential distortion pamrameter");
  pa_explain("-right-tau2","2nd tangential distortion pamrameter");

  pa_explain("-lr","left/right filter on (1) or off (0)");
  pa_explain("-ndisp","number of disparities to search");
  pa_explain("-offx","horopter x-offset");
  pa_explain("-offy","vertical image offset, not used");
  pa_explain("-thresh","confidence threshold value");
  pa_explain("-unique","unknown and not documented!");

  pa_explain("-o","define output filename (/dev/stdout at default)");


  pa_init(n,ppc,"-left(1) -right(1) -o(1) -convx(1) -convy(1) -corrxsize(1) -corrysize(1) "
          "-thresh(1) -unique(1) -lr(1) -ndisp(1) -dpp(1) -offx(1) -offy(1) -frame(1) "
          "-left-dpx(1) -right-dpx(1) -left-dpy(1) -right-dpy(1) -left-sx(1) -right-sx(1) "
          "-left-Cx(1) -right-Cx(1) -left-Cy(1) -right-Cy(1) -left-alpha(1) -right-alpha(1) "
          "-left-kappa1(1) -right-kapa1(1) -left-kappa2(1) -right-kappa2(1) -left-kappa3(1) "
          "-right-kappa3(1) -left-tau1(1) -right-tau1(1) -left-tau2(1) -right-tau2(1) "
          "-GTx(1) -GTy(1) -GTz(1) -GRx(1) -GRy(1) -GRz(1)");

  if(pa_defined("-test")){
    FixedMatrix<float,3,3> R1(9.990557e-001,-1.115309e-003,-4.343369e-002, 
                             1.107551e-003,9.999993e-001,-2.026861e-004, 
                             4.343389e-002,1.543897e-004,9.990563e-001);

    FixedMatrix<float,3,3> R2(9.988248e-001,1.767150e-003,-4.843450e-002, 
                              -1.758498e-003,9.999985e-001,2.212436e-004, 
                              4.843481e-002,-1.358116e-004,9.988263e-001);
    
    float x2,y2,z2;
    float x1,y1,z1;
    
    get_angles(R1.transp(),x1,y1,z1);
    get_angles(R2.transp(),x2,y2,z2);

    std::cout << "R:" << 0.000422 << "\t" << 0.005006 << "\t" << 0.002866 << std::endl;
    std::cout << "1:" << x1 << "\t" << y1 << "\t" << z1 << std::endl;
    std::cout << "2:" << x2 << "\t" << y2 << "\t" << z2 << std::endl;
    std::cout << "xx" << x2-x1 << "\t" << y2-y1 << "\t" << z1+z2 << std::endl;
    //std::cout << "+:" << x1+x2 << "\t" << y1+y2 << "\t" << z1+z2 << std::endl;
    //std::cout << "-:" << x1-x2 << "\t" << y1-y2 << "\t" << z1-z2 << std::endl;
    //std::cout << "||" << abs(x1)+abs(x2) << "\t" << abs(y1)+abs(y2) << "\t" << abs(z1)+abs(z2) << std::endl;
    
    return 0;
  }
  
  if(!pa_defined("-left") || !pa_defined("-right")){
    pa_usage("parameters -left and -right are mandatory!");
    exit(-1);
  }

  Camera left(pa_subarg<std::string>("-left",0,""));
  Camera right(pa_subarg<std::string>("-right",0,""));
  
  FixedMatrix<float,3,3> L,R;
  L.col(0) = left.getNorm().part<0,0,1,3>();
  L.col(1) = left.getUp().part<0,0,1,3>();
  L.col(2) = left.getHorz().part<0,0,1,3>();
  
  R.col(0) = right.getNorm().part<0,0,1,3>();
  R.col(1) = right.getUp().part<0,0,1,3>();
  R.col(2) = right.getHorz().part<0,0,1,3>();

  FixedMatrix<float,3,3> T = R*L.transp();

  float rx,ry,rz;
  get_angles(T,rx,ry,rz);

  Vec d = left.getPos()-right.getPos();
  float dx = d[0];
  float dy = d[1];
  float dz = d[2];

  //std::cout << "rot: \n" << create_rot_3D<float>(0.000422,0.005006,0.002866) << std::endl;
  
  std::ofstream os(pa_subarg<std::string>("-o",0,"/dev/stdout").c_str());
  
  ICLASSERT_THROW(left.getViewPort() == right.getViewPort(),ICLException("view port sizes of left and right cam must be equal"));
  
  int iW = left.getViewPort().width;
  int iH = left.getViewPort().height;
  
  std::vector<std::string> params(inifile,inifile+114);
  std::string scope = "none";
  for(unsigned int i=0;i<params.size();++i){
    std::string &p = params[i];
    
    if(!p.size()){  
      os << std::endl; 
      continue;
    }
    else if( p[0]=='#') {
      os << p << std::endl; 
      continue;
    }

    if(p[0]=='['){
      scope = p.substr(1,p.size()-2);
      os << p << std::endl;
      continue;
    }
    std::vector<std::string> ts = tok(p," ");
    if(ts.size() == 2){
      std::string &pa = ts[0];
      //std::string &va = ts[1];

#define REPLACE(s,x,v) if(scope == #s && pa == #x) { os << pa << " " << v << std::endl; continue; }
#define REPLACE_PA(s,x,d) if(scope == #s && pa == #x) { os << pa << " " << pa_subarg<std::string>("-" #x,0,str(#d)) << std::endl; continue; }

#define REPLACE_PA_LRCAM(x,d) \
      if(scope == "left camera" && pa == #x) { os << pa << " " << pa_subarg<std::string>("-left-" #x,0,str(d)) << std::endl; continue; } \
      else if(scope == "right camera" && pa == #x){ os << pa << " " << pa_subarg<std::string>("-right-" #x,0,str(d)) << std::endl; continue; }
      
      REPLACE(image,max_linelen,iW);
      REPLACE(image,max_lines,iH);
      REPLACE(image,width,iW);
      REPLACE(image,height,iH);
      REPLACE(image,linelen,iW);
      REPLACE(image,lines,iH);
      
      REPLACE_PA(stereo,convx,11);
      REPLACE_PA(stereo,convy,11);
      REPLACE_PA(stereo,corrxsize,15);
      REPLACE_PA(stereo,corrysize,15);
      REPLACE_PA(stereo,thresh,7);
      REPLACE_PA(stereo,unique,10);
      REPLACE_PA(stereo,lr,1);
      REPLACE_PA(stereo,ndisp,32);
      REPLACE_PA(stereo,dpp,16);
      REPLACE_PA(stereo,offx,0);
      REPLACE_PA(stereo,offy,0);
      REPLACE_PA(stereo,rame,1.05000);

      REPLACE(external,Tx,dx);
      REPLACE(external,Ty,dy);
      REPLACE(external,Tz,dz);

      REPLACE(external,Rx,rx);
      REPLACE(external,Ry,ry);
      REPLACE(external,Rz,rz);

      REPLACE(left camera,pwidth,iW);
      REPLACE(left camera,pheight,iH);
      REPLACE(right camera,pwidth,iW);
      REPLACE(right camera,pheight,iH);

      REPLACE_PA_LRCAM(dpx,0.007);
      REPLACE_PA_LRCAM(dpy,0.007);
      REPLACE_PA_LRCAM(sx,1.0000);
      REPLACE_PA_LRCAM(Cx,iW/2);
      REPLACE_PA_LRCAM(Cy,iH/2);
      
      REPLACE(left camera,f,left.getFocalLength()/pa_subarg<float>("-left-dpx",0,0.007));
      REPLACE(right camera,f,right.getFocalLength()/pa_subarg<float>("-right-dpx",0,0.007));

      REPLACE(left camera,fy,left.getFocalLength()/pa_subarg<float>("-left-dpy",0,0.007));
      REPLACE(right camera,fy,right.getFocalLength()/pa_subarg<float>("-right-dpy",0,0.007));


      REPLACE_PA_LRCAM(alpha,0.0);
      REPLACE_PA_LRCAM(kappa1,0.0);
      REPLACE_PA_LRCAM(kappa2,0.0);
      REPLACE_PA_LRCAM(kappa3,0.0);
      REPLACE_PA_LRCAM(tau1,0.0);
      REPLACE_PA_LRCAM(tau2,0.0);

      REPLACE_PA(global,GTx,0);
      REPLACE_PA(global,GTy,0);
      REPLACE_PA(global,GTz,0);

      REPLACE_PA(global,GRx,0);
      REPLACE_PA(global,GRy,0);
      REPLACE_PA(global,GRz,0);

      os << p << std::endl;
      
    }else if (ts.size()){
      Camera &cam = (scope == "left camera") ? left : right;
      float fy = cam.getFocalLength()/pa_subarg<float>("-left-dpy",0,0.007);
      float Cx = pa_subarg<float>((scope == "left camera") ? "-left-Cx" : "-right-Cx", 0, iW/2);
      float Cy = pa_subarg<float>((scope == "left camera") ? "-left-Cx" : "-right-Cx", 0, iW/2);

      if(ts[0] == "proj"){
        i+=3;
        os << "proj" << std::endl;
        os << fy  << " " << 0.0 << " "  << Cx  << " " << 0.0 << std::endl;
        os << 0.0 << " " << fy  << " "  << Cy  << " " << 0.0 << std::endl;
        os << 0.0 << " " << 0.0  << " " << 1.0 << " " << 0.0 << std::endl;
      }else if(ts[0] == "rect"){
        os << "rect" << std::endl;
        i+=3;
        /// first simple approach: right camera is (rot=0,0,0)
        if(scope == "left camera"){
          // here we need the rotation from left cam to the right cam ...
          FixedMatrix<float,3,3> T = create_rot_3D<float>(rx,ry,rz).transp();
          for(int y=0;y<3;++y){
            os << T(0,y) << " " << T(1,y) << " "<< T(2,y) << std::endl;
          }
        }else{
          os << "1 0 0" << std::endl;
          os << "0 1 0" << std::endl;
          os << "0 0 1" << std::endl;
        }
      }
    }else{
      os << std::endl;
    }  
  }


}
