// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Unified "mighty" filter app: pick a filter from a combo; the UI adapts
// automatically by reflecting the Op's Configurable properties via Prop(&op).
// Super-set of what the individual per-op demos used to do.
//
// Currently enumerates the ICLFilter ops that have been migrated to the
// Configurable property system. As more ops migrate, add them to FILTERS.

#include <icl/qt/Common2.h>
#include <icl/filter/UnaryOp.h>
#include <icl/filter/AffineOp.h>
#include <icl/filter/BilateralFilterOp.h>
#include <icl/filter/CannyOp.h>
#include <icl/filter/ChamferOp.h>
#include <icl/filter/ConvolutionOp.h>
#include <icl/filter/DitheringOp.h>
#include <icl/filter/FFTOp.h>
#include <icl/filter/FixedConvertOp.h>
#include <icl/filter/GaborOp.h>
#include <icl/filter/GradientOp.h>
#include <icl/filter/IntegralImgOp.h>
#include <icl/filter/LocalThresholdOp.h>
#include <icl/filter/LUTOp.h>
#include <icl/filter/MedianOp.h>
#include <icl/filter/MirrorOp.h>
#include <icl/filter/MorphologicalOp.h>
#include <icl/filter/MotionSensitiveTemporalSmoothing.h>
#include <icl/filter/PseudoColorOp.h>
#include <icl/filter/RotateOp.h>
#include <icl/filter/ScaleOp.h>
#include <icl/filter/ThresholdOp.h>
#include <icl/filter/TranslateOp.h>
#include <icl/filter/UnaryArithmeticalOp.h>
#include <icl/filter/UnaryCompareOp.h>
#include <icl/filter/UnaryLogicalOp.h>
#include <icl/filter/WarpOp.h>
#include <icl/filter/WeightChannelsOp.h>
#include <icl/filter/WeightedSumOp.h>
#include <icl/filter/WienerOp.h>
#include <icl/qt/BoxHandle.h>
#include <icl/qt/MouseHandler.h>

#include <memory>
#include <mutex>

HSplit gui;
GenericGrabber grabber;
std::unique_ptr<UnaryOp> currentOp;
GUI propGUI;                 // dynamic props panel — rebuilt on filter change
std::string currentFilter;   // tracks the displayed filter to detect swaps
// Serializes currentOp swap (GUI thread, via the filter-combo callback) with
// currentOp->apply() in run() (ICLApp's exec thread). Without this, a filter
// swap frees the op mid-apply → segfault on the next m_data dereference.
std::recursive_mutex opMutex;

// Interactive-ROI state. Written on the GUI thread by the mouse handler,
// read on the exec thread by run(). `interactiveRoi` is the committed rect
// (Rect::null → default to full image). `draggingRoi` is the live rubber-band
// while the mouse button is held.
std::mutex roiMtx;
Rect interactiveRoi;
Rect draggingRoi;

