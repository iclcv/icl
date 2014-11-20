/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/SoftObject.cpp               **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#include <ICLPhysics/SoftObject.h>
#include <BulletSoftBody/btSoftBody.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ICLUtils/StringUtils.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <map>
#include <fstream>
#include <ICLUtils/File.h>
#include <ICLUtils/StringUtils.h>
#include <ICLMath/FixedVector.h>

 std::ostream &operator<<(std::ostream &stream, btSoftBody::eAeroModel::_ m){
    switch(m){
      case btSoftBody::eAeroModel::V_Point: return stream << "V_Point";
      case btSoftBody::eAeroModel::V_OneSided: return stream << "V_OneSided";
      case btSoftBody::eAeroModel::V_TwoSided: return stream << "V_TwoSided";
      case btSoftBody::eAeroModel::F_OneSided: return stream << "F_OneSided";
      case btSoftBody::eAeroModel::F_TwoSided: return stream << "F_TwoSided";
      case btSoftBody::eAeroModel::END: return stream << "END";
      default:
        return stream <<"unknown value for eAeroModel!";
    }
  }

  std::istream &operator>>(std::istream &stream, btSoftBody::eAeroModel::_ &m){
    std::string s;    stream >> s;
    if(s == "V_Point") m = btSoftBody::eAeroModel::V_Point;
    else if(s == "V_OneSided") m = btSoftBody::eAeroModel::V_OneSided;
    else if(s == "V_TwoSided") m = btSoftBody::eAeroModel::V_TwoSided;
    else if(s == "F_OneSided") m = btSoftBody::eAeroModel::F_OneSided;
    else if(s == "F_TwoSided") m = btSoftBody::eAeroModel::F_TwoSided;
    else if(s == "END") m = btSoftBody::eAeroModel::END;
    else {
      WARNING_LOG("invalid aeromodel value!");
    }
    return stream;
  }

namespace icl{

  using namespace utils;
  using namespace core;
  using namespace geom;

namespace physics{
 

  btSoftBody *SoftObject::getSoftBody() {
    return dynamic_cast<btSoftBody*>(getCollisionObject());
  }
  const btSoftBody *SoftObject::getSoftBody() const {
    return dynamic_cast<const btSoftBody*>(getCollisionObject());
  }

  SoftObject::SoftObject(){
    m_usePoseMatching = true;
    m_useVolumeConversion = false; // todo: find the defaults !!
    registerCallback(function(this,&SoftObject::propertyChanged));
  }
  
  void SoftObject::createAllProperties(){
    btSoftBody *sb = getSoftBody();
    ICLASSERT_THROW(sb,ICLException("SoftObject::createAllProperties(): was called when internal soft-body object was still NULL"));
    btSoftBody::Config *cfg = &sb->m_cfg;
    btSoftBody::Material *mat = sb->m_materials[0];

    cfg->kSRHR_CL = 0.7;

    addProperty("general.aeromodel","menu","V_Point,V_OneSided,V_TwoSided,F_OneSided,F_TwoSided,END",cfg->aeromodel);
    addProperty("general.use volume conversion","flag","",false);
    addProperty("general.use pose matching","flag","",true);

    addProperty("coefficients.velocity correction","range:slider","[0.1,10]",cfg->kVCF);
    addProperty("coefficients.damping","range:slider","[0,1]",cfg->kDP);
    addProperty("coefficients.drag","range:slider","[0,10]",cfg->kDG);
    addProperty("coefficients.lift","range:slider","[0,10]",cfg->kLF);
    addProperty("coefficients.pressure","range:slider","[-100,100]",cfg->kPR);
    addProperty("coefficients.volume conversion","range:slider","[0,10]",cfg->kVC);
    addProperty("coefficients.dynamic friction","range:slider","[0,1]",cfg->kDF);
    addProperty("coefficients.pose matching","range:slider","[0,1]",cfg->kMT);
    addProperty("hardness.rigid contact","range:slider","[0,1]",cfg->kCHR);
    addProperty("hardness.kinetic contact","range:slider","[0,1]",cfg->kKHR);
    addProperty("hardness.soft contact","range:slider","[0,1]",cfg->kSHR);
    addProperty("hardness.anchors","range:slider","[0,1]",cfg->kAHR);
    addProperty("hardness.soft vs rigid","range:slider","[0,1]",cfg->kSRHR_CL);
    addProperty("hardness.soft vs kinetic","range:slider","[0,1]",cfg->kSKHR_CL);
    addProperty("hardness.soft vs soft","range:slider","[0,1]",cfg->kSSHR_CL);
    addProperty("stiffness.linear","range:slider","[0,1]",mat->m_kLST);
    addProperty("stiffness.angular","range:slider","[0,1]",mat->m_kAST);
    addProperty("stiffness.volume","range:slider","[0,1]",mat->m_kVST);
    addProperty("general.collision type","menu","default,soft+rigid","default");
  }
  
