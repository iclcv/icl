/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** File   : ICLQt/demos/mandelbrot/mandelbrot.cpp                  **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
** Interactive fractal explorer with OpenCL GPU acceleration and    **
** CPU fallback. Supports Mandelbrot, Julia, Burning Ship, and     **
** Tricorn fractals. Drag rectangles to zoom, right-click to undo. **
********************************************************************/

#include <ICLQt/Common.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Time.h>

#ifdef ICL_HAVE_OPENCL
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>
#endif

#include <vector>
#include <atomic>
#include <mutex>
#include <future>
#include <thread>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;

// Fractal types
enum FractalType { MANDELBROT=0, JULIA=1, BURNING_SHIP=2, TRICORN=3 };

// --- View state ---
static double cx = -0.5, cy = 0.0;
static double span = 3.0;
static std::mutex stateMutex;

// Zoom history (per fractal type)
static std::vector<std::tuple<double,double,double>> zoomHistory;

// Mouse drag state
static bool dragging = false;
static Point32f dragStart, dragEnd;
static std::mutex dragMutex;

// --- Zoom animation ---
struct ZoomAnim {
  double fromCx, fromCy, fromSpan;
  double toCx, toCy, toSpan;
  Time startTime;
  double duration;
  bool active = false;
};
static ZoomAnim anim;

