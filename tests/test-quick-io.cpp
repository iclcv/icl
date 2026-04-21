#include <icl/utils/Test.h>
#include <icl/qt/QuickIO.h>
#include <icl/qt/QuickCreate.h>
#include <icl/core/Img.h>

#include <cstdio>
#include <fstream>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

ICL_REGISTER_TEST("Quick2.IO.save.load.roundtrip", "save then load preserves data") {
  Image src = zeros(32, 32, 3, depth8u);
  src.as<icl8u>()(0, 0, 0) = 42;
  src.as<icl8u>()(0, 0, 1) = 84;
  src.as<icl8u>()(0, 0, 2) = 126;
  src.setFormat(formatRGB);

  std::string tmpFile = "/tmp/icl_test_quick2_io.ppm";
  save(src, tmpFile);

  Image loaded = load(tmpFile);
  ICL_TEST_TRUE(!loaded.isNull());
  ICL_TEST_EQ(loaded.getWidth(), 32);
  ICL_TEST_EQ(loaded.getHeight(), 32);

  // PPM preserves RGB pixel values
  loaded.visit([&](const auto &img) {
    ICL_TEST_EQ(static_cast<int>(img(0, 0, 0)), 42);
  });

  std::remove(tmpFile.c_str());
}

ICL_REGISTER_TEST("Quick2.IO.print.nocrash", "print does not crash") {
  Image img = zeros(10, 10, 1);
  ICL_TEST_NO_THROW(print(img));
}

ICL_REGISTER_TEST("Quick2.IO.print.null", "print null does not crash") {
  Image img;
  ICL_TEST_NO_THROW(print(img));
}
