/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_CAM_THREAD_H
#define ICL_CAM_THREAD_H

#include <ICLQt/Widget.h>
#include <ICLCore/ImgBase.h>
#include <ICLIO/PWCGrabber.h>

using namespace icl;
using namespace std;
namespace icl{

  /// A utility class ( DEPRECATED! ) \ingroup COMMON 
  class CamThread : public QObject{
    Q_OBJECT
    public:
    inline CamThread(int id, const Size &size=Size(320,240)):
    widget(new ICLWidget(0)),
    grabber(new PWCGrabber(size,30,id)),
    id(id){
      //grabber->setDesiredSize(size);
      widget->setGeometry(10,10,size.width,size.height);
      widget->show();
    } 
    inline CamThread(Grabber *grabber, const Size &size=Size(320,240)):
    widget(new ICLWidget(0)),
    grabber(grabber),
    id(-1){
      //grabber->setDesiredSize(size);
      widget->setGeometry(10,10,size.width,size.height);
      widget->show();
    }
    inline virtual ~CamThread(){
      delete grabber;
      delete widget;
    }
    
    public slots:
    
    void update(){
      widget->setImage(grabber->grab());
      widget->update();
    }

    void setGeomery(const Rect &bounds){
      widget->setGeometry(QRect(bounds.x, bounds.y, bounds.width, bounds.height));
    }
    
    private:
    ICLWidget *widget;
    Grabber *grabber;
    int id;
  };
}


#endif

