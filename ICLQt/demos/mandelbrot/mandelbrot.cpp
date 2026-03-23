/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** File   : ICLQt/demos/mandelbrot/mandelbrot.cpp                  **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
** Interactive Mandelbrot explorer with OpenCL GPU acceleration     **
** and CPU fallback. Drag rectangles to zoom into regions.         **
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

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;

// --- Mandelbrot coordinate state ---
static double cx = -0.5, cy = 0.0;  // center
static double span = 3.0;           // visible width in complex plane
static std::mutex stateMutex;

// Zoom history for undo
static std::vector<std::tuple<double,double,double>> zoomHistory;

// Mouse drag state
static bool dragging = false;
static Point32f dragStart, dragEnd;
static std::mutex dragMutex;

// --- Color palette (256 entries, smooth gradient) ---
static icl8u palR[256], palG[256], palB[256];

static void initPalette(){
  for(int i = 0; i < 256; ++i){
    double t = static_cast<double>(i) / 255.0;
    // Ultra Fractal-style smooth palette
    palR[i] = static_cast<icl8u>(std::min(255.0, 9.0 * (1-t) * t * t * t * 255));
    palG[i] = static_cast<icl8u>(std::min(255.0, 15.0 * (1-t) * (1-t) * t * t * 255));
    palB[i] = static_cast<icl8u>(std::min(255.0, 8.5 * (1-t) * (1-t) * (1-t) * t * 255));
  }
  palR[255] = palG[255] = palB[255] = 0; // inside set = black
}

// --- CPU Mandelbrot ---
static void mandelbrotCPU(icl8u *r, icl8u *g, icl8u *b,
                          int w, int h, double cxv, double cyv, double sp, int maxIter)
{
  double x0 = cxv - sp / 2.0;
  double y0 = cyv - sp * h / (2.0 * w);
  double dx = sp / w;
  double dy = sp / w;  // square pixels

  for(int py = 0; py < h; ++py){
    for(int px = 0; px < w; ++px){
      double cr = x0 + px * dx;
      double ci = y0 + py * dy;
      double zr = 0, zi = 0;
      int iter = 0;
      while(zr*zr + zi*zi <= 4.0 && iter < maxIter){
        double tmp = zr*zr - zi*zi + cr;
        zi = 2.0*zr*zi + ci;
        zr = tmp;
        ++iter;
      }
      int idx = px + py * w;
      if(iter >= maxIter){
        r[idx] = g[idx] = b[idx] = 0;
      } else {
        // smooth coloring
        double mu = iter - std::log2(std::log2(zr*zr + zi*zi) / 2.0);
        int ci2 = static_cast<int>(mu * 4.0) & 255;
        r[idx] = palR[ci2];
        g[idx] = palG[ci2];
        b[idx] = palB[ci2];
      }
    }
  }
}

