// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Coverage for the polymorphic GUIComponent design (ui-plan Phase 7 +
// re-engineering): components are typed structs that build their own widget
// via a virtual createWidget(); there is no stringly-typed param channel, so
// free-text payloads — including the commas that the old comma-split mangled —
// are carried verbatim as typed fields. These tests need no QApplication / GL.

#include "harness/Test.h"
#include <icl/qt/GUI.h>
#include <icl/qt/ui.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::utils;

// A component's free-text payload is a typed field: every grammar metacharacter
// — including the comma that used to split params — survives verbatim. This is
// the bug class the redesign eliminates structurally.
ICL_REGISTER_TEST("qt.GUI.payload_metachars_survive",
                  "component free-text fields carry , ( ) @ = verbatim") {
  const std::string nasty = "a, b (c) @scale=2, done";
  Label l(nasty, {.handle="h(0)", .label="L = a,b"});
  ICL_TEST_EQ(l.text, nasty);
  ICL_TEST_EQ(l.options().handle, std::string("h(0)"));
  ICL_TEST_EQ(l.options().label,  std::string("L = a,b"));

  // Combo's CSV is split at the component (not by a grammar), so its entries
  // are exactly what the user wrote.
  Combo c("alpha,beta,gamma", {.initialIndex=2, .handle="sel"});
  ICL_TEST_EQ(c.entries, std::string("alpha,beta,gamma"));
  ICL_TEST_EQ(c.opts.initialIndex, 2);
}

// clone() is polymorphic — the GUI tree stores shared_ptr<GUIComponent> and
// must not slice the concrete type away.
ICL_REGISTER_TEST("qt.GUI.clone_preserves_dynamic_type",
                  "GUIComponent::clone() keeps the concrete component type") {
  Slider s(0, 255, 42, {.handle="gain"});
  std::shared_ptr<GUIComponent> c = s.clone();
  Slider *back = dynamic_cast<Slider*>(c.get());
  ICL_TEST_TRUE(back != nullptr);
  ICL_TEST_EQ(back->min, 0);
  ICL_TEST_EQ(back->max, 255);
  ICL_TEST_EQ(back->val, 42);
  ICL_TEST_EQ(back->options().handle, std::string("gain"));
}

// End-to-end tree build (no Qt widgets): a labelled leaf wraps in a titled
// border and the whole tree streams to XML; a metachar payload never throws.
ICL_REGISTER_TEST("qt.GUI.labelled_leaf_wraps_in_border",
                  "a labelled component streams to a border-wrapped XML tree") {
  GUI g;
  g << Slider(0, 10, 5, {.handle="s", .label="Gain, (dB)"});
  const std::string xml = g.createXMLDescription();
  ICL_TEST_TRUE(xml.find("<border") != std::string::npos);
  ICL_TEST_TRUE(xml.find("<slider") != std::string::npos);
  ICL_TEST_TRUE(xml.find("handle=\"s\"") != std::string::npos);
}

// Containers carry the polymorphic component too: a labelled VBox wraps in a
// border and its children survive the traversal.
ICL_REGISTER_TEST("qt.GUI.labelled_container_wraps_in_border",
                  "a labelled container streams to a border-wrapped XML tree") {
  GUI g;
  g << ( VBox({.handle="box", .label="Panel, (1)"})
         << Button("OK", {.handle="ok"})
         << Slider(0, 10, 5, {.handle="s"}) );
  const std::string xml = g.createXMLDescription();
  ICL_TEST_TRUE(xml.find("<border") != std::string::npos);
  ICL_TEST_TRUE(xml.find("<vbox") != std::string::npos);
  ICL_TEST_TRUE(xml.find("<button") != std::string::npos);
  ICL_TEST_TRUE(xml.find("<slider") != std::string::npos);
}

// --- KeyboardHandler (the app-level keyboard input added for the driving game).
//     The held-key set + chain dispatch is pure logic — no QApplication needed.
#include <icl/qt/KeyboardHandler.h>

ICL_REGISTER_TEST("qt.keyboard.held_set", "KeyboardHandler tracks held keys across press/release")
{
  KeyboardHandler k;
  ICL_TEST_TRUE(!k.held(42));

  // press -> held; the base process() is observe-only (Forward)
  ICL_TEST_TRUE(k.process({42, true, 0}) == KeyResult::Forward);
  ICL_TEST_TRUE(k.held(42));
  ICL_TEST_TRUE(!k.held(43));

  k.process({43, true, 0});
  ICL_TEST_TRUE(k.held(42) && k.held(43));

  // release -> not held
  k.process({42, false, 0});
  ICL_TEST_TRUE(!k.held(42) && k.held(43));

  k.clearHeld();
  ICL_TEST_TRUE(!k.held(43));
}

// Regression: a *labeled* typed component must keep its dynamic type through the
// border-wrap in GUI::operator<<. A `GUIComponent inner = component` slice there
// dropped the createWidget() override -> "component type 'color' has no widget
// factory" at create time (only on a real GL display, which is why it slipped
// past the headless smoke tests). Here we assert the stored component is still
// the concrete ColorSelect, no QApplication needed.
ICL_REGISTER_TEST("qt.gui.labeled_component_not_sliced", "a labeled component keeps its dynamic type (no slice)")
{
  GUI g;
  g << ColorSelect(10, 20, 30, {.label = "bg"});

  // labeled -> wrapped in a border GUI whose single child carries the component
  ICL_TEST_EQ(g.getChildCount(), 1);
  const GUI *border = g.getChild(0);
  ICL_TEST_TRUE(border != nullptr);
  ICL_TEST_EQ(border->getChildCount(), 1);
  const GUIComponent *inner = border->getChild(0)->getComponent();
  ICL_TEST_TRUE(dynamic_cast<const ColorSelect*>(inner) != nullptr);   // NOT sliced to base

  // and an unlabeled one is stored directly, still typed
  GUI g2;
  g2 << ColorSelect(1, 2, 3, {.handle = "c"});
  ICL_TEST_EQ(g2.getChildCount(), 1);
  ICL_TEST_TRUE(dynamic_cast<const ColorSelect*>(g2.getChild(0)->getComponent()) != nullptr);
}
