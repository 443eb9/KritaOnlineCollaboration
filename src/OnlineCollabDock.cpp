#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <KoDocumentResourceManager.h>
#include <KoToolProxy.h>
#include <kis_command_utils.h>
#include <kis_config.h>
#include <kis_icon_utils.h>
#include <kis_transaction.h>

#include "CollabNetwork.h"
#include "NodeState.h"
#include "OnlineCollabDock.h"

OnlineCollabDock::OnlineCollabDock()
    : QDockWidget()
    , m_network(0)
{
    setWindowTitle(i18n("Online Collab"));
    auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    auto connectParams = new QVBoxLayout();
    m_ipInput = new QLineEdit();
    m_portInput = new QLineEdit();
    auto netBtns = new QHBoxLayout();
    auto startServerBtn = new QPushButton("Start Server");
    auto connectBtn = new QPushButton("Connet");
    netBtns->addWidget(startServerBtn);
    netBtns->addWidget(connectBtn);
    connectParams->addWidget(m_ipInput);
    connectParams->addWidget(m_portInput);
    connectParams->addLayout(netBtns);
    mainLayout->addLayout(connectParams);
}

void OnlineCollabDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    QPointer<KisCanvas2> myCanvas = dynamic_cast<KisCanvas2 *>(canvas);
    if (m_canvas == myCanvas || !myCanvas) {
        return;
    }
    m_canvas = myCanvas;

    auto image = myCanvas->image();
    auto root = image->root();

    connect(image.data(), &KisImage::sigNodeChanged, this, &OnlineCollabDock::nodeChanged);
    connect(image.data(), &KisImage::sigImageUpdated, this, &OnlineCollabDock::imageUpdated);
    qDebug() << "Current image: " << image->root()->name();
}

void OnlineCollabDock::unsetCanvas()
{
    setEnabled(false);
}

void OnlineCollabDock::nodeChanged(KisNodeSP node)
{
    qDebug() << "Node changed: " << node->name();
    NodeMetadata n(node.data());
    m_network.sendPacket(&n);
}

void OnlineCollabDock::imageUpdated(const QRect &rect)
{
    qDebug() << "Image updated: " << rect;
    auto image = m_canvas->image();
    auto view = m_canvas->imageView();
    auto node = view->currentNode();
    qDebug() << "Current node: " << node->name();
    NodePixelPatch n(node.data(), rect);
    m_network.sendPacket(&n);
}