// Factory for the UnaryOps currently known to the playground. Keep the order
// of this vector stable; the combo labels are derived from the first element.
static std::vector<std::pair<std::string, std::function<UnaryOp*()>>> &filters(){
  static std::vector<std::pair<std::string, std::function<UnaryOp*()>>> f = {
    {"AffineOp",         []{ return new AffineOp;         }},
    {"BilateralFilterOp",[]{ return new BilateralFilterOp;}},
    {"CannyOp",          []{ return new CannyOp;          }},
    {"ChamferOp",        []{ return new ChamferOp;        }},
    {"ConvolutionOp",    []{ return new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss3x3)); }},
    {"DitheringOp",      []{ return new DitheringOp;      }},
    {"FFTOp",            []{ return new FFTOp;            }},
    {"FixedConvertOp",   []{ return new FixedConvertOp(ImgParams(Size(320,240), formatRGB), depth8u); }},
    {"GaborOp",          []{ return new GaborOp;          }},
    {"GradientOp",       []{ return new GradientOp;       }},
    {"IntegralImgOp",    []{ return new IntegralImgOp;    }},
    {"LocalThresholdOp", []{ return new LocalThresholdOp;   }},
    {"LUTOp",            []{ return new LUTOp;            }},
    {"MedianOp",         []{ return new MedianOp(Size(3,3)); }},
    {"MirrorOp",         []{ return new MirrorOp(axisHorz);   }},
    {"MorphologicalOp",  []{ return new MorphologicalOp(MorphologicalOp::dilate); }},
    {"MSTS",             []{ return new MotionSensitiveTemporalSmoothing(-1, 20); }},
    {"PseudoColorOp",    []{ return new PseudoColorOp;    }},
    {"RotateOp",         []{ return new RotateOp;         }},
    {"ScaleOp",          []{ return new ScaleOp;          }},
    {"ThresholdOp",      []{ return new ThresholdOp;      }},
    {"TranslateOp",      []{ return new TranslateOp;      }},
    {"UnaryArithmeticalOp",[]{ return new UnaryArithmeticalOp; }},
    {"UnaryCompareOp",   []{ return new UnaryCompareOp;   }},
    {"UnaryLogicalOp",   []{ return new UnaryLogicalOp(UnaryLogicalOp::andOp, 255); }},
    {"WarpOp",           []{ return new WarpOp;           }},
    {"WeightChannelsOp", []{ return new WeightChannelsOp; }},
    {"WeightedSumOp",    []{ return new WeightedSumOp;    }},
    {"WienerOp",         []{ return new WienerOp(Size(5,5), 100.f); }},
  };
  return f;
}

static std::string filterCombo(){
  std::string out;
  for(auto &[name, _] : filters()){
    if(!out.empty()) out += ",";
    out += name;
  }
  return out;
}

// Rebuild the props panel for the given filter name. Follows the CamCfgWidget
// pattern: hide the old Prop GUI, construct a new one targeting &*currentOp,
// inject its root widget into the persistent "props" box.
static void rebuildPropPanel(const std::string &name){
  std::scoped_lock lock(opMutex);

  for(auto &[n, make] : filters()){
    if(n == name){
      currentOp.reset(make());
      break;
    }
  }
  if(!currentOp) return;

  BoxHandle props = gui.get<BoxHandle>("props");
  if(propGUI.hasBeenCreated()){
    propGUI.hide();
  }
  propGUI = GUI(VBox().handle("propBox"));
  propGUI << Prop(currentOp.get()).label(name);
  propGUI.create();
  props.add(propGUI.getRootWidget());
  currentFilter = name;
}

static Rect roiFor(const std::string &mode, const Rect &full){
  if(mode == "full" || mode == "none") return full;
  if(mode == "interactive"){
    // Committed rect, clamped to the current image. Empty → full image.
    std::scoped_lock lock(roiMtx);
    if(interactiveRoi == Rect::null) return full;
    Rect r = interactiveRoi & full;
    return r.getDim() > 0 ? r : full;
  }
  Size half = full.getSize()/2;
  if(mode == "center") return Rect(Point(full.width/4, full.height/4), half);
  Point off;
  if(mode[0] == 'L') off.y = full.height/2;  // L = lower
  if(mode[1] == 'R') off.x = full.width/2;   // R = right
  return Rect(off, half);
}

static void onSourceMouse(const MouseEvent &e){
  std::scoped_lock lock(roiMtx);
  if(e.isRight()){
    // Right-click anywhere clears the interactive ROI back to full image.
    interactiveRoi = Rect::null;
    draggingRoi    = Rect::null;
  }else if(e.isLeft()){
    if(e.isPressEvent()){
      draggingRoi = Rect(e.getPos(), Size(1,1));
    }else if(e.isDragEvent()){
      draggingRoi.width  = e.getX() - draggingRoi.x;
      draggingRoi.height = e.getY() - draggingRoi.y;
    }else if(e.isReleaseEvent()){
      interactiveRoi = draggingRoi.normalized();
      draggingRoi    = Rect::null;
    }
  }
}

