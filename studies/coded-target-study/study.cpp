// Calibration-target detectability / homography-quality study.
//
// Analytic projective renderer (exact ground-truth corner pixels) + the real
// detection pipeline, swept over target type / cell count / marker size /
// distortion / pose. See study.md for the design.
//
// Build (from builddir): see run.sh — compiles against the built dylibs.

#include <icl/core/Img.h>
#include <icl/core/Image.h>
#include <icl/io/SaveLoad.h>
#include <icl/markers/CodedCheckerboardTarget.h>
#include <icl/markers/CheckerboardTarget.h>
#include <icl/markers/CalibrationTarget.h>
#include <icl/markers/FiducialDetector.h>
#include <icl/markers/BCHCode.h>
#include <icl/filter/conv/ConvolutionOp.h>
#include <icl/cv/IntrinsicCalibrator.h>
#include <icl/math/la/DynMatrix.h>
#include <icl/math/transform/Homography2D.h>
#include <cmath>
#include <cstdio>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

using namespace icl::core;
using namespace icl::markers;
using namespace icl::utils;
using namespace icl::filter;
using icl::icl8u;
using icl::icl64f;

// ------------------------------------------------------------- small 3x3 math
struct M3 { double m[9]; double &operator()(int r,int c){return m[r*3+c];} double operator()(int r,int c)const{return m[r*3+c];} };
static M3 mul(const M3&a,const M3&b){ M3 o{}; for(int r=0;r<3;r++)for(int c=0;c<3;c++){double s=0;for(int k=0;k<3;k++)s+=a(r,k)*b(k,c); o(r,c)=s;} return o; }
struct V3 { double x,y,z; };
static V3 mul(const M3&a,const V3&v){ return { a(0,0)*v.x+a(0,1)*v.y+a(0,2)*v.z, a(1,0)*v.x+a(1,1)*v.y+a(1,2)*v.z, a(2,0)*v.x+a(2,1)*v.y+a(2,2)*v.z }; }
static M3 rotX(double a){ double c=cos(a),s=sin(a); return {1,0,0, 0,c,-s, 0,s,c}; }
static M3 rotY(double a){ double c=cos(a),s=sin(a); return {c,0,s, 0,1,0, -s,0,c}; }
static M3 rotZ(double a){ double c=cos(a),s=sin(a); return {c,-s,0, s,c,0, 0,0,1}; }

// -------------------------------------------------------------- camera + pose
struct Cam { double f, cx, cy, k1, k2; int W, H; };
struct Pose { M3 R; V3 t; };

// board-mm (X,Y,0) -> distorted pixel (analytic ground truth)
static bool projectGT(double X, double Y, const Pose &p, const Cam &cam, double &u, double &v){
  V3 P = { p.R(0,0)*X + p.R(0,1)*Y + p.t.x,
           p.R(1,0)*X + p.R(1,1)*Y + p.t.y,
           p.R(2,0)*X + p.R(2,1)*Y + p.t.z };
  if(P.z <= 1e-6) return false;
  double x = P.x/P.z, y = P.y/P.z;
  double r2 = x*x + y*y, d = 1.0 + cam.k1*r2 + cam.k2*r2*r2;
  u = cam.f*x*d + cam.cx;  v = cam.f*y*d + cam.cy;
  return true;
}

// texture sampling helper (bilinear, gray)
static inline float sampleTex(const Channel8u &t, int tw, int th, float fx, float fy){
  if(fx<0||fy<0||fx>=tw-1||fy>=th-1) return -1.f;   // outside board texture
  int x=(int)fx, y=(int)fy; float ax=fx-x, ay=fy-y;
  float a=t(x,y), b=t(x+1,y), c=t(x,y+1), d=t(x+1,y+1);
  return (a*(1-ax)+b*ax)*(1-ay) + (c*(1-ax)+d*ax)*ay;
}

