// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive demo exercising the ImageCompressor codec-swap path
// against the qt::Prop runtime rebuild that landed with Session 55.
//
// Grab frames from any Grabber backend, compress+decompress through an
// ImageCompressor, and show the original alongside the decoded image.
// The Prop widget bound to the ImageCompressor lets the user flip the
// `mode` combo at runtime (raw / rlen / jpeg / 1611 / zstd — whichever
// codecs are registered in this build).  When the mode changes,
// ImageCompressor::installPlugin swaps its codec child Configurable —
// qt::Prop observes the child-set change, rebuilds its widget tree, and
// codec-specific knobs (quality, level, ...) appear or disappear live.
//
// Runs at grab-tick rate; a Label shows the current compression ratio
// so effects of quality knobs are immediately visible.
//
// Launch: icl-compressor-playground -i create lena

#include <icl/qt/Common2.h>
#include <icl/io/ImageCompressor.h>

HSplit gui;
GenericGrabber grabber;
ImageCompressor compressor;

void init(){
  grabber.init(pa("-i"));

  gui << Display().handle("orig").label("original").minSize(16, 12)
      << Display().handle("decoded").label("decode(compress(img))").minSize(16, 12)
      << ( VBox().maxSize(22, 99)
           << Prop(&compressor).label("compressor")
           << Label("-").handle("ratio").label("compression ratio")
           << Label("-").handle("enc_time").label("encode (ms)")
           << Label("-").handle("dec_time").label("decode (ms)")
         )
      << Show();
}

void run(){
  Image img = grabber.grabImage();
  gui["orig"] = img;

  // A codec may reject the input — e.g. `1611` only handles
  // single-channel icl16s depth frames; feeding it an RGB icl8u
  // throws.  Surface the error in the UI instead of aborting so the
  // user can flip back to a compatible codec.
  try {
    const Time t0   = Time::now();
    auto data       = compressor.compress(img);
    const Time t1   = Time::now();
    Image decoded   = compressor.uncompress(data.bytes, data.len);
    const Time t2   = Time::now();

    gui["decoded"]  = decoded;
    gui["ratio"]    = str(data.compressionRatio);
    gui["enc_time"] = str((t1 - t0).toMilliSecondsDouble()) + " ms";
    gui["dec_time"] = str((t2 - t1).toMilliSecondsDouble()) + " ms";
  } catch (const utils::ICLException &e) {
    gui["ratio"]    = std::string("ERR: ") + e.what();
    gui["enc_time"] = std::string("-");
    gui["dec_time"] = std::string("-");
  }
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "[m]-input|-i(2)", init, run).exec();
}
