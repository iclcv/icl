/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/AdvancedMarkerGridDetector.cpp **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLMarkers/AdvancedMarkerGridDetector.h>
#include <ICLMarkers/MarkerGridEvaluater.h>

namespace icl{
  namespace markers{


    AdvancedMarkerGridDetector::AdvancedGridDefinition::AdvancedGridDefinition(const utils::Size &numCells,
                                                                               const utils::Size32f &markerBounds,
                                                                               const utils::Size32f &gridBounds,
                                                                               const std::vector<int> &markerIDs,
                                                                               const std::string &markerType)
      throw (utils::ICLException):MarkerGridDetector::GridDefinition(numCells,markerIDs, markerType){
      this->markerBounds = markerBounds;
      this->gridBounds = gridBounds;
    }

    utils::Rect32f AdvancedMarkerGridDetector::AdvancedGridDefinition::getBounds(int x, int y) const{
      const int w = getWidth(), h = getHeight(), mw = markerBounds.width, mh = markerBounds.height;
      const float dx = (gridBounds.width - w*mw)/(w-1);
      const float dy = (gridBounds.height - h*mh)/(h-1);
      return utils::Rect32f( utils::Point32f(x*(dx+mw),y*(dy+mh)), markerBounds);
    }

    void AdvancedMarkerGridDetector::Marker::KeyPoints::setup(const utils::Point32f corners[4],
                                                              const utils::Point32f &center){
      ur = corners[0];
      lr = corners[1];
      ll = corners[2];
      ul = corners[3];
      this->center = center;
    }

    std::vector<utils::Point32f>
    AdvancedMarkerGridDetector::Marker::KeyPoints::data() const {
      std::vector<utils::Point32f> ps(5);
      ps[0] = ur;
      ps[1] = lr;
      ps[2] = ll;
      ps[3] = ul;
      ps[4] = center;
      return ps;
    }

    std::vector<utils::Point32f>
    AdvancedMarkerGridDetector::Marker::KeyPoints::corners() const {
      std::vector<utils::Point32f> ps(4);
      ps[0] = ur;
      ps[1] = lr;
      ps[2] = ll;
      ps[3] = ul;
      return ps;
    }

    void AdvancedMarkerGridDetector::Marker::KeyPoints::appendCornersTo(std::vector<utils::Point32f> &dst) const{
      dst.push_back(ul);
      dst.push_back(ur);
      dst.push_back(lr);
      dst.push_back(ll);
    }



    AdvancedMarkerGridDetector::Marker::Marker():id(-1),found(false){}
    AdvancedMarkerGridDetector::Marker::Marker(int id, const utils::Point32f gridPoints[4],
                                               const utils::Point32f &center):
      id(id),found(false){
      gridPts.setup(gridPoints, center);
    }

    void AdvancedMarkerGridDetector::Marker::setImagePoints(const utils::Point32f corners[4],
                                                            const utils::Point32f &center){
      imagePts.setup(corners, center);
    }
    void AdvancedMarkerGridDetector::Marker::visTo(utils::VisualizationDescription &vd) const{
      if(!found) return;
      vd.polygon(imagePts.corners());
      vd.sym('+',imagePts.center);
      vd.text(imagePts.center, utils::str(id));
    }

    void AdvancedMarkerGridDetector::Marker::getImagePointsTo(utils::Point32f *dst) const{
      dst[0] = imagePts.ur;
      dst[1] = imagePts.lr;
      dst[2] = imagePts.ll;
      dst[3] = imagePts.ul;
    }




    AdvancedMarkerGridDetector::MarkerGrid::MarkerGrid(){}

    AdvancedMarkerGridDetector::MarkerGrid::MarkerGrid(const AdvancedGridDefinition &def){
      init(def);
    }

    void AdvancedMarkerGridDetector::MarkerGrid::init(const AdvancedGridDefinition &def){
      this->gridDef = def;
      setSize(def.getSize()); // this function is implemented in a lazy fashion!

      int i=0;
      for(int y=0;y<getHeight();++y){
        for(int x=0;x<getWidth();++x, ++i){
          utils::Rect32f r = def.getBounds(x,y);
          const utils::Point32f ps[4] = { r.ur(), r.lr(), r.ll(), r.ul() };
          (*this)(x,y) = Marker(def.getMarkerIDs()[i], ps, r.center());
        }
      }

    }

    void AdvancedMarkerGridDetector::MarkerGrid::update(const MarkerGridDetector::Result &r){
      for(int i=0;i<r.getDim();++i){
        const Fiducial &f = r[i];
        Marker &m = (*this)[i];
        if(f){
          m.setFound(true);
          const std::vector<markers::Fiducial::KeyPoint> &kps = f.getKeyPoints2D();
          if(kps.size() != 4){
            throw utils::ICLException("invalid key point number found");
          }
          utils::Point32f ps[4]; // ur, lr,  ll, ul
          for(int i=0;i<4;++i){
            const utils::Point32f &p = kps[i].markerPos;
            if(p.x < 0){
              if(p.y < 0) ps[3] = kps[i].imagePos;
              else ps[2] = kps[i].imagePos;
            }else{
              if(p.y < 0) ps[0] = kps[i].imagePos;
              else ps[1] = kps[i].imagePos;
            }
          }
          utils::Point32f c = f.getCenter2D();

          m.setImagePoints(ps,c);
        }else{
          m.setFound(false);
        }
      }
    }


    const AdvancedMarkerGridDetector::Marker &
    AdvancedMarkerGridDetector::MarkerGrid::getMarker(int id) const throw (utils::ICLException){
      int idx = gridDef.getIndex(id);
      if(idx < 0) throw utils::ICLException("invalid marker ID");
      return (*this)[idx];
    }

    utils::VisualizationDescription AdvancedMarkerGridDetector::MarkerGrid::vis() const{
      utils::VisualizationDescription vd;
      vd.color(255,0,0,255);
      vd.fill(0,0,0,0);
      for(Super::const_iterator it = begin(); it != Super::end(); ++it){
        it->visTo(vd);
      }
      return vd;
    }


    AdvancedMarkerGridDetector::AdvancedMarkerGridDetector(){

    }

    static utils::ParamList create_param_list(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def){
      return utils::ParamList("size",def.getMarkerBounds());
    }

    AdvancedMarkerGridDetector::AdvancedMarkerGridDetector(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def):
      Super(def,create_param_list(def)), grid(def){

    }
    void AdvancedMarkerGridDetector::init(const AdvancedMarkerGridDetector::AdvancedGridDefinition &def){
      Super::init(def,create_param_list(def));
      grid.init(def);
    }

    const AdvancedMarkerGridDetector::MarkerGrid &AdvancedMarkerGridDetector::detect(const core::ImgBase *image){
      grid.update(Super::detect(image));
      return grid;
    }
  }
}