  void SoftObject::propertyChanged(const Configurable::Property &prop){
    btSoftBody *sb = getSoftBody();
    btSoftBody::Config *cfg = &sb->m_cfg;
    //btSoftBody::Material *mat = sb->m_materials[0];

    std::string section = "coefficients.";
    if(prop.name == "general.collision type"){
      if(prop.value == "default"){
        cfg->collisions = btSoftBody::fCollision::Default;
      }else{
        cfg->collisions = btSoftBody::fCollision::Default | btSoftBody::fCollision::CL_SELF;
      }
    } else if(prop.name == "general.aeromodel") { 
      cfg->aeromodel = parse<btSoftBody::eAeroModel::_>(prop.value); 
    } else if(prop.name == "general.use volume conversion") {
      m_useVolumeConversion = parse<bool>(prop.value);
      sb->setPose(m_useVolumeConversion,m_usePoseMatching);
    }else if(prop.name == "general.use pose matching"){
      m_usePoseMatching = parse<bool>(prop.value);
      sb->setPose(m_useVolumeConversion,m_usePoseMatching);
    }
#define CASE(X,Y) if(prop.name == (section+X)) { Y = parse<float>(prop.value); }
    
    else CASE("velocity correction",cfg->kVCF)
    else CASE("damping",cfg->kDP)
    else CASE("drag",cfg->kDG)
    else CASE("lift",cfg->kLF)
    else CASE("pressure",cfg->kPR)
    else CASE("volume conversion",cfg->kVC)
    else CASE("dynamic friction",cfg->kDF)    
    else CASE("pose matching",cfg->kMT)

    section = "hardness.";
    
    CASE("rigid contact",cfg->kCHR)
    else CASE("kinetic contact",cfg->kKHR)
    else CASE("soft contact",cfg->kSHR)
    else CASE("anchors",cfg->kAHR)
    else CASE("soft vs rigid",cfg->kSRHR_CL)
    else CASE("soft vs kinetic",cfg->kSKHR_CL)
    else CASE("soft vs soft",cfg->kSSHR_CL)

   //    section = "stiffness.";

    else if(prop.name == "stiffness.linear"){
      for(int i=0;i<sb->m_materials.size();++i){
        sb->m_materials[i]->m_kLST = parse<float>(prop.value);
      }
    }
    else if(prop.name == "stiffness.angular"){
      for(int i=0;i<sb->m_materials.size();++i){
        sb->m_materials[i]->m_kAST = parse<float>(prop.value);
      }
    }

    else if(prop.name == "stiffness.volume"){
      for(int i=0;i<sb->m_materials.size();++i){
        sb->m_materials[i]->m_kVST = parse<float>(prop.value);
      }
    }
    
  }

