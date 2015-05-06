/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/examples/rsb-dyn-gui.cpp                         **
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

#include <ICLQt/DynamicGUI.h>
#include <ICLQt/RSBRemoteGUI.h>

#include <ICLQt/Common.h>

#include <fstream>

DynamicGUI gui;
RSBRemoteGUI remote;

void init(){
  if(pa("-g")){
    {
      std::ofstream stream( (*pa("-g")).c_str());
      stream << "<?xml version=\"1.0\"?>\n"
             << "<hbox>\n"
             << "  <vbox minsize=\"16x12\" label=\"a box\">\n"
             << "    <button args=\"Push\" minsize=\"10x2\" handle=\"the button\"/>\n"
             << "    <togglebutton args=\"Toggle,Me\" minsize=\"10x2\" handle=\"the t-button\"/>\n"
             << "    <string args=\"test,20\" handle=\"string\"/>\n"
             << "    <int args=\"0,1000,500\" handle=\"int\"/>\n"
             << "    <checkbox args=\"my checkbox,toggled\" handle=\"checkbox\"/>\n"
             << "    <float args=\"-0.2,0.8,0.6\" handle=\"float\" label=\"float\"/>\n"
             << "  </vbox>\n"
             << "  <vsplit>\n"
             << "    <image minsize=\"16x12\" label=\"an image\" handle=\"image\"/>\n"
             << "    <slider args=\"0,255,0\" label=\"the slider\" handle=\"s\"/>\n"
             << "    <hbox>\n"
             << "      <image minsize=\"16x12\" label=\"another image\" handle=\"what\"/>\n"
             << "      <slider args=\"0,255,0\" label=\"yet another slider\"/>\n"
             << "    </hbox>\n"
             << "  </vsplit>\n"
             << "  <hbox margin=\"20\" label=\"box with margin\">\n"
             << "    <image minsize=\"16x12\" label=\"last image then\" handle=\"otherimage\"/>\n"
             << "    <slider args=\"0,255,0\" label=\"slider slider slider\"/>\n"
             << "  </hbox>\n"
             << "</hbox>\n" << std::flush;
    }
    std::cout << "Successfully wrote file to " << *pa("-g") << std::endl;
    std::cout << "[please ignore the following error message]" << std::endl;
    std::terminate();
  }else{
    gui.load(*pa("-l"));
    gui << Show();

    remote.setVerboseMode(pa("-verbose"));    
    remote.init(&gui, *pa("-scope"), pa("-c"));
    
    gui["image"] = create("lena");
  }
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-load-file|-load|-l(filename) "
                "-rsb-scope|-scope|-s(scope=/foo) "
                "-create-setter-gui|-c "
                "-verbose "
                "-generate-gui-xml-file|-g(output-file-name)",init).exec();
}
