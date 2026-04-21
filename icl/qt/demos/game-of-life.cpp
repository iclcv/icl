// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <iomanip>
#include <icl/core/Img.h>
#include <icl/utils/Time.h>

#ifdef ICL_HAVE_OPENCL
#include <icl/utils/CLProgram.h>
#include <icl/utils/CLBuffer.h>
#include <icl/utils/CLKernel.h>
#endif

#include <vector>
#include <random>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;

// ---- Grid state ----
static int gridW = 256, gridH = 192;
static std::vector<int> grid;     // cell age: 0=dead, 1..255=alive N generations
static std::vector<int> gridBuf;  // double-buffer for CPU step
static int generation = 0;
static int lastAlive = 0;
static bool gridDirty = true;  // set when grid changes, cleared after render
static std::mutex gridMutex;

// ---- Patterns (relative cell coordinates) ----
struct Pattern {
  std::vector<std::pair<int,int>> cells;
};
static std::vector<Pattern> patterns;

static void initPatterns(){
  // 0: Glider
  patterns.push_back({{{1,0},{2,1},{0,2},{1,2},{2,2}}});
  // 1: LWSS (lightweight spaceship)
  patterns.push_back({{{1,0},{4,0},{0,1},{0,2},{4,2},{0,3},{1,3},{2,3},{3,3}}});
  // 2: R-pentomino (long-lived methuselah)
  patterns.push_back({{{1,0},{2,0},{0,1},{1,1},{1,2}}});
  // 3: Acorn (stabilizes after 5206 generations)
  patterns.push_back({{{1,0},{3,1},{0,2},{1,2},{4,2},{5,2},{6,2}}});
  // 4: Gosper glider gun
  patterns.push_back({{{24,0},{22,1},{24,1},{12,2},{13,2},{20,2},{21,2},{34,2},{35,2},
    {11,3},{15,3},{20,3},{21,3},{34,3},{35,3},
    {0,4},{1,4},{10,4},{16,4},{20,4},{21,4},
    {0,5},{1,5},{10,5},{14,5},{16,5},{17,5},{22,5},{24,5},
    {10,6},{16,6},{24,6},{11,7},{15,7},{12,8},{13,8}}});
  // 5: Pulsar (period-3 oscillator)
  patterns.push_back({{{2,0},{3,0},{4,0},{8,0},{9,0},{10,0},
    {0,2},{5,2},{7,2},{12,2},{0,3},{5,3},{7,3},{12,3},
    {0,4},{5,4},{7,4},{12,4},{2,5},{3,5},{4,5},{8,5},{9,5},{10,5},
    {2,7},{3,7},{4,7},{8,7},{9,7},{10,7},
    {0,8},{5,8},{7,8},{12,8},{0,9},{5,9},{7,9},{12,9},
    {0,10},{5,10},{7,10},{12,10},
    {2,12},{3,12},{4,12},{8,12},{9,12},{10,12}}});
  // 6: Diehard (dies after exactly 130 generations)
  patterns.push_back({{{6,0},{0,1},{1,1},{1,2},{5,2},{6,2},{7,2}}});
}

// ---- Color palette for cell age ----
static icl8u palR[256], palG[256], palB[256];

static void initPalette(){
  // Dead: dark blue-grey
  palR[0] = 12; palG[0] = 12; palB[0] = 20;
  // Alive: bright cyan-green → yellow → orange → dim crimson
  for(int i = 1; i < 256; ++i){
    float t = std::min(1.0f, (i - 1) / 50.0f);
    palR[i] = static_cast<icl8u>(20 + 210 * t);
    palG[i] = static_cast<icl8u>(240 - 120 * t);
    palB[i] = static_cast<icl8u>(80 * (1.0f - t * 0.9f));
  }
}

// ---- CPU step (toroidal boundary) ----
static void stepCPU(){
  gridBuf.resize(gridW * gridH);
  for(int y = 0; y < gridH; ++y){
    for(int x = 0; x < gridW; ++x){
      int n = 0;
      for(int dy = -1; dy <= 1; ++dy){
        for(int dx = -1; dx <= 1; ++dx){
          if(dx == 0 && dy == 0) continue;
          int nx = (x + dx + gridW) % gridW;
          int ny = (y + dy + gridH) % gridH;
          if(grid[ny * gridW + nx] > 0) ++n;
        }
      }
      int idx = y * gridW + x;
      int age = grid[idx];
      if(age > 0){
        gridBuf[idx] = (n == 2 || n == 3) ? std::min(255, age + 1) : 0;
      } else {
        gridBuf[idx] = (n == 3) ? 1 : 0;
      }
    }
  }
  std::swap(grid, gridBuf);
}

