// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Characterization tests for markers::BCHCoder — the 36-bit (6x6) BCH marker
// code. These lock in the CURRENT observable contract (round-trip, marker-image
// geometry, rotation, error correction, and the actual code distance) so the
// planned modern-C++ reimplementation / generalization to smaller (3x3/4x4/5x5)
// codes can be validated against a known-good baseline. They intentionally test
// behavior, not implementation (the encode() lookup table, the magic XOR in
// decode(), etc. are all free to change as long as these hold).

#include "harness/Test.h"
#include <icl/markers/BCHCode.h>
#include <icl/core/Img.h>
#include <cstdint>
#include <vector>

using namespace icl;
using namespace icl::markers;
using icl::core::Img8u;
using icl::core::Channel8u;
using icl::utils::Size;

namespace {
  // render a 36-bit code as a 6x6 0/255 image (marker interior, no border)
  Img8u codeImage(const BCHCode &c) {
    Img8u im(Size(6, 6), 1);
    Channel8u ch = im[0];
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 6; ++x) ch(x, y) = 255 * (c[x + 6*y] ? 1 : 0);
    return im;
  }
}

ICL_REGISTER_TEST("markers.bch.roundtrip_all_ids",
                  "encode(id) then decode() recovers every id in [0,4095] with 0 errors")
{
  BCHCoder coder;
  for (int id = 0; id <= 4095; ++id) {
    const BCHCode c = BCHCoder::encode(id);
    const DecodedBCHCode d = coder.decode(c);
    ICL_TEST_TRUE((bool)d);            // successful decode
    ICL_TEST_EQ(d.id, id);
    ICL_TEST_EQ(d.errors, 0);
  }
}

ICL_REGISTER_TEST("markers.bch.encode_out_of_range_throws",
                  "encode() rejects ids outside [0,4095]")
{
  ICL_TEST_THROW(BCHCoder::encode(-1),   icl::utils::ICLException);
  ICL_TEST_THROW(BCHCoder::encode(4096), icl::utils::ICLException);
}

ICL_REGISTER_TEST("markers.bch.marker_image_geometry",
                  "createMarkerImage size = (6+2*border)^2, border pixels black, "
                  "interior 6x6 equals the encoded bits")
{
  const int id = 1023, border = 2;
  const Img8u m = BCHCoder::createMarkerImage(id, border);   // no upscale
  ICL_TEST_EQ(m.getSize(), Size(6 + 2*border, 6 + 2*border));
  ICL_TEST_EQ(m.getChannels(), 1);

  const Channel8u ch = m[0];
  // border ring is black
  for (int x = 0; x < m.getWidth(); ++x) {
    ICL_TEST_EQ((int)ch(x, 0), 0);
    ICL_TEST_EQ((int)ch(x, m.getHeight()-1), 0);
  }
  // interior equals encode() bits (0/255)
  const BCHCode c = BCHCoder::encode(id);
  for (int y = 0; y < 6; ++y)
    for (int x = 0; x < 6; ++x)
      ICL_TEST_EQ((int)ch(border+x, border+y), 255 * (c[x + 6*y] ? 1 : 0));

  // upscaled request yields the requested size
  const Img8u big = BCHCoder::createMarkerImage(id, border, Size(100, 100));
  ICL_TEST_EQ(big.getSize(), Size(100, 100));
}

ICL_REGISTER_TEST("markers.bch.rotate_code_is_order_four",
                  "rotateCode applied four times is the identity")
{
  for (int id : {0, 1, 42, 777, 4095}) {
    const BCHCode c = BCHCoder::encode(id);
    BCHCode r = c;
    for (int i = 0; i < 4; ++i) r = BCHCoder::rotateCode(r);
    ICL_TEST_TRUE(r == c);
  }
}

