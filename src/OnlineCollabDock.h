#ifndef _ONLINE_COLLAB_DOCK_H_
#define _ONLINE_COLLAB_DOCK_H_

#include <QDockWidget>
#include <QObject>

#include <KoCanvasObserverBase.h>
#include <klocalizedstring.h>
#include <kundo2stack.h>
#include "KisViewManager.h"
#include "kis_canvas2.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_types.h"
#include <KoCanvasBase.h>
#include <KoShapeController.h>

#include "CollabNetwork.h"

class OnlineCollabDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    OnlineCollabDock();
    QString observerName() override
    {
        return "OnlineCollabDock";
    }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void nodeChanged(KisNodeSP node);
    void imageUpdated(const QRect &rect);

private:
    int lastIndex = 0;
    KisCanvas2 *m_canvas;
    CollabNetwork m_network;
};

#endif
