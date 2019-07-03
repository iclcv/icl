/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerGridDetector.cpp       **
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

#include <ICLMarkers/MarkerGridDetector.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLUtils/StringUtils.h>

namespace  icl{
  namespace markers{
    struct MarkerGridDetector::Data{
      utils::SmartPtr<FiducialDetector> fd;
      utils::Array2D<Fiducial> fids;
      MarkerGridDetector::GridDefinition def;
    };

    MarkerGridDetector::GridDefinition::GridDefinition(){}

    MarkerGridDetector::GridDefinition::GridDefinition(const utils::Size &numCells,
                                                       const std::vector<int> &markerIDs,
                                                       const std::string &markerType) :
      numCells(numCells),ids(markerIDs),markerType(markerType){

      int n = getDim();
      if(!ids.size()){
        for(int i=0;i<n;++i) ids.push_back(i);
      }else{
        if((int)ids.size() != n){
          throw utils::ICLException("given number of marker IDs differs from grid size");
        }
        int i = 0;
        for(int y=0;y<numCells.height;++y){
          for(int x=0;x<numCells.width;++x,++i){
            posLUT[ids[i]] = utils::Point(x,y);
          }
        }
      }
    }

    utils::Point MarkerGridDetector::GridDefinition::getPos(int id) const{
      if(posLUT.size()){
        std::map<int,utils::Point>::const_iterator it = posLUT.find(id);
        if(it == posLUT.end()) return utils::Point(-1,-1);
        return it->second;
      }else{
        if(id >= getDim()) return utils::Point(-1,-1);
        return utils::Point(id%getWidth(), id/getWidth());
      }
    }

    int MarkerGridDetector::GridDefinition::getIndex(int id) const{
      if(posLUT.size()){
        std::map<int,utils::Point>::const_iterator it = posLUT.find(id);
        if(it == posLUT.end()) return -1;
        return it->second.x + it->second.y * getWidth();
      }else{
        if((unsigned)id >= (unsigned)getDim()){
          return -1;
        }
        return id;
      }
    }
    int MarkerGridDetector::GridDefinition::getWidth() const{
      return numCells.width;
    }
    int MarkerGridDetector::GridDefinition::getHeight() const{
      return numCells.height;
    }
    int MarkerGridDetector::GridDefinition::getDim() const{
      return numCells.getDim();
    }
    const utils::Size &MarkerGridDetector::GridDefinition::getSize() const{
      return numCells;
    }
    const std::string &MarkerGridDetector::GridDefinition::getMarkerType() const{
      return markerType;
    }
    const std::vector<int> &MarkerGridDetector::GridDefinition::getMarkerIDs() const{
      return ids;
    }

    MarkerGridDetector::MarkerGridDetector() : m_data(new Data){

    }

    MarkerGridDetector::MarkerGridDetector(const MarkerGridDetector::GridDefinition &griddef,
                                           const utils::ParamList &extraParams) :
      m_data(new Data){
      init(griddef, extraParams);
    }

    int MarkerGridDetector::getPos(int id){
      return m_data->def.getIndex(id);
    }

    void MarkerGridDetector::init(const MarkerGridDetector::GridDefinition &def,
                                  const utils::ParamList &extraParams) {
      m_data->def = def;
      m_data->fids = Result(def.getSize());
      m_data->fd = new FiducialDetector(def.getMarkerType(), "{"+utils::cat(def.getMarkerIDs(),",")+"}",
                                        extraParams);
      addChildConfigurable(m_data->fd.get());
    }

    MarkerGridDetector::~MarkerGridDetector(){
      delete m_data;
    }

    const MarkerGridDetector::GridDefinition &MarkerGridDetector::getGridDefinition() const{
      return m_data->def;
    }

    const MarkerGridDetector::Result &MarkerGridDetector::detect(const core::ImgBase *image){
      if(isNull()){
        throw utils::ICLException("MarkerGridDetector::detect: instance not yet initialized");
      }
      const std::vector<Fiducial> &fs = m_data->fd->detect(image);

      m_data->fids.fill(Fiducial());

      for(size_t i=0;i<fs.size();++i){
        m_data->fids[getPos(fs[i].getID())] = fs[i];
      }

      return m_data->fids;
    }

    bool MarkerGridDetector::isNull() const{
      return !m_data->fd;
    }

    FiducialDetector *MarkerGridDetector::getFiducialDetector(){
      return m_data->fd.get();
    }
  }
}
