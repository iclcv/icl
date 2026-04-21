#include <icl/utils/Test.h>
#include <icl/qt/QuickCreate.h>
#include <icl/core/Img.h>

using namespace icl;
using namespace icl::qt;
using namespace icl::core;
using namespace icl::utils;

// ---- zeros ----

ICL_REGISTER_TEST("Quick2.Create.zeros.default", "zeros returns depth32f by default") {
  Image img = zeros(64, 48, 3);
  ICL_TEST_EQ(img.getWidth(), 64);
  ICL_TEST_EQ(img.getHeight(), 48);
  ICL_TEST_EQ(img.getChannels(), 3);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth32f));
}

ICL_REGISTER_TEST("Quick2.Create.zeros.allZero", "all pixels are zero") {
  Image img = zeros(10, 10, 1);
  img.visit([](const auto &typed) {
    for(int i = 0; i < typed.getDim(); ++i)
      ICL_TEST_EQ(typed.getData(0)[i], decltype(typed.getData(0)[i])(0));
  });
}

ICL_REGISTER_TEST("Quick2.Create.zeros.depth8u", "zeros with explicit depth") {
  Image img = zeros(32, 32, 1, depth8u);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth8u));
}

// ---- ones ----

ICL_REGISTER_TEST("Quick2.Create.ones.value", "all pixels are one") {
  Image img = ones(10, 10, 1, depth32f);
  const auto &f = img.as<icl32f>();
  for(int i = 0; i < f.getDim(); ++i)
    ICL_TEST_NEAR(f.getData(0)[i], 1.0f, 1e-6f);
}

// ---- create ----

ICL_REGISTER_TEST("Quick2.Create.create.parrot", "creates a valid test image") {
  Image img = create("parrot");
  ICL_TEST_TRUE(!img.isNull());
  ICL_TEST_TRUE(img.getWidth() > 0);
  ICL_TEST_TRUE(img.getHeight() > 0);
  ICL_TEST_EQ(img.getChannels(), 3);
}

ICL_REGISTER_TEST("Quick2.Create.create.invalid", "invalid name returns null") {
  Image img = create("nonexistent_image_that_does_not_exist");
  ICL_TEST_TRUE(img.isNull());
}

// ---- load ----

ICL_REGISTER_TEST("Quick2.Create.load.invalid", "loading nonexistent file returns null") {
  Image img = load("/tmp/icl_nonexistent_file_test.png");
  ICL_TEST_TRUE(img.isNull());
}
