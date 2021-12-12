/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/SimpleBlobSearcher.cpp                 **
** Module : ICLCV                                                  **
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

#include <ICLCV/SimpleBlobSearcher.h>
#include <ICLCV/RegionDetector.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{


    SimpleBlobSearcher::Blob::Blob(const ImageRegion *region,
                                   const Color &refColor,
                                   int refColorIndex):
      region(region),refColor(refColor),refColorIndex(refColorIndex){}

    struct SimpleBlobSearcher::Data{
      std::vector<Color> colors;
      std::vector<float> thresholds;
      std::vector<Range32s> ranges;
      std::vector<Blob> blobs;
      std::vector<Img8u> buffers;
      std::vector<RegionDetector*> rds;
    };

    SimpleBlobSearcher::SimpleBlobSearcher() :
      m_data(new SimpleBlobSearcher::Data) {}

    SimpleBlobSearcher::~SimpleBlobSearcher(){
      delete m_data;
    }

    void SimpleBlobSearcher::add(const Color &color,
                                 float thresh,
                                 const Range32s &sizeRange){
      m_data->colors.push_back(color);
      m_data->thresholds.push_back(thresh);
      m_data->rds.push_back(new RegionDetector);
      m_data->ranges.push_back(sizeRange);
    }

    void SimpleBlobSearcher::remove(int index){
      ICLASSERT_RETURN(index >= 0 && index < (int)m_data->colors.size());
      m_data->colors.erase(m_data->colors.begin()+index);
      m_data->thresholds.erase(m_data->thresholds.begin()+index);
      delete m_data->rds[index];
      m_data->rds.erase(m_data->rds.begin()+index);
      m_data->ranges.erase(m_data->ranges.begin()+index);
    }

    void SimpleBlobSearcher::adapt(int index, const Color &color,
                                   float thresh, const Range32s &sizeRange){
      ICLASSERT_RETURN(index >= 0 && index < (int)m_data->colors.size());
      m_data->colors[index] = color;
      m_data->thresholds[index] = thresh;
      m_data->rds.push_back(new RegionDetector);
      m_data->ranges[index] = sizeRange;
    }

    void SimpleBlobSearcher::clear() {
      for (unsigned int i=0;i<m_data->colors.size();++i) {
        remove(i);
      }
    }

    static int square(int i){ return i*i; }

    const std::vector<SimpleBlobSearcher::Blob> &SimpleBlobSearcher::detect(const Img8u &image){
      m_data->blobs.clear();
      ICLASSERT_RETURN_VAL(image.getChannels() == 3, m_data->blobs);

      int N = (int)m_data->colors.size();
      int dim = image.getDim();

      m_data->buffers.resize(N);
      std::vector<icl8u*> dst(N);
      for(int i=0;i<N;++i){
        m_data->buffers[i].setSize(image.getSize());
        m_data->buffers[i].setChannels(1);
        dst[i] = m_data->buffers[i].begin(0);
      }

      const icl8u *data[3] = {image.begin(0),image.begin(1),image.begin(2)};

      for(int i=0;i<dim;++i){
        register const icl8u &r=data[0][i], &g=data[1][i], &b=data[2][i];
        for(int j=0;j<N;++j){
          int x = square(r-m_data->colors[j][0])+square(g-m_data->colors[j][1])+square(b-m_data->colors[j][2]);
          dst[j][i] = 255*( x < square(m_data->thresholds[j]));
        }
      }


      for(int j=0;j<N;++j){
        RegionDetector &rd = *m_data->rds[j];
        rd.setConstraints(m_data->ranges[j].minVal,m_data->ranges[j].maxVal,250,255);
        const std::vector<ImageRegion> &rs = rd.detect(&m_data->buffers[j]);
        for(unsigned int i=0;i<rs.size();++i){
          m_data->blobs.push_back(Blob(&rs[i],m_data->colors[j],j));

        }
      }
      return m_data->blobs;
    }
  } // namespace cv
}