// Render the board plane into a distorted camera image (inverse map: for each
// output pixel, undistort -> ray -> intersect board plane -> sample texture).
// tex is generate()'s output; (texPxPerCell, ox, oy) map board-mm to texture-px.
static Img8u renderView(const Img8u &tex, int C, int R, double sq,
                        double texPxPerCell, double ox, double oy,
                        const Pose &p, const Cam &cam, int bg){
  const int tw = tex.getWidth(), th = tex.getHeight();
  Channel8u t = const_cast<Img8u&>(tex)[0];
  Img8u out(Size(cam.W, cam.H), 1);
  Channel8u o = out[0];
  const V3 r1{p.R(0,0),p.R(1,0),p.R(2,0)}, r2{p.R(0,1),p.R(1,1),p.R(2,1)};
  for(int py=0; py<cam.H; ++py){
    for(int px=0; px<cam.W; ++px){
      double xd=(px-cam.cx)/cam.f, yd=(py-cam.cy)/cam.f;
      double x=xd, y=yd;                                    // undistort (iterative)
      for(int it=0; it<6; ++it){ double r2v=x*x+y*y, k=1.0+cam.k1*r2v+cam.k2*r2v*r2v; x=xd/k; y=yd/k; }
      // solve X*r1 + Y*r2 - L*(x,y,1) = -t  for (X,Y,L)
      double A[3][3]={ {r1.x, r2.x, -x}, {r1.y, r2.y, -y}, {r1.z, r2.z, -1} };
      double b[3]={ -p.t.x, -p.t.y, -p.t.z };
      double det = A[0][0]*(A[1][1]*A[2][2]-A[1][2]*A[2][1])
                 - A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0])
                 + A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]);
      if(fabs(det)<1e-12){ o(px,py)=bg; continue; }
      // Cramer for X,Y
      double dX = b[0]*(A[1][1]*A[2][2]-A[1][2]*A[2][1]) - A[0][1]*(b[1]*A[2][2]-A[1][2]*b[2]) + A[0][2]*(b[1]*A[2][1]-A[1][1]*b[2]);
      double dY = A[0][0]*(b[1]*A[2][2]-A[1][2]*b[2]) - b[0]*(A[1][0]*A[2][2]-A[1][2]*A[2][0]) + A[0][2]*(A[1][0]*b[2]-b[1]*A[2][0]);
      double X=dX/det, Y=dY/det;
      float fxp = (float)(ox + X/sq*texPxPerCell), fyp = (float)(oy + Y/sq*texPxPerCell);
      float s = sampleTex(t, tw, th, fxp, fyp);
      o(px,py) = (s<0.f) ? (icl8u)bg : (icl8u)(s+0.5f);
    }
  }
  return out;
}

// ------------------------------------------------------------- target zoo
enum Type { CODED_WHITE, CODED_BLACK, PLAIN };
static const char* typeName(Type t){ return t==CODED_WHITE?"coded-white":t==CODED_BLACK?"coded-black":"plain-checker"; }

// small deterministic RNG (Date/rand unavailable-free)
struct RNG { unsigned s; double u(){ s=s*1103515245u+12345u; return ((s>>16)&0x7fff)/32767.0; } };

// mild sensor realism: additive uniform noise only (the renderer's bilinear
// plane-sampling already anti-aliases the edges; a further blur is double-
// softening and unrealistic). Coordinate-preserving by construction.
static void degrade(Img8u &img, int noiseAmp, RNG &rng){
  if(noiseAmp<=0) return;
  Channel8u c=img[0];
  for(int i=0;i<img.getDim();++i){ int v=c.begin()[i]+(int)((rng.u()*2-1)*noiseAmp); c.begin()[i]=v<0?0:v>255?255:v; }
}

