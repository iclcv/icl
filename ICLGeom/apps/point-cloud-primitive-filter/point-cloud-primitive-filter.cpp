/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/point-cloud-primitive-filter/             **
**          point-cloud-primitive-filter.cpp                       **
** Module : ICLGeom                                                **
** Authors: Lukas Twardon, Tobias Roehlig, Christof Elbrechter     **
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

#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <rsb/Handler.h>
#include <rsb/Listener.h>
#include <rsb/Factory.h>
#include <rsb/converter/Repository.h>
#include <rsb/converter/ProtocolBufferConverter.h>

#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>

#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/GenericPointCloudGrabber.h>
#include <ICLGeom/GenericPointCloudOutput.h>
#include <ICLGeom/Primitive3DFilter.h>
#include <ICLGeom/Primitive3DFloatSet.pb.h>


using namespace rsb;
using namespace rst::geometry;

HSplit gui;
Scene scene;

SceneObject *primitive_holder;

PointCloudObject obj;
GenericPointCloudGrabber grabber;
GenericPointCloudOutput output;
SmartPtr<Primitive3DFilter> primitiveFilter;

std::vector<Primitive3DFilter::Primitive3D> primitives;
icl::utils::Mutex primitivesMutex;

ListenerPtr primitivesetListener;

void getPrimitivesFromRSB(boost::shared_ptr<Primitive3DFloatSet> evPrimitiveSet){
  std::vector<Primitive3DFilter::Primitive3D> rsbPrimitives;
  for(int i = 0; i < evPrimitiveSet->primitives_size(); ++i) {
    Primitive3DFloat primitiveProto = evPrimitiveSet->primitives(i);
    Primitive3DFilter::PrimitiveType primitiveType = Primitive3DFilter::CUBE;
    Primitive3DFloat_PrimitiveType typeRST = primitiveProto.type();
    if(typeRST == Primitive3DFloat_PrimitiveType_CUBE)
        primitiveType = Primitive3DFilter::CUBE;
    else if(typeRST == Primitive3DFloat_PrimitiveType_SPHERE)
        primitiveType = Primitive3DFilter::SPHERE;
    else if(typeRST == Primitive3DFloat_PrimitiveType_CYLINDER)
        primitiveType = Primitive3DFilter::CYLINDER;
    Translation positionProto = primitiveProto.pose().translation();
    Vec primitivePosition(positionProto.x()*1000.0, positionProto.y()*1000.0, positionProto.z()*1000.0, 1);
    Rotation rotation = primitiveProto.pose().rotation();
    Primitive3DFilter::Quaternion primitiveOrientation(Vec3(rotation.qx(), rotation.qy(), rotation.qz()), rotation.qw());
    Vec primitiveScale(primitiveProto.scale().x()*1000.0, primitiveProto.scale().y()*1000.0, primitiveProto.scale().z()*1000.0, 1);
    Primitive3DFilter::Primitive3D primitive(primitiveType, primitivePosition, primitiveOrientation, primitiveScale, 0, primitiveProto.description());
    rsbPrimitives.push_back(primitive);
  }
  primitivesMutex.lock();
  primitives = rsbPrimitives;
  primitivesMutex.unlock();
}

