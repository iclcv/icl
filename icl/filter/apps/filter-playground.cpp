// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Unified "mighty" filter app: pick one or more filters from a combo; the UI
// adapts automatically by reflecting each Op's Configurable properties via
// Prop(&op). Super-set of every retired single-op demo plus the old
// filter-array app (via -n N multi-stage pipelines).
//
// Source → stage 0 → stage 1 → ... → stage N-1. Each stage has its own filter
// combo, Prop(&op) panel, preview Display, and timing label; stages apply
// in series. N=1 by default.

#include <icl/qt/Common2.h>
#include <icl/qt/BoxHandle.h>
#include <icl/qt/MouseHandler.h>
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

#include <memory>
#include <mutex>

HSplit gui;
GenericGrabber grabber;

// One per pipeline stage. Written on the GUI thread by the filter-combo
// callback and read on the exec thread by run(); serialized by `mutex`
// (same pattern as single-stage mode — apply() holds it for the full
// currentOp->apply() duration).
struct Stage {
  std::unique_ptr<UnaryOp> op;
  GUI propGUI;                 // dynamic props panel for this stage
  std::string currentFilter;
  std::recursive_mutex mutex;
};
static std::vector<std::unique_ptr<Stage>> stages;

// Interactive-ROI state (source canvas left-click-drag). Written on the GUI
// thread by the mouse handler, read on the exec thread by run().
static std::mutex roiMtx;
static Rect interactiveRoi;
static Rect draggingRoi;

// Factory for the UnaryOps. Keep the order stable; the combo labels are
// derived from the first element.
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

// Rebuild a stage's props panel in response to its filter-combo change.
// Follows the CamCfgWidget pattern: hide the old Prop GUI, construct a new
// one targeting the new op, inject its root widget into the stage's
// persistent "props_{idx}" box.
static void rebuildStage(int idx, const std::string &name){
  auto &st = *stages[idx];
  std::scoped_lock lock(st.mutex);

  for(auto &[n, make] : filters()){
    if(n == name){
      st.op.reset(make());
      break;
    }
  }
  if(!st.op) return;

  BoxHandle props = gui.get<BoxHandle>("props_" + str(idx));
  if(st.propGUI.hasBeenCreated()){
    st.propGUI.hide();
  }
  st.propGUI = GUI(VBox().handle("propBox_" + str(idx)));
  st.propGUI << Prop(st.op.get()).label(name);
  st.propGUI.create();
  props.add(st.propGUI.getRootWidget());
  st.currentFilter = name;
}

static Rect roiFor(const std::string &mode, const Rect &full){
  if(mode == "full" || mode == "none") return full;
  if(mode == "interactive"){
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

// Builds one stage's GUI column: filter combo, dynamic props container,
// per-stage preview Display, per-stage apply-time label.
static GUI stageColumn(int idx){
  const std::string is = str(idx);
  return ( VBox().label("stage " + is).minSize(14, 1)
           << Combo(filterCombo()).handle("filter_" + is).maxSize(99,2)
           << VBox().handle("props_" + is).minSize(14,10)
           << Display().handle("preview_" + is).minSize(14,10)
           << Label("--").handle("dt_" + is).label("apply time").maxSize(99,2)
         );
}

void init(){
  const int N = std::max(1, pa("-n").as<int>());
  stages.resize(N);
  for(int i = 0; i < N; ++i) stages[i] = std::make_unique<Stage>();

  grabber.init(pa("-i"));

  GUI stagesBox = HBox();
  for(int i = 0; i < N; ++i) stagesBox << stageColumn(i);

  gui << ( VBox().minSize(24,14)
           << Canvas().handle("src").minSize(24,20).label("source")
           << ( VBox().label("source")
                << Combo("1:1,QVGA,!VGA,SVGA,XGA,WXGA,UXGA").handle("dsize").label("size")
                << Combo("!depth8u,depth16s,depth32s,depth32f,depth64f").handle("ddepth").label("depth")
                << Combo("gray,!rgb,hls,lab,yuv").handle("dformat").label("format")
                << Combo("!none,UL,UR,LL,LR,center,interactive").handle("roi").label("source ROI")
              )
           << ( HBox().maxSize(99,2)
                << Fps(10).handle("fps").label("fps")
                << Label("ok").handle("status").label("status")
              )
         )
      << stagesBox
      << Show();

  // One callback per stage combo. Needs the index captured — bind a handle
  // list per stage so the callback knows which one fired.
  for(int i = 0; i < N; ++i){
    const std::string h = "filter_" + str(i);
    gui.registerCallback([i, h](const std::string &){
      rebuildStage(i, gui[h].as<std::string>());
    }, h);
  }

  static MouseHandler mouseHandler(onSourceMouse);
  gui["src"].install(&mouseHandler);

  // Auto-range each stage's preview so 16s/32f stage outputs render over
  // their actual dynamic range instead of a fixed [0,255] window.
  for(int i = 0; i < N; ++i){
    gui.get<ImageHandle>("preview_" + str(i))->setRangeMode(ICLWidget::rmAuto);
  }

  // Initial filter for each stage = first entry in the combo.
  for(int i = 0; i < N; ++i) rebuildStage(i, filters().front().first);
}

void run(){
  grabber.useDesired<depth>(parse<depth>(gui["ddepth"]));
  grabber.useDesired<format>(parse<format>(gui["dformat"]));
  if(gui["dsize"].as<std::string>() == "1:1"){
    grabber.useDesired(Size::null);
  }else{
    grabber.useDesired<Size>(parse<Size>(gui["dsize"]));
  }

  Image src = grabber.grabImage();
  if(!src) return;

  // Apply source-ROI mode before any filter sees the image.
  Rect roi = roiFor(gui["roi"].as<std::string>(), src.getImageRect());
  Image flow = src.shallowCopy();   // threads through the pipeline stages
  flow.setROI(roi);

  // Source display: active ROI outlined in red; rubber-band in blue while
  // dragging.
  DrawHandle srcH = gui.get<DrawHandle>("src");
  srcH = flow;
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

  // Feed through each stage. On exception, short-circuit the rest of the
  // chain (the remaining previews keep their last successful frame).
  std::string status = "ok";
  for(size_t i = 0; i < stages.size(); ++i){
    Stage &st = *stages[i];
    std::scoped_lock lock(st.mutex);
    if(!st.op){ continue; }

    const std::string is = str(i);
    Time t0 = Time::now();
    Image result;
    try{
      st.op->apply(flow, result);
      gui["dt_" + is] = str((Time::now()-t0).toMilliSecondsDouble())+"ms";
      gui["preview_" + is] = result;
      flow = result;  // feeds the next stage
    }catch(const std::exception &e){
      status = "stage " + is + ": " + e.what();
      break;
    }
  }

  gui["status"] = status;
  gui["fps"].render();
}

int main(int n, char **ppc){
  pa_explain
    ("-input","grabber definition, e.g. '-i file lena.png' or '-i dc 0'")
    ("-n",    "number of filter stages (pipeline length), default 1");
  return ICLApp(n, ppc,
                "[m]-input|-i(device,device-params) "
                "-n|-num-filters(int=1)",
                init, run).exec();
}
