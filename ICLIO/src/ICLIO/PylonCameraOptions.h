// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#pragma once

#include <ICLIO/PylonIncludes.h>

#include <ICLUtils/Macros.h>
#include <ICLIO/PylonUtils.h>
#include <ICLUtils/Configurable.h>

namespace icl::io {
    namespace pylon {

      /// This is a helper class for Pylon camera settings \ingroup GIGE_G
      class ICLIO_API PylonCameraOptions : public utils::Configurable{
        public:
          /// Constructor
          /**
          * @param camera The IPylonDevice to work with.
          * @param interu An Interruptable class that provides interruption
          * functionality for the passed camera.
          */
          PylonCameraOptions(Pylon::IPylonDevice* camera, Interruptable* interu);

          /// Destructor
          ~PylonCameraOptions();

          /// convenience function to get the cameras PixelType
          Pylon::PixelType getCameraPixelType();
          /// getter for cameras PixelSize in bits
          int getCameraPixelSize();
          /// getter of the BufferSize needed by the camera.
          long getNeededBufferSize();
          /// Executes the ICommand 'AcquisitionStart'
          void acquisitionStart();
          /// Executes the ICommand 'AcquisitionStop'
          void acquisitionStop();
          /// getter for the camera image height.
          int getHeight();
          /// getter for the camera image width.
          int getWidth();
          /// returns the cameras PixelFormat as string
          std::string getFormatString();
          /// whether double frames should be omitted.
          bool omitDoubleFrames();
          /// getter for the current expected framerate
          double getResultingFrameRateAbs();

        private:
          /// the Interruptable that provides interruption for the camera.
          Interruptable* m_Interu;
          /// The camera
          Pylon::IPylonDevice* m_Camera;
          /// whether double frames should be omitted.
          bool m_OmitDoubleFrames;

          /// get type of property.
          std::string getType(const std::string &name);
          /// get information of a properties valid values.
          std::string getInfo(const std::string &name);
          /// returns the current value of a property or a parameter.
          std::string getValue(const std::string &name);
          /// Returns whether this property may be changed internally.
          int isVolatile(const std::string &propertyName);
          /// callback for changed configurable properties
          void processPropertyChange(const utils::Configurable::Property &prop);
          /// helper function for getPropertyList.
          void addToPropertyList(std::vector<std::string> &ps, const GenApi::CNodePtr& node);

          /// gets the corresponding CValuePtr to the passed name.
          GenApi::INode *getNode(std::string name);
      };

    } // namespace pylon

    // here come some convenience functions for pylon namespace
    namespace pylon {

      /// template function to get the value of an IValue-subclass
      template <typename NODE, typename RET>
      RET getNodeValue(NODE* node){
        return node -> GetValue();
      }

      /// template function overload to get the int64_t-value of an IEnumeration
      template <typename NODE, typename RET>
      int64_t getNodeValue(GenApi::IEnumeration* node){
        return node -> GetIntValue();
      }

      /// template function to set the value of an IValue-subclass
      template <typename NODE, typename VAL>
      void setNodeValue(NODE* node, VAL value){
        node -> SetValue(value, true);
        return;
      }

      /// template function overload to set the value of an IEnumeration
      template <typename NODE, typename VAL>
      void setNodeValue(GenApi::IEnumeration* node, std::string value){
        node -> FromString(value.c_str(), true);
        return;
      }

      /// template function overload to set the value of an IEnumeration
      template <typename NODE, typename VAL>
      void setNodeValue(GenApi::IEnumeration* node, int64_t value){
        node -> SetIntValue(value, true);
        return;
      }

      /// set the value of a parameter of a specific type on a specific source (camera/grabber)
      template <typename OBJ, typename NODE, typename VAL>
      bool setParameterValueOf(OBJ* object, std::string parameter, VAL value){
        GenApi::INode* node = object -> GetNodeMap() -> GetNode(parameter.c_str());
        if (!node) {
          DEBUG_LOG("There is no parameter called '" << parameter << "'")
              return false;
        }
        //dynamic cast to needed node-type
        NODE* node2;
        try{
          node2 = dynamic_cast<NODE*>(node);
        } catch (std::exception &e){
          DEBUG_LOG ("Could not cast node '"<< parameter << "' to desired type2")
              return false;
        }
        if(!GenApi::IsWritable(node2)){
          DEBUG_LOG("Parameter called '" << parameter << "' is not writable.")
              return false;
        }
        // now setting parameter
        try{
          setNodeValue<NODE, VAL>(node2, value);
          return true;
        }
        catch (GenICam::GenericException &e) {
          std::cerr << e.what() << std::endl;
          return false;
        }
      }

      /// get the value of a parameter of a specific type from a spec. source (camera/grabber)
      template <typename SOURCE, typename NODE, typename RET>
      RET getParameterValueOf(SOURCE* source, std::string param){
        GenApi::INode* node = source -> GetNodeMap() -> GetNode(param.c_str());
        if(!node){
          throw utils::ICLException("parameter of this type not found");
        }
        //dynamic cast to needed node-type
        try{
          NODE* node2 = dynamic_cast<NODE*>(node);
          if(!node2){
            throw utils::ICLException("Could not cast " + param + " to desired type");
          }
          if(!GenApi::IsReadable(node2)){
            throw utils::ICLException("The node " + param + " is not Readable");
          }
          return getNodeValue<NODE, RET>(node2);
        } catch (std::exception &e){
          throw utils::ICLException(e.what());
        }
      }

      /// returns a string representation of the value of a parameter of the camera.
      ICLIO_API std::string getParameterValueString(
          Pylon::IPylonDevice* device, std::string parameter);

    } //namespace pylon
  } // namespace icl::io