static double easeInOut(double t){
  t = std::clamp(t, 0.0, 1.0);
  return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

static void startAnimation(double toCx, double toCy, double toSpan, double dur){
  if(anim.active){
    cx = anim.toCx; cy = anim.toCy; span = anim.toSpan;
  }
  anim.fromCx = cx;    anim.fromCy = cy;    anim.fromSpan = span;
  anim.toCx = toCx;    anim.toCy = toCy;    anim.toSpan = toSpan;
  anim.startTime = Time::now();
  anim.duration = dur;
  anim.active = true;
}

static void tickAnimation(){
  if(!anim.active) return;
  double elapsed = (Time::now() - anim.startTime).toSecondsDouble();
  double t = elapsed / anim.duration;
  if(t >= 1.0){
    cx = anim.toCx; cy = anim.toCy; span = anim.toSpan;
    anim.active = false;
  } else {
    double e = easeInOut(t);
    cx = anim.fromCx + (anim.toCx - anim.fromCx) * e;
    cy = anim.fromCy + (anim.toCy - anim.fromCy) * e;
    double logFrom = std::log(anim.fromSpan);
    double logTo = std::log(anim.toSpan);
    span = std::exp(logFrom + (logTo - logFrom) * e);
  }
}

// Default views per fractal type
static void getDefaultView(int type, double &dcx, double &dcy, double &dspan){
  switch(type){
    case MANDELBROT:   dcx = -0.5;  dcy = 0.0;   dspan = 3.0;  break;
    case JULIA:        dcx =  0.0;  dcy = 0.0;   dspan = 3.5;  break;
    case BURNING_SHIP: dcx = -0.4;  dcy = -0.6;  dspan = 4.0;  break;
    case TRICORN:      dcx = -0.3;  dcy = 0.0;   dspan = 3.5;  break;
    default:           dcx = -0.5;  dcy = 0.0;   dspan = 3.0;  break;
  }
}

// --- Color palette ---
static icl8u palR[256], palG[256], palB[256];

static void initPalette(){
  for(int i = 0; i < 256; ++i){
    double t = static_cast<double>(i) / 255.0;
    palR[i] = static_cast<icl8u>(std::min(255.0, 9.0 * (1-t) * t * t * t * 255));
    palG[i] = static_cast<icl8u>(std::min(255.0, 15.0 * (1-t) * (1-t) * t * t * 255));
    palB[i] = static_cast<icl8u>(std::min(255.0, 8.5 * (1-t) * (1-t) * (1-t) * t * 255));
  }
  palR[255] = palG[255] = palB[255] = 0;
}

// --- CPU fractal (multi-threaded) ---
static void fractalCPUSlice(icl8u *r, icl8u *g, icl8u *b,
                            int w, int yStart, int yEnd,
                            double x0, double y0, double dx, double dy,
                            int maxIter, int type, double jcr, double jci)
{
  for(int py = yStart; py < yEnd; ++py){
    for(int px = 0; px < w; ++px){
      double pr = x0 + px * dx;
      double pi = y0 + py * dy;

      double zr, zi, cr, ci;
      if(type == JULIA){
        zr = pr; zi = pi; cr = jcr; ci = jci;
      } else {
        zr = 0; zi = 0; cr = pr; ci = pi;
      }

      int iter = 0;
      while(zr*zr + zi*zi <= 4.0 && iter < maxIter){
        double tmp;
        switch(type){
          case BURNING_SHIP:
            tmp = zr*zr - zi*zi + cr;
            zi = std::abs(2.0*zr*zi) + ci;
            zr = tmp;
            break;
          case TRICORN:
            tmp = zr*zr - zi*zi + cr;
            zi = -2.0*zr*zi + ci;
            zr = tmp;
            break;
          default: // MANDELBROT, JULIA
            tmp = zr*zr - zi*zi + cr;
            zi = 2.0*zr*zi + ci;
            zr = tmp;
            break;
        }
        ++iter;
      }
      int idx = px + py * w;
      if(iter >= maxIter){
        r[idx] = g[idx] = b[idx] = 0;
      } else {
        double mu = iter - std::log2(std::log2(zr*zr + zi*zi) / 2.0);
        int ci2 = static_cast<int>(mu * 4.0) & 255;
        r[idx] = palR[ci2];
        g[idx] = palG[ci2];
        b[idx] = palB[ci2];
      }
    }
  }
}

static void fractalCPU(icl8u *r, icl8u *g, icl8u *b,
                       int w, int h, double cxv, double cyv, double sp,
                       int maxIter, int type, double jcr, double jci)
{
  double x0 = cxv - sp / 2.0;
  double y0 = cyv - sp * h / (2.0 * w);
  double dx = sp / w;
  double dy = sp / w;

  int nThreads = std::max(1u, std::thread::hardware_concurrency());
  std::vector<std::future<void>> futures;
  for(int t = 0; t < nThreads; ++t){
    int yStart = t * h / nThreads;
    int yEnd = (t + 1) * h / nThreads;
    futures.push_back(std::async(std::launch::async,
      fractalCPUSlice, r, g, b, w, yStart, yEnd, x0, y0, dx, dy,
      maxIter, type, jcr, jci));
  }
  for(auto &f : futures) f.get();
}

// --- OpenCL GPU fractal ---
#ifdef ICL_HAVE_OPENCL
static const char *fractalKernelSrc = R"CL(
__kernel void fractal(__global uchar *outR,
                      __global uchar *outG,
                      __global uchar *outB,
                      __global const uchar *palR,
                      __global const uchar *palG,
                      __global const uchar *palB,
                      const float cx, const float cy,
                      const float span, const int maxIter,
                      const int width, const int height,
                      const int type,
                      const float julia_cr, const float julia_ci)
{
  int px = get_global_id(0);
  int py = get_global_id(1);
  if(px >= width || py >= height) return;

  float pr = cx - span / 2.0f + (float)px * span / (float)width;
  float pi = cy - span * (float)height / (2.0f * (float)width) + (float)py * span / (float)width;

  float zr, zi, cr, ci;
  if(type == 1){  // Julia
    zr = pr; zi = pi; cr = julia_cr; ci = julia_ci;
  } else {
    zr = 0; zi = 0; cr = pr; ci = pi;
  }

  int iter = 0;
  while(zr*zr + zi*zi <= 4.0f && iter < maxIter){
    float tmp;
    if(type == 2){        // Burning Ship
      tmp = zr*zr - zi*zi + cr;
      zi = fabs(2.0f*zr*zi) + ci;
      zr = tmp;
    } else if(type == 3){ // Tricorn
      tmp = zr*zr - zi*zi + cr;
      zi = -2.0f*zr*zi + ci;
      zr = tmp;
    } else {              // Mandelbrot, Julia
      tmp = zr*zr - zi*zi + cr;
      zi = 2.0f*zr*zi + ci;
      zr = tmp;
    }
    ++iter;
  }
  int idx = px + py * width;
  if(iter >= maxIter){
    outR[idx] = outG[idx] = outB[idx] = 0;
  } else {
    float logzn = log(zr*zr + zi*zi) / 2.0f;
    float mu = (float)iter - log2(logzn);
    int c = ((int)(mu * 4.0f)) & 255;
    outR[idx] = palR[c];
    outG[idx] = palG[c];
    outB[idx] = palB[c];
  }
}
)CL";

