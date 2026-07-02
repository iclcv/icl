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
#include <iostream>

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

// ============================ SquareBCHCode (n x n) ==========================

namespace {
  // min pairwise Hamming distance over the first `count` rendered patterns
  int measuredMinDistance(const SquareBCHCode &c, int count) {
    std::vector<uint64_t> w(count);
    for (int i = 0; i < count; ++i) w[i] = c.encode(i);
    int md = c.numBits();
    for (int i = 0; i < count; ++i)
      for (int j = i+1; j < count; ++j)
        md = std::min(md, __builtin_popcountll(w[i] ^ w[j]));
    return md;
  }
}

ICL_REGISTER_TEST("markers.squarebch.report_parameters",
                  "report id-space + measured min distance for each (grid, t) config")
{
  struct Cfg { int n, t; };
  const std::vector<Cfg> cfgs = {
    {3,1},{3,2}, {4,1},{4,2},{4,3}, {5,1},{5,2},{5,3},{5,4}, {6,4}
  };
  std::cout << "\n    grid  t  bits  ids       d(design)  d(measured, <=1024 ids)\n";
  for (const auto &cf : cfgs) {
    SquareBCHCode c(cf.n, cf.t);
    const int cap = std::min(c.numIds(), 1024);
    const int md  = cap > 1 ? measuredMinDistance(c, cap) : c.numBits();
    std::cout << "    " << c.gridSize() << "x" << c.gridSize()
              << "   " << c.correctable()
              << "  " << c.numBits()
              << "   " << c.numIds()
              << "\t   " << c.minDistance()
              << "         " << md << "\n";
    // the code must never do WORSE than its design distance on the sampled ids
    if (cap > 1) ICL_TEST_TRUE(md >= c.minDistance());
    ICL_TEST_TRUE(c.numIds() >= 2);
  }
  std::cout << std::flush;
}

ICL_REGISTER_TEST("markers.squarebch.roundtrip_and_error_correction",
                  "every id round-trips; up to t flipped bits are corrected back")
{
  struct Cfg { int n, t; };
  // exhaustive over modest id spaces (small k)
  const std::vector<Cfg> cfgs = { {3,1}, {4,2}, {5,3}, {5,4} };
  for (const auto &cf : cfgs) {
    SquareBCHCode c(cf.n, cf.t);
    const int n = c.gridSize(), t = c.correctable(), ids = c.numIds();
    for (int id = 0; id < ids; ++id) {
      const uint64_t clean = c.encode(id);
      // clean round-trip
      const auto d0 = c.decode(clean);
      ICL_TEST_TRUE((bool)d0);
      ICL_TEST_EQ(d0.id, id);
      ICL_TEST_EQ(d0.errors, 0);
      // flip the first t distinct bit positions -> must correct back
      uint64_t noisy = clean;
      for (int b = 0; b < t; ++b) noisy ^= (1ull << ((b * 7) % (n*n)));
      const auto d1 = c.decode(noisy);
      ICL_TEST_EQ(d1.id, id);
      ICL_TEST_EQ(d1.errors, t);
    }
  }
}

ICL_REGISTER_TEST("markers.squarebch.rotation_safe_prefix",
                  "measure how many low ids [0,N) decode2D recovers from ALL 4 "
                  "orientations (rotation shrinks effective distance)")
{
  struct Cfg { int n, t; };
  const std::vector<Cfg> cfgs = { {3,1}, {4,1}, {4,2}, {5,2}, {5,3}, {5,4}, {6,4} };
  std::cout << "\n    grid  t   ids     rotation-safe prefix\n";
  for (const auto &cf : cfgs) {
    SquareBCHCode c(cf.n, cf.t);
    const int cap = std::min(c.numIds(), 4096);
    int safe = 0;
    for (int id = 0; id < cap; ++id) {
      bool ok = true;
      uint64_t bits = c.encode(id);
      for (int r = 0; r < 4 && ok; ++r) { ok = (c.decode2D(bits).id == id); bits = c.rotate90(bits); }
      if (!ok) break;
      ++safe;
    }
    std::cout << "    " << c.gridSize() << "x" << c.gridSize()
              << "   " << c.correctable() << "   " << c.numIds()
              << "\t   " << safe << "\n";
    // every config must offer at least a handful of rotation-safe low ids —
    // enough for a coded calibration target (which allocates ids from 0 up)
    ICL_TEST_TRUE(safe >= 8);
  }
  std::cout << std::flush;
}

ICL_REGISTER_TEST("markers.squarebch.rotated_distance_and_selection",
                  "rotated min distance + automated rotation-robust id selection: "
                  "how many usable markers each grid yields at a target rotated distance")
{
  struct Cfg { int n, t; };
  const std::vector<Cfg> cfgs = { {3,1}, {4,1}, {4,2}, {5,2}, {5,3}, {5,4}, {6,4} };
  std::cout << "\n    grid  t   ids     | rotation-robust set size at min rotated distance d\n"
            <<   "                      |  d>=3   d>=5   d>=7   d>=9\n";
  for (const auto &cf : cfgs) {
    SquareBCHCode c(cf.n, cf.t);
    std::cout << "    " << c.gridSize() << "x" << c.gridSize()
              << "   " << c.correctable() << "   " << c.numIds() << "\t      | ";
    for (int d : {3, 5, 7, 9}) {
      // cap the greedy scan so huge id spaces (e.g. 5x5 t=1) stay cheap
      const std::vector<int> set = c.selectRotationRobustIds(d, 128);
      std::cout << "  " << set.size() << "\t";
      // the selected set must actually achieve the requested rotated distance
      if (set.size() >= 2) ICL_TEST_TRUE(c.rotatedMinDistance(set) >= d);
    }
    std::cout << "\n";
  }
  std::cout << std::flush;

  // sanity: rotated distance never exceeds the plain code distance, and a
  // selected d>=5 set for 4x4 t=2 is non-trivial
  SquareBCHCode c44(4, 2);
  const std::vector<int> s = c44.selectRotationRobustIds(5, 64);
  ICL_TEST_TRUE(s.size() >= 8);
  ICL_TEST_TRUE(c44.rotatedMinDistance(s) >= 5);
}

ICL_REGISTER_TEST("markers.squarebch.marker_image_geometry",
                  "markerImage size = (n+2*border)^2, border black, interior = code bits")
{
  SquareBCHCode c(4, 2);
  const int n = c.gridSize(), border = 1, id = 3;
  const Img8u m = c.markerImage(id, border);
  ICL_TEST_EQ(m.getSize(), Size(n + 2*border, n + 2*border));

  const Channel8u ch = m[0];
  for (int x = 0; x < m.getWidth(); ++x) {          // black border ring
    ICL_TEST_EQ((int)ch(x, 0), 0);
    ICL_TEST_EQ((int)ch(x, m.getHeight()-1), 0);
  }
  const uint64_t bits = c.encode(id);
  for (int y = 0; y < n; ++y)                        // interior == encoded bits
    for (int x = 0; x < n; ++x)
      ICL_TEST_EQ((int)ch(border+x, border+y), 255 * (int)((bits >> (x + n*y)) & 1));

  const Img8u big = c.markerImage(id, border, Size(80, 80));
  ICL_TEST_EQ(big.getSize(), Size(80, 80));
}

ICL_REGISTER_TEST("markers.squarebch.invalid_config_throws",
                  "a t too large for the grid (no data bits) is rejected")
{
  ICL_TEST_THROW(SquareBCHCode(3, 5), icl::utils::ICLException);   // 9 bits can't carry t=5
}