// ---- OpenCL GPU step + render ----
#ifdef ICL_HAVE_OPENCL
static const char *golKernelSrc = R"CL(
__kernel void gol_step(__global const int *src, __global int *dst,
                       const int w, const int h)
{
  int x = get_global_id(0);
  int y = get_global_id(1);
  if(x >= w || y >= h) return;
  int n = 0;
  for(int dy = -1; dy <= 1; ++dy){
    for(int dx = -1; dx <= 1; ++dx){
      if(dx == 0 && dy == 0) continue;
      int nx = (x + dx + w) % w;
      int ny = (y + dy + h) % h;
      if(src[ny * w + nx] > 0) n++;
    }
  }
  int idx = y * w + x;
  int age = src[idx];
  if(age > 0){
    dst[idx] = (n == 2 || n == 3) ? min(255, age + 1) : 0;
  } else {
    dst[idx] = (n == 3) ? 1 : 0;
  }
}

__kernel void gol_render(__global const int *state,
                         __global uchar *outR,
                         __global uchar *outG,
                         __global uchar *outB,
                         __global const uchar *palR,
                         __global const uchar *palG,
                         __global const uchar *palB,
                         const int w, const int h)
{
  int x = get_global_id(0);
  int y = get_global_id(1);
  if(x >= w || y >= h) return;
  int idx = y * w + x;
  int age = clamp(state[idx], 0, 255);
  outR[idx] = palR[age];
  outG[idx] = palG[age];
  outB[idx] = palB[age];
}
)CL";

static CLProgram *clProg = nullptr;
static CLKernel clKernel, clRenderKernel;
static CLBuffer clSrc, clDst;
static CLBuffer clOutR, clOutG, clOutB;
static CLBuffer clPalR, clPalG, clPalB;
static int clBufN = 0;

static void initOpenCL(){
  try {
    clProg = new CLProgram("gpu", golKernelSrc);
    clKernel = clProg->createKernel("gol_step");
    clRenderKernel = clProg->createKernel("gol_render");
    int n = gridW * gridH;
    int bytes = n * static_cast<int>(sizeof(int));
    clSrc = clProg->createBuffer("rw", bytes);
    clDst = clProg->createBuffer("rw", bytes);
    clOutR = clProg->createBuffer("w", n);
    clOutG = clProg->createBuffer("w", n);
    clOutB = clProg->createBuffer("w", n);
    clPalR = clProg->createBuffer("r", 256, palR);
    clPalG = clProg->createBuffer("r", 256, palG);
    clPalB = clProg->createBuffer("r", 256, palB);
    clBufN = n;
  } catch(const ICLException &e){
    WARNING_LOG("OpenCL init failed: " << e.what());
    delete clProg;
    clProg = nullptr;
  }
}

static void resizeCLBuffers(){
  if(!clProg) return;
  int n = gridW * gridH;
  int bytes = n * static_cast<int>(sizeof(int));
  clSrc = clProg->createBuffer("rw", bytes);
  clDst = clProg->createBuffer("rw", bytes);
  clOutR = clProg->createBuffer("w", n);
  clOutG = clProg->createBuffer("w", n);
  clOutB = clProg->createBuffer("w", n);
  clBufN = n;
}

// Step simulation on GPU, grid[] is updated
static void stepGPU(){
  int n = gridW * gridH;
  if(n != clBufN) resizeCLBuffers();
  int bytes = n * static_cast<int>(sizeof(int));
  clSrc.write(grid.data(), bytes);
  clKernel.setArgs(clSrc, clDst, gridW, gridH);
  clKernel.apply(gridW, gridH);
  clDst.read(grid.data(), bytes);
}