  SoftObject::SoftObject(const std::string &objFileName, PhysicsWorld *world) throw (ICLException){
    File file(objFileName,File::readText);
    if(!file.exists()) throw ICLException("Error in SceneObject(objFilename): unable to open file " + objFileName);

    setSmoothShading(true);

    m_usePoseMatching = true;
    m_useVolumeConversion = false; // todo: find the defaults !!
    registerCallback(function(this,&SoftObject::propertyChanged));

    setPhysicalObject(new btSoftBody(world->getWorldInfo()));

    //typedef FixedColVector<float,3> F3;
    //typedef FixedColVector<int,3> I3;

    int nSkippedVT = 0;
    //int nSkippedVN = 0;
    int nSkippedO = 0;
    int nSkippedG = 0;
    int nSkippedS = 0;
    int nSkippedMTLLIB = 0;
    int nSkippedUSEMTL = 0;

    int lineNr=0;

    while(file.hasMoreLines()){
      ++lineNr;
      std::string line = file.readLine();
      if(line.length()<2) continue;

      else if(line[0] == 'v'){ // most common: vertex
        switch(line[1]){
          case ' ':
            //m_vertices.push_back(parse<F3>(line.substr(2)).resize<1,4>(1));
            m_vertexColors.push_back(GeomColor(200,200,200,255));
            break;
          case 't': // texture coordinates u,v,[w] (w is optional) (this is skipped)
            ++nSkippedVT;
            break;
          case 'n': // normal for vertex x,y,z (might not be unit!)
            //m_normals.push_back(parse<F3>(line.substr(2)).resize<1,4>(1));
            break;
          default:
            ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [unknown format]");
            break;
        }
      }else if(line[0] == 'l'){ // line
        // f v1 v2 v3 v4 ... -> line strip
        std::vector<int> linestrip = parseVecStr<int>(line.substr(2)," ");
        for(unsigned int l=1;l<linestrip.size();++l){
          addLine(linestrip[l-1]-1,linestrip[l]-1);
        }
      }else if(line[0] == 'f'){
        // A: f v1 v2 v3 ..
        // B: f v1/vt1 v2/vt2 v3/vt3 .. with texture coordinate
        // C: f v1/vt1/n1 ..            with texture and normal index
        // D: f v1//n1 ..               with normal indices only
        const std::vector<std::string> x = tok(line.substr(2)," ");
        if(!x.size()) {
          ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [face definition expected]" );
          continue;
        }
        char C = 10;//get_format(x[0], lineNr); // we assume, that the format is the same here
        int n = (int)x.size();
        if( n < 3 ){
          ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [unsupported number of face vertices]" );
          continue;
        }
        switch(C){
          case 'A':
            if(n == 3){
              addTriangle(parse<int>(x[0])-1,parse<int>(x[1])-1,parse<int>(x[2])-1);
            }else if(n==4){
              addQuad(parse<int>(x[0])-1,parse<int>(x[1])-1,parse<int>(x[2])-1,parse<int>(x[3])-1);
            }else{
              std::vector<int> xx(x.size());
              for(unsigned int i=0;i<x.size();++i){
                xx[i] = parse<int>(x[i])-1;
              }
              addPolygon(xx.size(),xx.data());
            }
            break;
          case 'B': // for now, this is simple, we simply dont use the 2nd and 3rd token;
          case 'C':
          case 'D':{
            std::vector<int> is(n),ns(n);
            for(int i=0;i<n;++i){
              std::vector<int> t = parseVecStr<int>(x[i],"/");
              is[i] = t.at(0)-1;
              if(C == 'C'){
                ns[i] = t.at(2)-1;
              }else if(C == 'D'){
                ns[i] = t.at(1)-1;
              }
            }
            if(n == 3){
              if(C == 'B'){
                addTriangle(is[0],is[1],is[2]);
              }else{
                addTriangle(is[0],is[1],is[2],ns[0],ns[1],ns[2]);
              }
            }else if (n == 4){
              if( C == 'B'){
                addQuad(is[0],is[1],is[2],is[3]);
              }else{
                addQuad(is[0],is[1],is[2],is[3],ns[0],ns[1],ns[2],ns[3]);
              }
            }else{
              if( C == 'B'){
                addPolygon(is.size(),is.data());
              }else{
                addPolygon(is.size(),is.data(),GeomColor(0,100,255,255),ns.data());
              }
            }
          }
        }
      }else if(line[0] == '#') {
        if(line.substr(1,4) == "!icl"){
          std::string rest = line.substr(5);
          std::vector<std::string> ts = tok(rest," ");
          if(ts.size() < 2){
            WARNING_LOG("parsing object file " << objFileName << " (line: "
                        << line  << "): #!icl - line does not contain enough tokens!");
          }else if(ts[0] == "transformation"){
            setTransformation(parse<Mat>(cat(std::vector<std::string>(ts.begin()+1,ts.end())," ")));
          }else{
            WARNING_LOG("parsing object file " << objFileName << " (line: "
                        << line  << "): #!icl - line cannot be parsed!");
          }
        }
        continue;
      }else if(line[0] == ' ') {
        continue;
      }
      else if(line[0] == 's') {
        ++nSkippedS;
        continue;
      }else if(line[0] == 'o'){
        ++nSkippedO;
        continue;
      }else if(line[0] == 'g'){
        ++nSkippedG;
        continue;
      }else if(!line.find("#")) {
        continue; // comment
      }else if(!line.find("usemtl")) {
        ++nSkippedUSEMTL;
        continue; // todo try to load material description
      }else if(!line.find("mtllib")){
        ++nSkippedMTLLIB;
        continue;
      }else{
        ERROR_LOG("skipping line " + str(lineNr) + ":\"" + line + "\" [unknown format]" );
        continue;
      }
    }
    setVisible(Primitive::line,false);
    setVisible(Primitive::vertex,false);
    setVisible(Primitive::triangle,true);
    setVisible(Primitive::quad,true);
    setVisible(Primitive::polygon,true);
  }

}
}
