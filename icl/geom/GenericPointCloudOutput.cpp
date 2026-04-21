// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/GenericPointCloudOutput.h>
#include <icl/geom/PointCloudOutputRegistry.h>

namespace icl{
  using namespace utils;

  namespace geom{

    static PointCloudOutput *create_null_point_cloud_output(const PointCloudOutputData&){
      struct NullOutput : public PointCloudOutput{
        virtual void send(const PointCloudObjectBase &){}
      };
      return new NullOutput;
    }

    REGISTER_POINT_CLOUD_OUTPUT(null, create_null_point_cloud_output,
                                "Null output, not sending images",
                                "creation-string: dummy")

    struct GenericPointCloudOutput::Data{
      PointCloudOutput *impl;
    };


    GenericPointCloudOutput::GenericPointCloudOutput():m_data(new Data)
    {
      m_data->impl = 0;
    }

    GenericPointCloudOutput::GenericPointCloudOutput(const std::string &sourceType,
                                                       const std::string &srcDescription):m_data(new Data){
      m_data->impl = 0;
      init(sourceType,srcDescription);
    }

    GenericPointCloudOutput::GenericPointCloudOutput(const ProgArg &pa):m_data(new Data){
      m_data->impl = 0;
      init(pa[0],pa[1]);
    }

    GenericPointCloudOutput::~GenericPointCloudOutput(){
      ICL_DELETE(m_data->impl);
      delete m_data;
    }

    void GenericPointCloudOutput::init(const std::string &sourceType, const std::string &srcDescription){
      if(sourceType == "list"){
        std::cout << pointCloudOutputInfoTable() << std::endl;
        throw ICLException("GenericPointCloudOutput list presented successfully");
      }
      ICL_DELETE(m_data->impl);
      PointCloudOutputData data;
      data["creation-string"] = srcDescription;
      const auto *e = pointCloudOutputRegistry().get(sourceType);
      if(!e){
        throw ICLException("GenericPointCloudOutput::init::unable to create"
                           " GenericPointOutput instance of type" + sourceType);
      }
      m_data->impl = e->payload(data).release();
      if(!m_data->impl){
        throw ICLException("GenericPointCloudOutput::init::unable to create"
                           " GenericPointOutput instance of type" + sourceType);
      }
    }

    void GenericPointCloudOutput::init(const utils::ProgArg &pa){
      init(pa[0],pa[1]);
    }


    bool GenericPointCloudOutput::isNull() const{
      return !m_data->impl;
    }

    void GenericPointCloudOutput::send(const PointCloudObjectBase &src){
      if(isNull()) throw ICLException("GenericPointCloudOutput::grab: called on a null instance");
      return m_data->impl->send(src);
    }
  }
}
