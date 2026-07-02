// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/markers/FiducialDetectorPluginSquareBCH.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/markers/BCHCode.h>
#include <icl/markers/Fiducial.h>
#include <numeric>
#include <vector>
#include <cstdint>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::geom;
using namespace icl::cv;

namespace icl::markers {

  struct FiducialDetectorPluginSquareBCH::Data{
    int n;                       // grid size
    SquareBCHCode code;          // the parameterized BCH code
    Img8u buffer;                // n x n sampling buffer
    std::vector<char> loaded;    // per-id enabled flag (size numIds)
    int maxLoaded;
    std::vector<Size32f> sizes;  // real-world size per id
    int border;                  // marker border in code cells
    int maxBCHErr;

    Data(int gridSize, int correctable)
      : n(gridSize), code(gridSize, correctable) {}
  };

  // binarize the n*n patch in place (mean-seeded 1-center-per-class k-means)
  namespace {
    void binarize(icl8u *p, int dim, int kmeansSteps){
      float mean = std::accumulate(p, p+dim, 0.0f) / dim;
      for(int s=0;s<kmeansSteps;++s){
        double lo=0, hi=0; int nlo=0, nhi=0;
        for(int i=0;i<dim;++i){ if(p[i]<mean){ lo+=p[i]; ++nlo; } else { hi+=p[i]; ++nhi; } }
        const float m = std::round(((nlo?lo/nlo:0) + (nhi?hi/nhi:255)) / 2.0f);
        mean = m;
      }
      const int t = (int)mean;
      for(int i=0;i<dim;++i) p[i] = 255*(p[i]>t);
    }
  }

  FiducialDetectorPluginSquareBCH::FiducialDetectorPluginSquareBCH(int gridSize, int correctable):
    data(new Data(gridSize, correctable)){
    data->buffer.setChannels(1);
    data->buffer.setSize(Size(data->n, data->n));

    data->loaded.assign(data->code.numIds(), 0);
    data->maxLoaded = -1;
    data->sizes.resize(data->code.numIds());

    addProperty("max bch errors",prop::Range{.min=0, .max=data->code.correctable(), .step=1},
                data->code.correctable(),
                "Maximum number of correctable binary BCH bit errors accepted");
    addProperty("match factor",prop::Range{.min=1, .max=10, .step=1}, 2,
                "Internal subsampling for matching (usually 2 is fine)");
    addProperty("border width",prop::Range{.min=1, .max=10, .step=1}, 2,
                "Marker border width in code-cell units");
    addProperty("binarize.k-means steps",utils::prop::Range{.min=1, .max=5, .step=1}, 2,
                "Number of k-means steps splitting black/white marker cells");
  }

  FiducialDetectorPluginSquareBCH::~FiducialDetectorPluginSquareBCH(){
    delete data;
  }

  void FiducialDetectorPluginSquareBCH::addOrRemoveMarkers(bool add, const std::string &which,
                                                           const ParamMap &l){
    Size s = l.at("size");
    const int N = data->code.numIds();
    std::vector<int> ids = parse_list_str(which);
    for(int x : ids){
      if(x<0 || x>=N) continue;
      data->loaded[x] = add;
      if(add) data->sizes[x] = s;
    }
    data->maxLoaded = -1;
    for(int i=N-1;i>=0;--i) if(data->loaded[i]){ data->maxLoaded = i; break; }
  }

  void FiducialDetectorPluginSquareBCH::prepareForPatchClassification(){
    data->maxBCHErr = prop("max bch errors").value;
  }

  FiducialImpl *FiducialDetectorPluginSquareBCH::classifyPatch(const Img8u &image, int *rot,
                                                               bool returnRejectedQuads, ImageRegion r){
    const int n = data->n, dim = n*n;
    image.scaledCopyROI(&data->buffer, interpolateRA);
    icl8u *p = data->buffer.begin(0);
    binarize(p, dim, prop("binarize.k-means steps").as<int>());

    // read the binarized n*n patch into a codeword (bit index = col + n*row,
    // matching SquareBCHCode's layout and the row-major buffer)
    uint64_t bits = 0;
    for(int i=0;i<dim;++i) if(p[i]) bits |= (1ull<<i);

    const SquareBCHCode::Decoded d = data->code.decode2D(bits);

    static Fiducial::FeatureSet supported = Fiducial::AllFeatures;
    static Fiducial::FeatureSet computed = ( Fiducial::Center2D |
                                             Fiducial::Rotation2D |
                                             Fiducial::Corners2D );

    if(d && d.id < (int)data->loaded.size() && data->loaded[d.id] && d.errors <= data->maxBCHErr){
      FiducialImpl *impl = new FiducialImpl(this,supported,computed,
                                            d.id, -1, data->sizes[d.id]);
      impl->imageRegion = r;
      *rot = d.rotation;
      return impl;
    }else if(returnRejectedQuads){
      *rot = 0;
      FiducialImpl *impl = new FiducialImpl(this,supported,computed, 999999, -1, Size(1,1));
      impl->imageRegion = r;
      return impl;
    }
    return 0;
  }

  void FiducialDetectorPluginSquareBCH::getQuadRectificationParameters(Size &markerSizeWithBorder,
                                                                       Size &markerSizeWithoutBorder){
    const int f = prop("match factor").value;
    const int b = prop("border width").value;
    const int n = data->n;
    markerSizeWithBorder    = Size(f*(2*b+n), f*(2*b+n));
    markerSizeWithoutBorder = Size(f*n, f*n);
  }

  Img8u FiducialDetectorPluginSquareBCH::createMarker(const std::string &whichOne, const Size &size,
                                                      const ParamMap &params){
    const int border = params.count("border width") ? (int)params.at("border width") : 2;
    return data->code.markerImage(parse<int>(whichOne), border, size);
  }

  } // namespace icl::markers
