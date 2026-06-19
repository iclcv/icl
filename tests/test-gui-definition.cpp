// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Regression coverage for the structured GUIDefinition path (ui-plan Phase 7A).
//
// The GUI builder used to serialise every component to a stringly-typed
// definition (`type(params)[@handle=..@label=..]`) that GUIDefinition then
// re-parsed.  Any free-text payload containing the grammar metacharacters
// `, ( ) @ =` broke that round-trip and threw a syntax error at runtime.
// GUIDefinition now also has a constructor that copies a GUIComponent's
// structured fields directly — no grammar, so metacharacters survive.
//
// These tests exercise that constructor with no QApplication / GL context.
// The public designated-init components (qt::Label, qt::Slider, ...) build a
// GUIComponent through toComponent(); detail::Xxx are the legacy factories.

#include "harness/Test.h"
#include <icl/qt/GUI.h>
#include <icl/qt/GUIComponents.h>
#include <icl/qt/GUIDefinition.h>
#include <icl/qt/ui.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::utils;

// A Label's text is its single positional param.  The grammar metacharacters
// that used to break the envelope — `(`, `)`, `@`, `=` — must now survive
// verbatim in a single param (comma stays the param delimiter, so it is
// excluded here; String() is the component that escapes commas).
ICL_REGISTER_TEST("qt.GUIDefinition.label_text_with_metachars",
                  "parens / @ / = in a Label text no longer break parsing") {
  const std::string nasty = "Pos: (3 4) @scale=2 (done)";
  GUIDefinition def(Label(nasty).toComponent(), nullptr);
  ICL_TEST_EQ(def.type(), std::string("label"));
  ICL_TEST_EQ(def.numParams(), 1u);
  ICL_TEST_EQ(def.param(0), nasty);
}

// handle / label / tooltip options must also carry metacharacters verbatim
// (they used to be serialised as @handle=.. @label=.. @tooltip=..).
ICL_REGISTER_TEST("qt.GUIDefinition.options_with_metachars",
                  "handle/label/tooltip carry metacharacters verbatim") {
  const std::string h = "h(0)";
  const std::string l = "Label = a,b";
  const std::string t = "tip@home, (really)";
  GUIDefinition def(Label("x", {.handle=h, .label=l, .tooltip=t}).toComponent(), nullptr);
  ICL_TEST_EQ(def.handle(), h);
  ICL_TEST_EQ(def.label(), l);
  ICL_TEST_TRUE(def.hasToolTip());
  ICL_TEST_EQ(def.toolTip(), t);
}

// Numeric params keep splitting on commas and parse as before.
ICL_REGISTER_TEST("qt.GUIDefinition.slider_params",
                  "Slider min/max/curr parse from the structured param list") {
  GUIDefinition def(Slider(10, 200, 42).toComponent(), nullptr);
  ICL_TEST_EQ(def.type(), std::string("slider"));
  ICL_TEST_EQ(def.intParam(0), 10);
  ICL_TEST_EQ(def.intParam(1), 200);
  ICL_TEST_EQ(def.intParam(2), 42);
}

// The "string" type leading-empty-param special case is preserved: an empty
// initial text yields an empty first param, not a missing one.
ICL_REGISTER_TEST("qt.GUIDefinition.string_leading_empty_param",
                  "empty String() init text keeps a leading empty param") {
  GUIDefinition def(String("", {.maxLen=50}).toComponent(), nullptr);
  ICL_TEST_EQ(def.type(), std::string("string"));
  ICL_TEST_EQ(def.numParams(), 2u);
  ICL_TEST_EQ(def.param(0), std::string(""));
  ICL_TEST_EQ(def.param(1), std::string("50"));
}

// Faithfulness: for a payload with no metacharacters the structured ctor and
// the legacy string ctor produce identical params / handle / sizes.  Uses the
// legacy detail:: factory directly (it still exposes the fluent setters +
// toString()).
ICL_REGISTER_TEST("qt.GUIDefinition.structured_matches_legacy_string",
                  "structured and string ctors agree on a metachar-free component") {
  qt::detail::Slider s(0, 255, 128);
  s.handle("gain").minSize(4, 1).maxSize(8, 2);

  GUIDefinition a(s, nullptr);                 // structured (GUIComponent ctor)
  GUIDefinition b(s.toString(), nullptr);      // legacy string round-trip

  ICL_TEST_EQ(a.type(), b.type());
  ICL_TEST_EQ(a.numParams(), b.numParams());
  for(unsigned i = 0; i < a.numParams(); ++i) ICL_TEST_EQ(a.param(i), b.param(i));
  ICL_TEST_EQ(a.handle(), b.handle());
  ICL_TEST_EQ(a.minSize(), b.minSize());
  ICL_TEST_EQ(a.maxSize(), b.maxSize());
}

// End-to-end insertion (no Qt widgets): a labelled component is wrapped in a
// titled border, and the whole tree streams to XML without a parse crash even
// when the component's text carries envelope-breaking metacharacters.
// Exercises GUI::operator<<(GUIComponent) (structured label→border wrap) and
// to_string_recursive (structured traversal).
ICL_REGISTER_TEST("qt.GUI.labelled_component_wraps_in_border",
                  "a labelled component streams to a border-wrapped XML tree") {
  GUI g;
  g << Label("text with (parens) @ x=1", {.handle="lab", .label="My Border"});
  const std::string xml = g.createXMLDescription();
  ICL_TEST_TRUE(xml.find("<border") != std::string::npos);
  ICL_TEST_TRUE(xml.find("<label") != std::string::npos);
  ICL_TEST_TRUE(xml.find("handle=\"lab\"") != std::string::npos);
}

// Containers carry the structured component too: a labelled VBox wraps in a
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