static CLProgram *clProg = nullptr;
static CLKernel clKernel;
static CLBuffer clBufR, clBufG, clBufB;
static CLBuffer clPalR, clPalG, clPalB;
static int clW = 0, clH = 0;

static void initOpenCL(int w, int h){
  try {
    clProg = new CLProgram("gpu", fractalKernelSrc);
    clKernel = clProg->createKernel("fractal");
    clBufR = clProg->createBuffer("w", w * h);
    clBufG = clProg->createBuffer("w", w * h);
    clBufB = clProg->createBuffer("w", w * h);
    clPalR = clProg->createBuffer("r", 256, palR);
    clPalG = clProg->createBuffer("r", 256, palG);
    clPalB = clProg->createBuffer("r", 256, palB);
    clW = w; clH = h;
  } catch(const ICLException &e){
    WARNING_LOG("OpenCL init failed: " << e.what());
    delete clProg;
    clProg = nullptr;
  }
}

static void resizeOpenCL(int w, int h){
  if(!clProg) return;
  clBufR = clProg->createBuffer("w", w * h);
  clBufG = clProg->createBuffer("w", w * h);
  clBufB = clProg->createBuffer("w", w * h);
  clW = w; clH = h;
}

static void fractalGPU(icl8u *r, icl8u *g, icl8u *b,
                       int w, int h, double cxv, double cyv, double sp,
                       int maxIter, int type, double jcr, double jci)
{
  if(w != clW || h != clH) resizeOpenCL(w, h);
  clKernel.setArgs(clBufR, clBufG, clBufB, clPalR, clPalG, clPalB,
                   static_cast<float>(cxv), static_cast<float>(cyv),
                   static_cast<float>(sp), maxIter, w, h,
                   type, static_cast<float>(jcr), static_cast<float>(jci));
  clKernel.apply(w, h);
  clBufR.read(r, w * h);
  clBufG.read(g, w * h);
  clBufB.read(b, w * h);
}
#endif

// --- GUI ---
VSplit gui;
Img8u image;
static int lastFractalType = -1;

static void mouseHandler(const MouseEvent &event){
  if(event.isLeft()){
    if(event.isPressEvent()){
      std::lock_guard<std::mutex> lock(dragMutex);
      dragging = true;
      dragStart = event.getPos32f();
      dragEnd = dragStart;
    } else if(event.isDragEvent()){
      std::lock_guard<std::mutex> lock(dragMutex);
      dragEnd = event.getPos32f();
    } else if(event.isReleaseEvent()){
      std::lock_guard<std::mutex> lock(dragMutex);
      dragging = false;
      dragEnd = event.getPos32f();

      float x1 = std::min(dragStart.x, dragEnd.x);
      float y1 = std::min(dragStart.y, dragEnd.y);
      float x2 = std::max(dragStart.x, dragEnd.x);
      float y2 = std::max(dragStart.y, dragEnd.y);

      if(x2 - x1 > 5 && y2 - y1 > 5){
        std::lock_guard<std::mutex> slock(stateMutex);
        tickAnimation();
        int w = image.getWidth();
        int h = image.getHeight();
        zoomHistory.push_back({cx, cy, span});
        double px2cx = span / w;
        double left = cx - span / 2.0;
        double top = cy - span * h / (2.0 * w);
        double newCx = left + (x1 + x2) / 2.0 * px2cx;
        double newCy = top + (y1 + y2) / 2.0 * px2cx;
        double newSpan = (x2 - x1) * px2cx;
        startAnimation(newCx, newCy, newSpan, 0.8);
      }
    }
  } else if(event.isRight() && event.isPressEvent()){
    std::lock_guard<std::mutex> lock(stateMutex);
    tickAnimation();
    if(!zoomHistory.empty()){
      auto [ocx, ocy, ospan] = zoomHistory.back();
      zoomHistory.pop_back();
      startAnimation(ocx, ocy, ospan, 0.6);
    }
  }
}