// one rendered view -> {corners recovered, RMS vs GT} for a given target
struct ViewResult { int cornersFound, cornersTotal; double rmsPx; int markers; };
static ViewResult evalView(CalibrationTarget &tgt, FiducialDetector *fd, const std::string &pp,
                           const Img8u &tex, int C,int R,double sq,double px,double ox,double oy,
                           const Pose &p, const Cam &cam, RNG &rng){
  if(fd && !pp.empty()){ fd->setPropertyValue("pp.filter", pp); fd->setPropertyValue("quads.minimum region size", 25); }
  Img8u img = renderView(tex,C,R,sq,px,ox,oy,p,cam,205);
  degrade(img, 4, rng);
  auto corr = tgt.detect(img);   // internally runs the marker detector + saddle
  int markers = -1;
  double se=0; int n=0;
  for(auto &c: corr){ double u,v; if(!projectGT(c.objectPos[0]+sq, c.objectPos[1]+sq, p, cam, u, v)) continue;
    double du=c.imagePos.x-u, dv=c.imagePos.y-v; se+=du*du+dv*dv; ++n; }
  return { n, (C-1)*(R-1), n? std::sqrt(se/n):-1.0, markers };
}

// ======================================================================
// Tier B — end-to-end intrinsic calibration from rendered+detected views.
// Fixed camera (f,cx,cy,k1,k2); many diverse poses; feed detected corners into
// cv::IntrinsicCalibrator (partial-board masked overload) and compare recovered
// intrinsics to ground truth. Tests whether the target's corner noise (esp. the
// black-cell saddle penalty) actually moves the estimate.
// ======================================================================
// one detected corner in a view: board index + measured image pixel
struct Obs { int idx; Point32f img; };

// Per-view homography outlier reject: fit a board->image homography and iteratively
// drop correspondences whose reprojection residual exceeds `thr` px. A gross
// MISLABEL is off by >=1 cell (>=cellpx, here ~26-35px) while radial distortion
// deviates <=~8px, so thr~12 cleanly separates them. Returns the kept subset.
static std::vector<Obs> rejectOutliers(const std::vector<Obs> &obs, double sq, int NC, double thr){
  std::vector<Obs> keep = obs;
  for(int iter=0; iter<4 && keep.size()>=5; ++iter){
    std::vector<Point32f> B,I;
    for(auto &o: keep){ B.push_back(Point32f((o.idx%NC)*sq,(o.idx/NC)*sq)); I.push_back(o.img); }
    // Homography2D(A,B) yields H with apply(B)=A; we want apply(board)=image → (image,board)
    icl::math::Homography2D H(I.data(), B.data(), (int)B.size());
    std::vector<Obs> nk;
    for(auto &o: keep){ Point32f pr=H.apply(Point32f((o.idx%NC)*sq,(o.idx/NC)*sq));
      if(std::hypot(pr.x-o.img.x, pr.y-o.img.y) < thr) nk.push_back(o); }
    if(nk.size()==keep.size()){ keep=nk; break; }
    keep=nk;
  }
  return keep;
}

struct CalRes { double fx,fy,cx,cy,k1,k2; int used; bool ok; };
static CalRes runCalib(const std::vector<std::vector<Obs>> &views, int NC,int NR,double sq,int W,int H){
  using icl::math::DynMatrix; using icl::cv::IntrinsicCalibrator;
  const int NV=(int)views.size(), bSize=NC*NR;
  DynMatrix<icl64f> world=DynMatrix<icl64f>::create(3,bSize), impoints=DynMatrix<icl64f>::create(2*NV,bSize), mask=DynMatrix<icl64f>::create(NV,bSize);
  for(int r=0;r<NR;++r)for(int c=0;c<NC;++c){int idx=r*NC+c; world(0,idx)=c*sq; world(1,idx)=r*sq; world(2,idx)=0;}
  for(int i=0;i<2*NV*bSize;++i) impoints[i]=0; for(int i=0;i<NV*bSize;++i) mask[i]=0;
  int used=0;
  for(int v=0;v<NV;++v){ for(auto&o:views[v]){ impoints(2*v,o.idx)=o.img.x; impoints(2*v+1,o.idx)=o.img.y; mask(v,o.idx)=1; }
    if(views[v].size()>=6) used++; }
  try{ IntrinsicCalibrator cal(NC,NR,NV,W,H); auto r=cal.calibrate(impoints,world,mask);
    return { r.getFocalLengthX(),r.getFocalLengthY(),r.getPrincipalX(),r.getPrincipalY(),r.getK1(),r.getK2(),used,true }; }
  catch(std::exception&){ return {0,0,0,0,0,0,used,false}; }
}