void init(){
  grabber.init(pa("-i"));

  gui << ( VBox().minSize(24,14)
           << Canvas().handle("src").minSize(24,16).label("source")
           << Display().handle("dst").minSize(24,16).label("result")
         )
      << ( VBox().minSize(16,1)
           << Combo(filterCombo()).handle("filter").label("filter").maxSize(99,2)
           << ( VBox().label("source")
                << Combo("1:1,QVGA,!VGA,SVGA,XGA,WXGA,UXGA").handle("dsize").label("size")
                << Combo("!depth8u,depth16s,depth32s,depth32f,depth64f").handle("ddepth").label("depth")
                << Combo("gray,!rgb,hls,lab,yuv").handle("dformat").label("format")
                << Combo("!none,UL,UR,LL,LR,center,interactive").handle("roi").label("source ROI")
              )
           << VBox().handle("props").minSize(16,10)
           << ( HBox().maxSize(99,2)
                << Label("--").handle("dt").label("apply time")
                << Fps(10).handle("fps").label("fps")
              )
           << Label("ok").handle("status").label("status").maxSize(99,2)
         )
      << Show();

  gui.registerCallback([](const std::string &h){
    if(h == "filter") rebuildPropPanel(gui["filter"].as<std::string>());
  }, "filter");

  // Drag-select ROI on the source canvas (only honored when the "source ROI"
  // combo is set to "interactive"; other modes override). Right-click resets.
  static MouseHandler mouseHandler(onSourceMouse);
  gui["src"].install(&mouseHandler);

  // Auto-range the result view so 16s/32f filter outputs (gradients, FFT,
  // etc.) render over their actual dynamic range instead of the fixed
  // [0,255] window that renders them as mostly-black.
  gui.get<ImageHandle>("dst")->setRangeMode(ICLWidget::rmAuto);

  // Initial filter = first entry in the combo.
  rebuildPropPanel(filters().front().first);
}

void run(){
  // Configure grabber from the source controls.
  grabber.useDesired<depth>(parse<depth>(gui["ddepth"]));
  grabber.useDesired<format>(parse<format>(gui["dformat"]));
  if(gui["dsize"].as<std::string>() == "1:1"){
    grabber.useDesired(Size::null);
  }else{
    grabber.useDesired<Size>(parse<Size>(gui["dsize"]));
  }

  Image src = grabber.grabImage();
  if(!src) return;

  // Apply source-ROI mode (before filter sees the image).
  Rect roi = roiFor(gui["roi"].as<std::string>(), src.getImageRect());
  Image roiedSrc = src.shallowCopy();
  roiedSrc.setROI(roi);

  // Source display with the active ROI outlined in red, plus — if the user
  // is currently dragging — a transparent-blue rubber-band rect.
  DrawHandle srcH = gui.get<DrawHandle>("src");
  srcH = roiedSrc;
  srcH->color(255,0,0,255);
  srcH->fill(0,0,0,0);
  srcH->rect(roi);
  {
    std::scoped_lock lock(roiMtx);
    if(draggingRoi != Rect::null){
      srcH->color(0, 128, 255, 255);
      srcH->fill(0, 128, 255, 60);
      srcH->rect(draggingRoi);
    }
  }
  srcH.render();

  // Lock across the whole apply — the filter-combo callback on the GUI
  // thread can otherwise free currentOp mid-apply.
  std::scoped_lock lock(opMutex);
  if(!currentOp) return;

  Time t0 = Time::now();
  Image result;
  try{
    currentOp->apply(roiedSrc, result);
    gui["dt"] = str((Time::now()-t0).toMilliSecondsDouble())+"ms";
    gui["status"] = std::string("ok");
    gui["dst"] = result;
  }catch(const std::exception &e){
    gui["status"] = std::string(e.what());
  }
  gui["fps"].render();
}

int main(int n, char **ppc){
  pa_explain("-input","grabber definition, e.g. '-i file lena.png' or '-i dc 0'");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params)",init,run).exec();
}