void init(){
  initPalette();

  gui << Canvas().handle("draw").minSize(40, 30)
      << ( VBox()
           << Combo("!Mandelbrot,Julia,Burning Ship,Tricorn")
              .handle("fractal").label("fractal type")
           << Slider(32, 4096, 512).handle("maxiter").label("max iterations")
           << FSlider(-2.0, 2.0, -0.7269).handle("julia_cr").label("Julia c (real)")
           << FSlider(-2.0, 2.0, 0.1889).handle("julia_ci").label("Julia c (imag)")
           << CheckBox("Use GPU", true).handle("gpu")
           << Label("---").handle("info")
           << Button("Reset View").handle("reset")
         )
      << Show();

  gui["draw"].install(mouseHandler);

#ifdef ICL_HAVE_OPENCL
  initOpenCL(800, 600);
#endif
}

void run(){
  DrawHandle draw = gui["draw"];
  int maxIter = gui["maxiter"].as<int>();
  bool reset = gui["reset"].as<ButtonHandle>().wasTriggered();
  int fractalType = gui["fractal"].as<ComboHandle>().getSelectedIndex();
  float jcr = gui["julia_cr"].as<float>();
  float jci = gui["julia_ci"].as<float>();

#ifdef ICL_HAVE_OPENCL
  bool useGPU = gui["gpu"].as<bool>();
#else
  bool useGPU = false;
#endif

  // Fractal type changed → reset view to that fractal's default
  if(fractalType != lastFractalType){
    if(lastFractalType >= 0){
      std::lock_guard<std::mutex> lock(stateMutex);
      tickAnimation();
      zoomHistory.clear();
      double dcx, dcy, dspan;
      getDefaultView(fractalType, dcx, dcy, dspan);
      startAnimation(dcx, dcy, dspan, 0.5);
    } else {
      // First frame: set directly
      getDefaultView(fractalType, cx, cy, span);
    }
    lastFractalType = fractalType;
  }

  if(reset){
    std::lock_guard<std::mutex> lock(stateMutex);
    tickAnimation();
    zoomHistory.clear();
    double dcx, dcy, dspan;
    getDefaultView(fractalType, dcx, dcy, dspan);
    startAnimation(dcx, dcy, dspan, 0.6);
  }

  ICLWidget *w = *draw;
  int rw = std::max(320, w->width());
  int rh = std::max(240, w->height());

  if(image.getSize() != Size(rw, rh)){
    image.setSize(Size(rw, rh));
    image.setChannels(3);
    image.setFormat(formatRGB);
  }

  double lcx, lcy, lspan;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    tickAnimation();
    lcx = cx; lcy = cy; lspan = span;
  }

  Time t = Time::now();

#ifdef ICL_HAVE_OPENCL
  if(useGPU && clProg){
    fractalGPU(image.begin(0), image.begin(1), image.begin(2),
               rw, rh, lcx, lcy, lspan, maxIter, fractalType, jcr, jci);
  } else
#endif
  {
    fractalCPU(image.begin(0), image.begin(1), image.begin(2),
               rw, rh, lcx, lcy, lspan, maxIter, fractalType, jcr, jci);
  }

  double ms = (Time::now() - t).toMilliSecondsDouble();

  draw = &image;

  // Draw selection rectangle
  {
    std::lock_guard<std::mutex> lock(dragMutex);
    if(dragging){
      draw->color(255, 255, 255, 200);
      draw->nofill();
      draw->linewidth(2);
      float x1 = std::min(dragStart.x, dragEnd.x);
      float y1 = std::min(dragStart.y, dragEnd.y);
      float x2 = std::max(dragStart.x, dragEnd.x);
      float y2 = std::max(dragStart.y, dragEnd.y);
      draw->rect(x1, y1, x2 - x1, y2 - y1);
    }
  }

  // Info
  static const char *names[] = {"Mandelbrot", "Julia", "Burning Ship", "Tricorn"};
  std::ostringstream oss;
  oss << names[fractalType] << "  "
      << std::fixed << std::setprecision(1) << ms << " ms"
      << (useGPU ? " [GPU]" : " [CPU]")
      << "  zoom=" << std::setprecision(2) << std::scientific << (3.0 / lspan) << "x";
  if(fractalType == JULIA){
    oss << "  c=" << std::fixed << std::setprecision(4) << jcr
        << (jci >= 0 ? "+" : "") << jci << "i";
  }
  gui["info"] = oss.str();

  draw.render();
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
