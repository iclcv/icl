#ifndef MOUSE_INTERACTION_RECEIVER_H
#define MOUSE_INTERACTION_RECEIVER_H

#include <QObject>
#include "MouseInteractionInfo.h"


namespace icl{
  /// Abstract class for receiving MouseInteractionEvents
  class MouseInteractionReceiver : public QObject{
    Q_OBJECT
    public slots:

    /// when called, processMouseInteraction is called
    void mouseInteraction(MouseInteractionInfo *info);

    /// this function is called if by the mouseInteraction slot 
    /** It can be reimplemented for an individual mouse event handling 
    */
    virtual void processMouseInteraction(MouseInteractionInfo *info);
  };

}

#endif