// Render grid to image entirely on GPU (age→RGB mapping)
static void renderGPU(Img8u &img){
  int n = gridW * gridH;
  if(n != clBufN) resizeCLBuffers();
  if(img.getSize() != Size(gridW, gridH)){
    img.setSize(Size(gridW, gridH));
    img.setChannels(3);
    img.setFormat(formatRGB);
  }
  int bytes = n * static_cast<int>(sizeof(int));
  // Upload current grid state (may have been modified by drawing)
  clSrc.write(grid.data(), bytes);
  clRenderKernel.setArgs(clSrc, clOutR, clOutG, clOutB,
                         clPalR, clPalG, clPalB, gridW, gridH);
  clRenderKernel.apply(gridW, gridH);
  clOutR.read(img.begin(0), n);
  clOutG.read(img.begin(1), n);
  clOutB.read(img.begin(2), n);
}
#endif

// ---- Rendering ----
static Img8u image;

static void renderGrid(){
  if(image.getSize() != Size(gridW, gridH)){
    image.setSize(Size(gridW, gridH));
    image.setChannels(3);
    image.setFormat(formatRGB);
  }
  icl8u *r = image.begin(0);
  icl8u *g = image.begin(1);
  icl8u *b = image.begin(2);
  for(int i = 0, total = gridW * gridH; i < total; ++i){
    int a = std::clamp(grid[i], 0, 255);
    r[i] = palR[a];
    g[i] = palG[a];
    b[i] = palB[a];
  }
}

// ---- Pattern placement (centered at click) ----
static void placePattern(int patIdx, int cx, int cy){
  if(patIdx < 0 || patIdx >= static_cast<int>(patterns.size())) return;
  const auto &cells = patterns[patIdx].cells;
  int minX = 9999, minY = 9999, maxX = 0, maxY = 0;
  for(const auto &c : cells){
    minX = std::min(minX, c.first);
    maxX = std::max(maxX, c.first);
    minY = std::min(minY, c.second);
    maxY = std::max(maxY, c.second);
  }
  int ox = cx - (minX + maxX) / 2;
  int oy = cy - (minY + maxY) / 2;
  for(const auto &c : cells){
    int x = (c.first + ox + gridW) % gridW;
    int y = (c.second + oy + gridH) % gridH;
    grid[y * gridW + x] = 1;
  }
}

// ---- Grid helpers ----
static void resetGrid(){
  grid.assign(gridW * gridH, 0);
  gridBuf.assign(gridW * gridH, 0);
  generation = 0;
  gridDirty = true;
}

static void randomizeGrid(float density = 0.25f){
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  for(int i = 0, n = gridW * gridH; i < n; ++i){
    grid[i] = (dist(rng) < density) ? 1 : 0;
  }
  generation = 0;
  gridDirty = true;
}

// ---- GUI ----
static HSplit gui;
static const int SIZES[][2] = {{128,96},{256,192},{512,384},{1024,768},{1920,1080},{3840,2160}};
static int curSizeIdx = 1;

static void mouseHandler(const MouseEvent &event){
  if(!event.isLeft() && !event.isRight()) return;
  if(!event.isPressEvent() && !event.isDragEvent()) return;

  std::scoped_lock<std::mutex> lock(gridMutex);
  int x = static_cast<int>(event.getPos32f().x);
  int y = static_cast<int>(event.getPos32f().y);
  if(x < 0 || x >= gridW || y < 0 || y >= gridH) return;

  if(event.isRight()){
    // Right-click: erase (3x3 brush)
    for(int dy = -1; dy <= 1; ++dy){
      for(int dx = -1; dx <= 1; ++dx){
        int ex = (x + dx + gridW) % gridW;
        int ey = (y + dy + gridH) % gridH;
        grid[ey * gridW + ex] = 0;
      }
    }
    gridDirty = true;
    return;
  }

  // Left click: draw or place pattern
  ComboHandle tool = gui["tool"];
  int toolIdx = tool.getSelectedIndex();
  if(toolIdx == 0){
    // Draw mode: paint alive cells
    grid[y * gridW + x] = 1;
  } else if(event.isPressEvent()){
    // Place pattern (only on click, not drag)
    placePattern(toolIdx - 1, x, y);
  }
  gridDirty = true;
}