static int tierB(){
  Cam cam; cam.W=640; cam.H=480; cam.cx=320; cam.cy=240;
  cam.f=650; cam.k1=-0.15; cam.k2=0.03;                 // ground-truth intrinsics
  const double sq=25.0, texPxPerCell=60;
  struct P{ double ax,ay,az,ox,oy,D; };
  P poses[] = {
    {-.5,-.32,.09,-60,-40,520},{ .46,-.35,-.14, 55,-32,560},{-.44,.44,.17,-20, 26,500},
    { .49,.38,-.09, 50, 40,600},{-.6,.09,0,  0,-45,480},{ .09,-.6,0,-40,  5,540},
    { .26,.52,.26, 30,-28,580},{-.35,-.52,-.2,-52, 22,520},{ .58,-.18,.1, 22, 46,560},
    {-.18,.58,-.16,-30,-40,500},{ .32,.32,0, 44,-44,620},{-.3,-.3,.12,-44, 44,540},
    { .2,-.2,-.2, 0,  0,470},{-.2,.2,.2, 10,-10,470},
  };
  const int NV=sizeof(poses)/sizeof(poses[0]);
  const int NSEED=6;
  struct Cfg{ Type type; int C,R; double fill; };
  Cfg cfgs[] = { {PLAIN,13,9,0}, {CODED_WHITE,13,9,0.62}, {CODED_BLACK,13,9,1.0} };

  printf("Tier B — %d seeds x %d views, 13x9. GT f=%.0f cx=320 cy=240 k1=%.3f k2=%.3f\n",
         NSEED,NV,cam.f,cam.k1,cam.k2);
  printf("Per target: mean|f%%err|, mean|k1err|, mean|cx err|px over seeds — RAW vs OUTLIER-REJECT (thr=12px)\n\n");
  printf("%-13s | %-28s | %-28s | dropped\n","target","RAW (no reject)","REJECT (homography, thr12)");
  printf("%-13s | %8s %8s %8s | %8s %8s %8s |\n","","f%err","k1err","cxErr","f%err","k1err","cxErr");
  for(auto &cf : cfgs){
    std::unique_ptr<CalibrationTarget> tgt; FiducialDetector *fd=nullptr; std::string pp;
    if(cf.type==PLAIN) tgt.reset(new CheckerboardTarget(cf.C,cf.R,(float)sq));
    else { auto*ct=new CodedCheckerboardTarget(cf.C,cf.R,(float)sq,(float)cf.fill,
             SquareBCHPreset::BCH_4x4_t2_RS, cf.type==CODED_BLACK?MarkerCells::Black:MarkerCells::White);
           tgt.reset(ct); fd=ct->markerDetector(); pp=cf.type==CODED_BLACK?"dilatation":"none"; }
    if(fd && !pp.empty()){ fd->setPropertyValue("pp.filter",pp); fd->setPropertyValue("quads.minimum region size",25); }
    Img8u tex=tgt->generate(Size((int)((cf.C+2)*texPxPerCell),(int)((cf.R+2)*texPxPerCell)));
    const double px=std::min(tex.getWidth()/double(cf.C+2),tex.getHeight()/double(cf.R+2));
    const double ox=(tex.getWidth()-px*cf.C)/2.0, oy=(tex.getHeight()-px*cf.R)/2.0;
    const int NC=cf.C-1, NR=cf.R-1;

    double sF[2]={0,0}, sK[2]={0,0}, sCx[2]={0,0}; int nOk[2]={0,0}, totalDropped=0, totalCorr=0;
    for(int seed=0; seed<NSEED; ++seed){
      RNG rng{ (unsigned)(seed*2654435761u+11u) };
      std::vector<std::vector<Obs>> raw(NV), clean(NV);
      for(int v=0; v<NV; ++v){
        Pose p; p.R=mul(mul(rotZ(poses[v].az),rotY(poses[v].ay)),rotX(poses[v].ax));
        V3 Cb=mul(p.R,V3{cf.C*sq/2,cf.R*sq/2,0}); p.t={ poses[v].ox-Cb.x, poses[v].oy-Cb.y, poses[v].D-Cb.z };
        Img8u img=renderView(tex,cf.C,cf.R,sq,px,ox,oy,p,cam,205); degrade(img,4,rng);
        for(auto&cc:tgt->detect(img)){ int c=(int)std::lround(cc.objectPos[0]/sq), r=(int)std::lround(cc.objectPos[1]/sq);
          if(c<0||c>=NC||r<0||r>=NR) continue; raw[v].push_back({r*NC+c, cc.imagePos}); }
        clean[v]=rejectOutliers(raw[v],sq,NC,12.0);
        totalCorr+=(int)raw[v].size(); totalDropped+=(int)(raw[v].size()-clean[v].size());
      }
      CalRes rr=runCalib(raw,NC,NR,sq,cam.W,cam.H), cr=runCalib(clean,NC,NR,sq,cam.W,cam.H);
      if(rr.ok){ sF[0]+=std::fabs(100*(rr.fx-cam.f)/cam.f); sK[0]+=std::fabs(rr.k1-cam.k1); sCx[0]+=std::fabs(rr.cx-cam.cx); nOk[0]++; }
      if(cr.ok){ sF[1]+=std::fabs(100*(cr.fx-cam.f)/cam.f); sK[1]+=std::fabs(cr.k1-cam.k1); sCx[1]+=std::fabs(cr.cx-cam.cx); nOk[1]++; }
    }
    auto A=[&](int m,double*s){ return nOk[m]? s[m]/nOk[m] : -1.0; };
    printf("%-13s | %7.2f%% %8.4f %7.1f | %7.2f%% %8.4f %7.1f | %d/%d\n",
           typeName(cf.type), A(0,sF),A(0,sK),A(0,sCx), A(1,sF),A(1,sK),A(1,sCx), totalDropped,totalCorr);
  }
  return 0;
}

