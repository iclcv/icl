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

// ---- zeros for all depths ----

ICL_REGISTER_TEST("Quick2.Create.zeros.depth16s", "zeros with depth16s") {
  Image img = zeros(16, 16, 1, depth16s);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth16s));
  img.visit([](const auto &typed) {
    for(int i = 0; i < typed.getDim(); ++i)
      ICL_TEST_EQ(typed.getData(0)[i], decltype(typed.getData(0)[i])(0));
  });
}

ICL_REGISTER_TEST("Quick2.Create.zeros.depth32s", "zeros with depth32s") {
  Image img = zeros(16, 16, 1, depth32s);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth32s));
  img.visit([](const auto &typed) {
    for(int i = 0; i < typed.getDim(); ++i)
      ICL_TEST_EQ(typed.getData(0)[i], decltype(typed.getData(0)[i])(0));
  });
}

ICL_REGISTER_TEST("Quick2.Create.zeros.depth32f", "zeros with depth32f explicit") {
  Image img = zeros(16, 16, 1, depth32f);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth32f));
  const auto &f = img.as<icl32f>();
  for(int i = 0; i < f.getDim(); ++i)
    ICL_TEST_NEAR(f.getData(0)[i], 0.f, 1e-6f);
}

ICL_REGISTER_TEST("Quick2.Create.zeros.depth64f", "zeros with depth64f") {
  Image img = zeros(16, 16, 1, depth64f);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth64f));
  const auto &d = img.as<icl64f>();
  for(int i = 0; i < d.getDim(); ++i)
    ICL_TEST_NEAR(d.getData(0)[i], 0.0, 1e-12);
}

// ---- ones for all depths ----

ICL_REGISTER_TEST("Quick2.Create.ones.depth8u", "ones with depth8u") {
  Image img = ones(8, 8, 1, depth8u);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth8u));
  ICL_TEST_EQ(img.as<icl8u>()(0, 0, 0), icl8u(1));
}

ICL_REGISTER_TEST("Quick2.Create.ones.depth16s", "ones with depth16s") {
  Image img = ones(8, 8, 1, depth16s);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth16s));
  ICL_TEST_EQ(img.as<icl16s>()(0, 0, 0), icl16s(1));
}

ICL_REGISTER_TEST("Quick2.Create.ones.depth32s", "ones with depth32s") {
  Image img = ones(8, 8, 1, depth32s);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth32s));
  ICL_TEST_EQ(img.as<icl32s>()(0, 0, 0), icl32s(1));
}

ICL_REGISTER_TEST("Quick2.Create.ones.depth64f", "ones with depth64f") {
  Image img = ones(8, 8, 1, depth64f);
  ICL_TEST_EQ(static_cast<int>(img.getDepth()), static_cast<int>(depth64f));
  ICL_TEST_NEAR(img.as<icl64f>()(0, 0, 0), 1.0, 1e-12);
}

// ---- create with formatGray ----

ICL_REGISTER_TEST("Quick2.Create.create.gray", "create with formatGray returns 1 channel") {
  Image img = create("parrot", formatGray);
  ICL_TEST_TRUE(!img.isNull());
  ICL_TEST_EQ(img.getChannels(), 1);
  ICL_TEST_EQ(static_cast<int>(img.getFormat()), static_cast<int>(formatGray));
}

// ---- grab with create device ----

ICL_REGISTER_TEST("Quick2.Create.grab.create", "grab with create device returns valid image") {
  Image img = grab("create", "parrot");
  ICL_TEST_TRUE(!img.isNull());
  ICL_TEST_TRUE(img.getWidth() > 0);
  ICL_TEST_TRUE(img.getHeight() > 0);
  ICL_TEST_EQ(img.getChannels(), 3);
}
