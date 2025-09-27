#ifndef _ONLINE_COLLAB_DOCK_H_
#define _ONLINE_COLLAB_DOCK_H_

#include <QDockWidget>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QTcpSocket>

#include <KisViewManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasObserverBase.h>
#include <KoShapeController.h>
#include <kis_canvas2.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_shared_ptr.h>
#include <kis_types.h>
#include <klocalizedstring.h>
#include <kundo2stack.h>

#include "CollabNetwork.h"
#include "ImageChangeAnalyzer.h"

struct ChangeHistory {
    QTime time;
};

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
    void initNetwork(bool isServer);
    void setNetworkButtonState(bool running);
    void setStatusText(QString text);
    void clearStatusText();
    void submitPacket(KisSharedPtr<DataPacket> p);

public Q_SLOTS:
    void nodeChanged(KisNodeSP node);
    void nodePixelChanged(KisNodeSP node, const QRect &rect);

private:
    int lastIndex = 0;
    bool m_isServer;

    KisCanvas2 *m_canvas;
    CollabServer *m_server;
    CollabClient *m_client;
    ImageChangeAnalyzer *m_analyzer;

    QLabel *m_statusText;
    QPushButton *m_startServerBtn;
    QPushButton *m_connectBtn;
    QPushButton *m_stopBtn;
    QLineEdit *m_ipInput;
    QLineEdit *m_portInput;
};

#endif