void init(){
  grabber.init(pa("-pci"));
  if(pa("-pco")){
    output.init(pa("-pco"));
  }

  Camera cam;
  if(pa("-c")){
    cam = Camera(*pa("-c"));
  }

  gui << Draw3D().minSize(32,24).handle("scene")
      << ( VBox() << Fps(10).handle("fps").label("FPS")
                  << CheckBox("Filter with primitives from RSB").handle("filterRSBHandle")
                  << CheckBox("Filter with primitive from GUI").handle("filterGUIHandle")
                  << CheckBox("Show primitives").handle("showPrimitivesHandle")
                  << Combo("Cube, Sphere, Cylinder").label("primitive type").handle("primitiveTypeHandle")
                  << Slider(-5000,5000,0).out("posX").label("position x").handle("posXHandle")
                  << Slider(-5000,5000,0).out("posY").label("position y").handle("posYHandle")
                  << Slider(-5000,5000,0).out("posZ").label("position z").handle("posZHandle")
                  << FSlider(-1,1,0).out("orientX").label("orientation x").handle("orientXHandle")
                  << FSlider(-1,1,0).out("orientY").label("orientation y").handle("orientYHandle")
                  << FSlider(-1,1,1).out("orientZ").label("orientation z").handle("orientZHandle")
                  << FSlider(-6.28,6.28,0).out("orientAngle").label("orientation angle").handle("orientAngleHandle")
                  << Slider(1,5000,1000).out("scaleX").label("scale x").handle("scaleXHandle")
                  << Slider(1,5000,1000).out("scaleY").label("scale y").handle("scaleYHandle")
                  << Slider(1,5000,1000).out("scaleZ").label("scale z").handle("scaleZHandle"))
      << Show();

  // kinect camera
  scene.addCamera(cam);
  scene.setBounds(1000);
  scene.addObject(&obj,false);

  primitive_holder = new SceneObject();
  primitive_holder->setLockingEnabled(true);
  scene.addObject(primitive_holder,true);

  gui["scene"].link(scene.getGLCallback(0));
  gui["scene"].install(scene.getMouseHandler(0));

  obj.setPointSize(3);
  obj.setPointSmoothingEnabled(false);
  scene.setLightingEnabled(false);

  // rsb
  try {
    boost::shared_ptr< rsb::converter::ProtocolBufferConverter<Primitive3DFloatSet> >
        primitiveset_converter(new rsb::converter::ProtocolBufferConverter<Primitive3DFloatSet>());
    rsb::converter::converterRepository<std::string>()->registerConverter(primitiveset_converter);
    Factory& factory = getFactory();
    Scope primitivesetScope(*pa("-primitivescope"));
    primitivesetListener = factory.createListener(primitivesetScope);
    primitivesetListener->addHandler(HandlerPtr(new DataFunctionHandler<Primitive3DFloatSet> (&getPrimitivesFromRSB)));
  } catch(const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  // init primitive filter
  primitiveFilter = new Primitive3DFilter(Primitive3DFilter::FilterConfig(*pa("-config")));
}


void run(){
  obj.lock();
  grabber.grab(obj);

  primitive_holder->lock();
  primitive_holder->removeAllChildren();

  static CheckBoxHandle showPrimitives = gui["showPrimitivesHandle"];

  // Filter the point cloud with primitive from GUI
  static CheckBoxHandle filterGUI = gui["filterGUIHandle"];
  if(filterGUI.isChecked()) {
    Primitive3DFilter::PrimitiveType primitiveType;
    int primitiveTypeIndex = gui["primitiveTypeHandle"];
    if(primitiveTypeIndex == 0)
      primitiveType = Primitive3DFilter::CUBE;
    else if(primitiveTypeIndex == 1)
      primitiveType = Primitive3DFilter::SPHERE;
    else
      primitiveType = Primitive3DFilter::CYLINDER;
    Vec primitivePosition(gui["posX"], gui["posY"], gui["posZ"], 1);
    Primitive3DFilter::Quaternion primitiveOrientation(Vec3(gui["orientX"], gui["orientY"], gui["orientZ"]), gui["orientAngle"], true);
    Vec primitiveScale(gui["scaleX"], gui["scaleY"], gui["scaleZ"], 1);
    Primitive3DFilter::Primitive3D primitive(primitiveType, primitivePosition, primitiveOrientation, primitiveScale, 0, "gui");
    std::vector<Primitive3DFilter::Primitive3D> guiPrimitives;
    guiPrimitives.push_back(primitive);
    primitiveFilter->apply(guiPrimitives, obj);
    if(showPrimitives.isChecked())
      primitive.toSceneObject(primitive_holder);
  }

  // Filter the point cloud with primitives from RSB
  static CheckBoxHandle filterRSB = gui["filterRSBHandle"];
  if(filterRSB.isChecked()) {
    primitivesMutex.lock();
    primitiveFilter->apply(primitives, obj);
    if(showPrimitives.isChecked()) {
      for(uint i = 0; i < primitives.size(); ++i) {
        primitives[i].toSceneObject(primitive_holder);
      }
    }
    primitivesMutex.unlock();
  }

  primitive_holder->unlock();
  obj.unlock();

  if(pa("-pco")){
    output.send(obj);
  }

  gui["scene"].render();
  gui["fps"].render();
}


 int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-point-cloud-input|-pci(point-cloud-source,descrition) "
                "[m]-config(filename) "
                "-primitivescope(scopename=/nirobots/primitivesetScope) "
                "-point-cloud-output|-pco(point-cloud-destination,descrition) "
                "-view-camera|-c(filename)",init,run).exec();
 }