void init(){
  initPalette();
  initPatterns();
  grid.assign(gridW * gridH, 0);
  gridBuf.assign(gridW * gridH, 0);
  randomizeGrid(0.2f);

  gui << Canvas().handle("draw").minSize(40,30)
      << ( VBox().maxSize(15,99)
           << CheckBox("Running", true).handle("running").maxSize(15,2)
           << Button("Step").handle("step").maxSize(15,2)
           << Combo("1,2,5,10,!30,60,120").handle("speed").label("speed (gen/s)").maxSize(15,3)
           << Combo("!Draw,Glider,LWSS,R-pentomino,Acorn,Gun,Pulsar,Diehard")
              .handle("tool").label("tool").maxSize(15,3)
           << Button("Random").handle("random").maxSize(15,2)
           << Button("Clear").handle("clear").maxSize(15,2)
           << Combo("128x96,!256x192,512x384,1024x768,1920x1080,3840x2160")
              .handle("size").label("grid size").maxSize(15,3)
           << CheckBox("Use GPU", true).handle("gpu").maxSize(15,2)
           << Label("---").handle("info").label("info").maxSize(15,2)
           << Label("---").handle("info2").label("info2")
         )
      << Show();

  gui["draw"].install(mouseHandler);

#ifdef ICL_HAVE_OPENCL
  initOpenCL();
#endif
}

static Time lastStep = Time::now();

void run(){
  bool running = gui["running"].as<bool>();
  int speed = std::stoi(gui["speed"].as<ComboHandle>().getSelectedItem());
  bool doStep = gui["step"].as<ButtonHandle>().wasTriggered();
  bool doClear = gui["clear"].as<ButtonHandle>().wasTriggered();
  bool doRandom = gui["random"].as<ButtonHandle>().wasTriggered();

#ifdef ICL_HAVE_OPENCL
  bool useGPU = gui["gpu"].as<bool>();
#else
  bool useGPU = false;
#endif

  // Grid size change
  int newSizeIdx = gui["size"].as<ComboHandle>().getSelectedIndex();
  if(newSizeIdx != curSizeIdx){
    curSizeIdx = newSizeIdx;
    std::scoped_lock<std::mutex> lock(gridMutex);
    gridW = SIZES[curSizeIdx][0];
    gridH = SIZES[curSizeIdx][1];
    resetGrid();
    randomizeGrid(0.2f);
  }

  {
    std::scoped_lock<std::mutex> lock(gridMutex);
    if(doClear) resetGrid();
    if(doRandom) randomizeGrid(0.25f);
  }

  // Advance simulation
  double interval = 1000.0 / speed;
  double elapsed = (Time::now() - lastStep).toMilliSecondsDouble();

  static double lastStepMs = 0;
  if(doStep || (running && elapsed >= interval)){
    Time t = Time::now();
    {
      std::scoped_lock<std::mutex> lock(gridMutex);
#ifdef ICL_HAVE_OPENCL
      if(useGPU && clProg){
        stepGPU();
      } else
#endif
      {
        stepCPU();
      }
      ++generation;
      gridDirty = true;
    }
    lastStepMs = (Time::now() - t).toMilliSecondsDouble();
    lastStep = Time::now();
  }

  // Only re-render and recount when grid changed
  {
    std::scoped_lock<std::mutex> lock(gridMutex);
    if(gridDirty){
#ifdef ICL_HAVE_OPENCL
      if(useGPU && clProg){
        renderGPU(image);
      } else
#endif
      {
        renderGrid();
      }
      lastAlive = 0;
      for(int i = 0, n = gridW * gridH; i < n; ++i){
        if(grid[i] > 0) ++lastAlive;
      }
      gridDirty = false;
    }
  }

  DrawHandle draw = gui["draw"];
  draw = &image;
  draw.render();

  // Info label
  std::ostringstream oss;
  oss << "Gen " << generation
      << "  Alive: " << lastAlive
      << (useGPU ? "  [GPU]" : "  [CPU]")
      << std::fixed << std::setprecision(1) << "  " << lastStepMs << " ms";
  gui["info"] = oss.str();

  // Tool description
  static const char *toolDesc[] = {
    "Left-click to draw cells, right-click to erase",
    "Click to place a Glider (diagonal spaceship)",
    "Click to place a Lightweight Spaceship (LWSS)",
    "Click to place an R-pentomino (long-lived methuselah)",
    "Click to place an Acorn (stabilizes after 5206 gen)",
    "Click to place a Gosper Glider Gun (infinite growth)",
    "Click to place a Pulsar (period-3 oscillator)",
    "Click to place a Diehard (dies after 130 gen)"
  };
  int toolIdx = gui["tool"].as<ComboHandle>().getSelectedIndex();
  gui["info2"] = std::string(toolDesc[std::clamp(toolIdx, 0, 7)]);

  Thread::msleep(5);
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
