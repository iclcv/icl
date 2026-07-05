// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "camera-calibration-planar-GridIndicatorObject.h"
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/TextNode.h>
#include <icl/geom/Material.h>

namespace icl{
  using namespace utils;
  using namespace math;
  using namespace geom;   // Vec, GeomColor, geom_blue/geom_red (foundation, not scene graph)

  namespace markers{

    /// one grid cell: an extruded box (MeshNode) + a billboard id label (TextNode)
    struct GridIndicatorObject::MarkerObj : public geom2::GroupNode{
      int x, y;
      MarkerObj(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def,
                int x, int y) : x(x), y(y){
        Rect32f b = def.getBounds(x,y);
        static const float H = 2;

        auto box = std::make_shared<geom2::MeshNode>();
        for(float h = 0; h <= H; h+=H){
          box->addVertex(Vec(b.x, b.y, -h, 1));
          box->addVertex(Vec(b.right(), b.y, -h, 1));
          box->addVertex(Vec(b.right(), b.bottom(), -h, 1));
          box->addVertex(Vec(b.x, b.bottom(), -h, 1));
        }
        for(int h=0;h<2;++h){
          for(int i=0;i<4;++i){
            box->addLine(4*h+i, 4*h +(i+1) % 4, h ? geom_blue(255) : geom_red(255));
          }
        }
        for(int i=0;i<4;++i){
          box->addLine(i, i+4, geom_blue(255));
        }
        // geom2 addQuad carries no per-face colour — the translucent blue comes
        // from the node Material instead.
        box->addQuad(0,1,5,4);
        box->addQuad(1,2,6,5);
        box->addQuad(2,3,7,6);
        box->addQuad(3,0,4,7);
        box->setMaterial(Material::fromColor(geom_blue(100)));
        addChild(box);

        const std::vector<int> &ids = def.getMarkerIDs();
        int id = ids[x + y * def.getSize().width];

        // marker id label (was addTextTexture on the front face; now a TextNode)
        auto label = geom2::TextNode::create(str(id), b.height*0.5f, geom_blue(255));
        label->translate(b.x + b.width*0.5f, b.y + b.height*0.5f, -H);
        addChild(label);
      }
    };

    GridIndicatorObject::GridIndicatorObject(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def){
      for(int y=0;y<def.getSize().height;++y){
        for(int x=0;x<def.getSize().width;++x){
          addChild(std::make_shared<MarkerObj>(def,x,y));
        }
      }
    }

    GridIndicatorObject::GridIndicatorObject(const Size &cells, const Size32f &bounds){
      float dx = bounds.width/cells.width;
      float dy = bounds.height/cells.height;

      auto grid = std::make_shared<geom2::MeshNode>();
      for(int y=0;y<cells.height;++y){
        for(int x=0;x<cells.width;++x){
          grid->addVertex(Vec(x*dx, y*dy, 0, 1));
        }
      }
      for(int y=0;y<cells.height;++y){
        for(int x=0;x<cells.width;++x){
          int idx = x + cells.width * y;
          if(x){
            grid->addLine(idx, idx -1, geom_blue());
          }
          if(y){
            grid->addLine(idx, idx - cells.width, geom_blue());
          }
        }
      }
      addChild(grid);
    }
  }
}
