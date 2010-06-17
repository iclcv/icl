/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/memory-listener.cpp                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/ProgArg.h>
#include <ICLIO/XCFMemoryListener.h>

using namespace icl;

int main(int n, char **ppc){
  paex
  ("-m","define memory name (xcf:ShortTerm at default)")
  ("-x","define xpath to listen on (if not given, all memory events are printed)")
  ("-e","define event types to listen on. this is a | separated list of REPLACE|INSERT and REMOVE. (At default all event types are used)")
  ("-t","if this flag is set, a timestamp is printed each time when a document matched the given xpath")
  ("-n","if this flag is set, documents are printed without the pretty flag")
  ("-p","print only matching sub locations");
  painit(n,ppc,"-memory|-m(memory-name=xcf:wb) "
         "-xpath|-x(xpath=/) "
         "-events|-e(string=REPLACE|INSERT|REMOVE) "
         "-print-time-stamps|-t "
         "-print-sub-locations-only|-p");
  
  XCFMemoryListener l(*pa("-m"),*pa("-x"),*pa("-e"));
  
  l.setPrintPretty(!pa("-n"));
  l.setPrintTimeStamps(pa("-t"));
  l.setPrintSubLocationsOnly(pa("-p"));
  
  l.run(); 
}