// --- OpenCL Mandelbrot ---
#ifdef ICL_HAVE_OPENCL
static const char *mandelbrotKernelSrc = R"CL(
__kernel void mandelbrot(__global uchar *outR,
                         __global uchar *outG,
                         __global uchar *outB,
                         __global const uchar *palR,
                         __global const uchar *palG,
                         __global const uchar *palB,
                         const double cx, const double cy,
                         const double span, const int maxIter,
                         const int width, const int height)
{
  int px = get_global_id(0);
  int py = get_global_id(1);
  if(px >= width || py >= height) return;

  double x0 = cx - span / 2.0 + px * span / width;
  double y0 = cy - span * height / (2.0 * width) + py * span / width;
  double zr = 0, zi = 0;
  int iter = 0;
  while(zr*zr + zi*zi <= 4.0 && iter < maxIter){
    double tmp = zr*zr - zi*zi + x0;
    zi = 2.0*zr*zi + y0;
    zr = tmp;
    ++iter;
  }
  int idx = px + py * width;
  if(iter >= maxIter){
    outR[idx] = outG[idx] = outB[idx] = 0;
  } else {
    double logzn = log(zr*zr + zi*zi) / 2.0;
    double mu = iter - log2(logzn);
    int ci = ((int)(mu * 4.0)) & 255;
    outR[idx] = palR[ci];
    outG[idx] = palG[ci];
    outB[idx] = palB[ci];
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
    clProg = new CLProgram("gpu", mandelbrotKernelSrc);
    clKernel = clProg->createKernel("mandelbrot");
    clBufR = clProg->createBuffer("w", w * h);
    clBufG = clProg->createBuffer("w", w * h);
    clBufB = clProg->createBuffer("w", w * h);
    clPalR = clProg->createBuffer("r", 256, palR);
    clPalG = clProg->createBuffer("r", 256, palG);
    clPalB = clProg->createBuffer("r", 256, palB);
    clW = w; clH = h;
  } catch(const ICLException &e){
    WARNING_LOG("OpenCL init failed: " << e.what() << " — falling back to CPU");
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

static void mandelbrotGPU(icl8u *r, icl8u *g, icl8u *b,
                          int w, int h, double cxv, double cyv, double sp, int maxIter)
{
  if(w != clW || h != clH) resizeOpenCL(w, h);
  clKernel.setArgs(clBufR, clBufG, clBufB, clPalR, clPalG, clPalB,
                   cxv, cyv, sp, maxIter, w, h);
  clKernel.apply(w, h);
  clBufR.read(r, w * h);
  clBufG.read(g, w * h);
  clBufB.read(b, w * h);
}
#endif

// --- GUI ---
GUI gui;
Img8u image;

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

      // Compute zoom rect in pixel coords
      float x1 = std::min(dragStart.x, dragEnd.x);
      float y1 = std::min(dragStart.y, dragEnd.y);
      float x2 = std::max(dragStart.x, dragEnd.x);
      float y2 = std::max(dragStart.y, dragEnd.y);

      if(x2 - x1 > 5 && y2 - y1 > 5){
        std::lock_guard<std::mutex> slock(stateMutex);
        int w = image.getWidth();
        int h = image.getHeight();
        // Save for undo
        zoomHistory.push_back({cx, cy, span});
        // Convert pixel rect to complex plane coords
        double px2cx = span / w;
        double left = cx - span / 2.0;
        double top = cy - span * h / (2.0 * w);
        cx = left + (x1 + x2) / 2.0 * px2cx;
        cy = top + (y1 + y2) / 2.0 * px2cx;
        span = (x2 - x1) * px2cx;
      }
    }
  } else if(event.isRight() && event.isPressEvent()){
    // Right-click: undo zoom
    std::lock_guard<std::mutex> lock(stateMutex);
    if(!zoomHistory.empty()){
      auto [ocx, ocy, ospan] = zoomHistory.back();
      zoomHistory.pop_back();
      cx = ocx; cy = ocy; span = ospan;
    }
  }
}

void init(){
  initPalette();

  gui << Draw().handle("draw").minSize(40, 30)
      << ( VBox()
           << Slider(32, 4096, 512).handle("maxiter").label("max iterations")
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

#ifdef ICL_HAVE_OPENCL
  bool useGPU = gui["gpu"].as<bool>();
#else
  bool useGPU = false;
#endif

  if(reset){
    std::lock_guard<std::mutex> lock(stateMutex);
    cx = -0.5; cy = 0.0; span = 3.0;
    zoomHistory.clear();
  }

  // Get widget size for rendering resolution
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
    lcx = cx; lcy = cy; lspan = span;
  }

  Time t = Time::now();

#ifdef ICL_HAVE_OPENCL
  if(useGPU && clProg){
    mandelbrotGPU(image.begin(0), image.begin(1), image.begin(2),
                  rw, rh, lcx, lcy, lspan, maxIter);
  } else
#endif
  {
    mandelbrotCPU(image.begin(0), image.begin(1), image.begin(2),
                  rw, rh, lcx, lcy, lspan, maxIter);
  }

  double ms = (Time::now() - t).toMilliSecondsDouble();

  draw = &image;

  // Draw selection rectangle if dragging
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

  // Info overlay
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1) << ms << " ms"
      << (useGPU ? " [GPU]" : " [CPU]")
      << "  zoom=" << std::setprecision(2) << std::scientific << (3.0 / lspan) << "x";
  gui["info"] = oss.str();

  draw.render();
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