int main(int argc, char **argv){
  if(argc>1 && std::string(argv[1])=="calib") return tierB();
  Cam cam0; cam0.W=640; cam0.H=480; cam0.cx=320; cam0.cy=240;
  const double D=1000, texPxPerCell=60;
  // a fixed pose set: fronto-parallel + tilts (radians)
  struct PoseAng{ double ax,ay; } angs[] = { {0,0},{0.30,0},{0,0.35},{0.28,-0.30},{-0.25,0.32},{0.15,0.5} };
  const int NP = sizeof(angs)/sizeof(angs[0]);

  struct Cfg { Type type; int C,R; double fill; };
  std::vector<Cfg> cfgs = {
    {CODED_WHITE,9,7,0.62},{CODED_WHITE,13,9,0.62},{CODED_WHITE,7,5,0.62},
    {CODED_BLACK,9,7,1.0}, {CODED_BLACK,13,9,1.0}, {CODED_BLACK,7,5,1.0},
    {PLAIN,      9,7,0.0}, {PLAIN,      13,9,0.0},
  };
  double cellpxs[] = { 40, 30, 24, 20, 16, 13, 10 };
  double k1s[]     = { 0.0, -0.20 };

  FILE *csv = fopen("studies/coded-target-study/results.csv","w");
  fprintf(csv,"type,cols,rows,fill,markerPx,cellpx,k1,poses,cornerRecoveryPct,medRMSpx,markerDetPct\n");
  printf("%-13s %-5s %5s %7s %5s  meanRec  usable  medRMS\n","type","cells","cellpx","markerPx","k1");
  for(auto &cf : cfgs){
    // build target once per (type,C,R,fill); generate texture once
    std::unique_ptr<CalibrationTarget> tgt;
    FiducialDetector *fd=nullptr; std::string pp;
    if(cf.type==PLAIN){ tgt.reset(new CheckerboardTarget(cf.C, cf.R, 25.f)); }
    else {
      auto *ct = new CodedCheckerboardTarget(cf.C, cf.R, 25.f, (float)cf.fill,
                    SquareBCHPreset::BCH_4x4_t2_RS,
                    cf.type==CODED_BLACK?MarkerCells::Black:MarkerCells::White);
      tgt.reset(ct); fd = ct->markerDetector();
      pp = (cf.type==CODED_BLACK) ? "dilatation" : "none";
    }
    const double sq=25.0;
    Img8u tex = tgt->generate(Size((int)((cf.C+2)*texPxPerCell),(int)((cf.R+2)*texPxPerCell)));
    const double px=std::min(tex.getWidth()/double(cf.C+2), tex.getHeight()/double(cf.R+2));
    const double ox=(tex.getWidth()-px*cf.C)/2.0, oy=(tex.getHeight()-px*cf.R)/2.0;

    for(double k1 : k1s){
      for(double cellpx : cellpxs){
        Cam cam=cam0; cam.k1=k1; cam.k2=0; cam.f=cellpx*D/sq;
        RNG rng{ (unsigned)(cf.C*131+cf.R*17+(int)(cellpx)*7+(int)(k1*-100)) };
        std::vector<double> rmss; double recovSum=0; int usable=0;
        for(int i=0;i<NP;i++){
          Pose p; p.R=mul(rotX(angs[i].ax), rotY(angs[i].ay));
          V3 Cb=mul(p.R, V3{cf.C*sq/2, cf.R*sq/2, 0}); p.t={ -Cb.x,-Cb.y, D-Cb.z };
          ViewResult vr=evalView(*tgt, fd, pp, tex, cf.C,cf.R,sq,px,ox,oy,p,cam,rng);
          double frac = vr.cornersTotal? vr.cornersFound/double(vr.cornersTotal):0;
          recovSum += frac;
          if(vr.cornersFound>0 && vr.rmsPx>=0 && vr.rmsPx<5.0){ rmss.push_back(vr.rmsPx); }
          if(frac>=0.5) usable++;                     // views yielding a usable corner set
        }
        std::sort(rmss.begin(),rmss.end());
        double med = rmss.empty()? -1 : rmss[rmss.size()/2];
        double meanRecovPct = 100.0*recovSum/NP;      // mean fraction of corners recovered
        double usablePct = 100.0*usable/NP;           // % of poses with >=50% corners
        double markerPx = (cf.type==CODED_WHITE? cf.fill:1.0)*cellpx*(cf.type==PLAIN?0:1);
        fprintf(csv,"%s,%d,%d,%.2f,%.1f,%.0f,%.2f,%d,%.0f,%.3f,%.0f\n",
                typeName(cf.type),cf.C,cf.R,cf.fill,markerPx,cellpx,k1,NP,meanRecovPct,med,usablePct);
        printf("%-13s %2dx%-2d %5.0f %7.1f %5.2f   %4.0f%%   %5.0f%%  %6.3f\n",
               typeName(cf.type),cf.C,cf.R,cellpx,markerPx,k1,meanRecovPct,usablePct,med<0?0:med);
      }
    }
    printf("\n");
  }
  fclose(csv);
  printf("wrote studies/coded-target-study/results.csv\n");
  return 0;
}
