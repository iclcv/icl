/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/GenericPointCloudOutput.cp         **
** Module : ICLGeom                                                **
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

#include <ICLGeom/GenericPointCloudOutput.h>

#include <ICLUtils/PluginRegister.h>

namespace icl{
  using namespace utils;

  namespace geom{

    static PointCloudOutput *create_null_point_cloud_output(const std::map<std::string,std::string>&){
      struct NullOutput : public PointCloudOutput{
        virtual void send(const PointCloudObjectBase &){}
      };
      return new NullOutput;
    }

    REGISTER_PLUGIN(PointCloudOutput,null,create_null_point_cloud_output,
                    "Null output, not sending images",
                    "creation-string: dummy");

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
        std::cout << PluginRegister<PointCloudOutput>::instance().getRegisteredInstanceDescription()
                  << std::endl;
        throw ICLException("GenericPointCloudOutput list presented successfully");
      }
      ICL_DELETE(m_data->impl);
      std::map<std::string,std::string> data;
      data["creation-string"] = srcDescription;
      m_data->impl = PluginRegister<PointCloudOutput>::instance().createInstance(sourceType,data);
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

