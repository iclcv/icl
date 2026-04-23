// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive demo exercising GenericGrabber's backend-swap path
// against the qt::Prop runtime rebuild that landed with Session 55.
//
// GenericGrabber is itself a Configurable (Session 54 flip), and its
// init() call destroys the current backend, fires
// removeChildConfigurable, builds a new backend, and re-attaches via
// addChildConfigurable with empty prefix.  qt::Prop subscribes to
// onChildSetChanged and rebuilds — backend-specific properties
// (format / size / gain / exposure / ...) appear or disappear live.
//
// Three buttons hook to different init() calls:
//   - create lena
//   - create parrot
//   - demo
//
// Launch: icl-grabber-backend-swap-demo

#include <icl/qt/Common2.h>

HSplit gui;
GenericGrabber grabber;

void swapBackend(const std::string &type, const std::string &id){
  // init() is re-entrant; destroys the current backend, fires
  // removeChildConfigurable on the old one, builds the new one,
  // addChildConfigurable's it.  Our Prop widget catches the child-set
  // change through Configurable::onChildSetChanged and rebuilds.
  //
  // The second arg is a param-map, parsed as "type=id[,type=id,...]" —
  // same shape the ProgArg form (init(pa)) feeds in: see
  // GenericGrabber.cpp:101.  Not just the raw id.
  grabber.init(type, id.empty() ? type : (type + "=" + id));
}

void init(){
  swapBackend("create", "lena");

  gui << ( VBox().maxSize(15, 99)
           << Button("create lena").handle("lena")
           << Button("create parrot").handle("parrot")
           << Button("create cameraman").handle("cameraman")
           << Button("demo").handle("demo")
         )
      << Prop(&grabber).label("grabber").minSize(24, 20)
      << Display().handle("img").minSize(16, 12).label("frame")
      << Show();
}

void run(){
  // Poll backend-swap buttons on the run thread.  init() holds the
  // grabber's internal mutex; grabImage() takes it too, so the two
  // serialize cleanly even if a swap lands mid-frame.
  if(gui["lena"].as<ButtonHandle>().wasTriggered())       swapBackend("create", "lena");
  if(gui["parrot"].as<ButtonHandle>().wasTriggered())     swapBackend("create", "parrot");
  if(gui["cameraman"].as<ButtonHandle>().wasTriggered())  swapBackend("create", "cameraman");
  if(gui["demo"].as<ButtonHandle>().wasTriggered())       swapBackend("demo", "");

  gui["img"] = grabber.grabImage();
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