ICL_REGISTER_TEST("markers.bch.decode2d_recovers_rotation",
                  "decode2D recovers the id from any of the 4 orientations")
{
  BCHCoder coder;
  for (int id : {0, 1, 42, 777, 4095}) {
    BCHCode c = BCHCoder::encode(id);
    for (int k = 0; k < 4; ++k) {
      const Img8u im = codeImage(c);
      const DecodedBCHCode2D d = coder.decode2D(im, 4095, false);
      ICL_TEST_EQ(d.id, id);
      ICL_TEST_EQ(d.errors, 0);
      c = BCHCoder::rotateCode(c);   // next orientation
    }
  }
}

ICL_REGISTER_TEST("markers.bch.corrects_single_bit_errors",
                  "a single flipped bit is corrected back to the original id")
{
  BCHCoder coder;
  // sample of ids; single-bit flips at every one of the 36 positions
  for (int id : {0, 1, 5, 17, 63, 255}) {
    const BCHCode clean = BCHCoder::encode(id);
    for (int bit = 0; bit < 36; ++bit) {
      BCHCode noisy = clean;
      noisy.flip(bit);
      const DecodedBCHCode d = coder.decode(noisy);
      ICL_TEST_EQ(d.id, id);
      ICL_TEST_TRUE(d.errors >= 1 && d.errors <= 4);
    }
  }
}

ICL_REGISTER_TEST("markers.bch.min_hamming_distance",
                  "measured minimum pairwise Hamming distance over all 4096 codewords")
{
  std::vector<uint64_t> words(4096);
  for (int id = 0; id <= 4095; ++id)
    words[id] = BCHCoder::encode(id).to_ullong();

  int minDist = 36;
  for (int i = 0; i < 4096; ++i)
    for (int j = i+1; j < 4096; ++j) {
      const int hd = __builtin_popcountll(words[i] ^ words[j]);
      if (hd < minDist) minDist = hd;
    }
  // The shipped table is a true distance-9 code over ALL 4096 codewords
  // (d = 2t+1 = 9, corrects up to t=4 bit errors). NOTE: the BCHCode.h claim
  // "minimal hamming distance is 8" is WRONG (it is 9); but its remark that
  // higher indices have lower distance is CORRECT — see the prefix check below.
  ICL_TEST_EQ(minDist, 9);
}

ICL_REGISTER_TEST("markers.bch.corrects_up_to_4_bit_errors",
                  "2, 3 and 4 simultaneous bit errors are all corrected back to the id "
                  "(the code has distance 9 = corrects t=4 for every id)")
{
  BCHCoder coder;
  // deterministic distinct-bit-position error patterns of weight 2, 3, 4
  const std::vector<std::vector<int>> patterns = {
    {0,1}, {5,20}, {10,35}, {17,33},
    {0,1,2}, {3,14,25}, {8,19,30}, {11,22,34},
    {0,9,18,27}, {1,2,34,35}, {5,11,23,31}, {4,13,26,35},
  };
  for (int id : {0, 1, 5, 17, 63, 255, 1000, 4095}) {
    const BCHCode clean = BCHCoder::encode(id);
    for (const auto &bits : patterns) {
      BCHCode noisy = clean;
      for (int b : bits) noisy.flip(b);
      const DecodedBCHCode d = coder.decode(noisy);
      ICL_TEST_EQ(d.id, id);
      ICL_TEST_EQ(d.errors, (int)bits.size());
    }
  }
}

ICL_REGISTER_TEST("markers.bch.low_ids_have_larger_distance",
                  "prefix subsets [0,N) of low ids have strictly higher min distance "
                  "(so a calibration board with few markers should use low ids)")
{
  std::vector<uint64_t> words(256);
  for (int id = 0; id < 256; ++id) words[id] = BCHCoder::encode(id).to_ullong();

  auto minDistOfPrefix = [&](int N) {
    int md = 36;
    for (int i = 0; i < N; ++i)
      for (int j = i+1; j < N; ++j)
        md = std::min(md, __builtin_popcountll(words[i] ^ words[j]));
    return md;
  };

  // measured tiers: <=16 ids -> 11 (corrects 5), <=64 -> 10, then -> 9
  ICL_TEST_EQ(minDistOfPrefix(16), 11);
  ICL_TEST_EQ(minDistOfPrefix(64), 10);
  ICL_TEST_EQ(minDistOfPrefix(128), 9);
}
