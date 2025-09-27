#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QMetaEnum>
#include <QRegExpValidator>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <KisDocument.h>
#include <kis_command_utils.h>
#include <kis_config.h>
#include <kis_icon_utils.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>

#include "CollabNetwork.h"
#include "NetworkPacket.h"
#include "OnlineCollabDock.h"

OnlineCollabDock::OnlineCollabDock()
    : m_server(0)
    , m_client(0)
    , m_analyzer(0)
{
    setWindowTitle(i18n("Online Collab"));
    auto mainWidget = new QWidget(this);
    setWidget(mainWidget);
    auto mainLayout = new QVBoxLayout(mainWidget);

    m_statusText = new QLabel();
    m_statusText->setWordWrap(true);

    auto connectParams = new QFormLayout();
    m_ipInput = new QLineEdit();
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    auto ipVal =
        new QRegExpValidator(QRegExp("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$"));
    m_ipInput->setValidator(ipVal);
    m_portInput = new QLineEdit();
    auto portVal = new QIntValidator(0, 65535);
    m_portInput->setValidator(portVal);
    connectParams->addRow("IP", m_ipInput);
    connectParams->addRow("Port", m_portInput);

    auto netBtns = new QHBoxLayout();
    m_startServerBtn = new QPushButton("Start Server");
    m_connectBtn = new QPushButton("Connect");
    m_stopBtn = new QPushButton("Stop");
    m_stopBtn->setEnabled(false);
    netBtns->addWidget(m_startServerBtn);
    netBtns->addWidget(m_connectBtn);
    netBtns->addWidget(m_stopBtn);

    mainLayout->addWidget(m_statusText);
    mainLayout->addLayout(connectParams);
    mainLayout->addLayout(netBtns);

    // TODO read from settings
    m_ipInput->setText("127.0.0.1");
    m_portInput->setText("45304");

    connect(m_startServerBtn, &QPushButton::clicked, [this]() {
        clearStatusText();
        if (m_ipInput->text().isEmpty() || m_portInput->text().isEmpty()) {
            return;
        }

        initNetwork(true);
        auto ipText = m_ipInput->text();
        auto portText = m_portInput->text();
        auto ip = QHostAddress(ipText);

        if (ip.isNull()) {
            setStatusText(QString("Invalid ip address: %1").arg(ipText));
            return;
        }

        bool success = m_server->start(ip, portText.toUShort());

        if (success) {
            setStatusText(QString("Successfully started server on address %1 with port %2").arg(ipText).arg(portText));
            setNetworkButtonState(true);
        } else {
            setStatusText(QString("Unable to server on address %1 with port %2").arg(ipText).arg(portText));
            m_stopBtn->setEnabled(true);
        }
    });

    connect(m_connectBtn, &QPushButton::clicked, [this]() {
        setNetworkButtonState(true);

        initNetwork(false);
        auto ipText = m_ipInput->text();
        auto portText = m_portInput->text();
        auto ip = QHostAddress(m_ipInput->text());

        if (ip.isNull()) {
            setStatusText(QString("Invalid ip address: %1").arg(ipText));
            return;
        }

        setStatusText(QString("Connecting to %1 with port %2").arg(ipText).arg(portText));
        m_client->connectTo(ip, m_portInput->text().toUShort());
        connect(m_client->socket(), &QTcpSocket::errorOccurred, [this](QAbstractSocket::SocketError error) {
            auto me = QMetaEnum::fromType<QAbstractSocket::SocketError>();
            setStatusText(QString("Unable to connect to server on address %1 with port %2: %3")
                              .arg(m_ipInput->text())
                              .arg(m_portInput->text())
                              .arg(me.valueToKey(error)));
            setNetworkButtonState(false);
        });
        connect(m_client->socket(), &QTcpSocket::connected, [this]() {
            setStatusText(QString("Successfully connected to server on address %1 with port %2")
                              .arg(m_ipInput->text())
                              .arg(m_portInput->text()));
        });
    });

    connect(m_stopBtn, &QPushButton::clicked, [this]() {
        clearStatusText();
        setNetworkButtonState(false);

        if (m_server) {
            delete m_server;
        }
        if (m_client) {
            delete m_client;
        }

        m_server = 0;
        m_client = 0;
    });
}

void OnlineCollabDock::setStatusText(QString text)
{
    m_statusText->setText(text);
}

void OnlineCollabDock::clearStatusText()
{
    m_statusText->setText(QString());
}

void OnlineCollabDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    QPointer<KisCanvas2> myCanvas = dynamic_cast<KisCanvas2 *>(canvas);
    if (m_canvas == myCanvas || !myCanvas) {
        return;
    }
    m_canvas = myCanvas;
    if (m_analyzer)
        delete m_analyzer;
    m_analyzer = new ImageChangeAnalyzer(m_canvas);

    connect(m_analyzer, &ImageChangeAnalyzer::sigNodeChanged, this, &OnlineCollabDock::nodeChanged);
    connect(m_analyzer, &ImageChangeAnalyzer::sigNodePixelChanged, this, &OnlineCollabDock::nodePixelChanged);
}

void OnlineCollabDock::unsetCanvas()
{
    setEnabled(false);
}

void OnlineCollabDock::initNetwork(bool isServer)
{
    m_isServer = isServer;
    if (isServer) {
        m_server = new CollabServer();
    } else {
        m_client = new CollabClient(this, m_canvas->image().data());
    }
}

void OnlineCollabDock::setNetworkButtonState(bool running)
{
    m_startServerBtn->setEnabled(!running);
    m_connectBtn->setEnabled(!running);
    m_stopBtn->setEnabled(running);
    m_ipInput->setEnabled(!running);
    m_portInput->setEnabled(!running);
}

void OnlineCollabDock::submitPacket(KisSharedPtr<DataPacket> p)
{
    if (m_isServer && m_server) {
        m_server->broadcast(p->toNetworkPacket(), nullptr);
    } else if (m_client) {
        m_client->sendPacket(p);
    }
}

void OnlineCollabDock::nodeChanged(KisNodeSP node)
{
    qDebug() << "Node changed: " << node->name();
    KisSharedPtr<DataPacket> p = 0;

    if (node->inherits("KisPaintLayer")) {
        p = new PaintLayerMetadata(static_cast<KisPaintLayer *>(node.data()));
    }

    if (!p) {
        // TODO notify
        return;
    }

    submitPacket(p);
}

void OnlineCollabDock::nodePixelChanged(KisNodeSP node, const QRect &rect)
{
    if (rect.width() == 0 || rect.height() == 0) {
        return;
    }

    submitPacket(new NodePixelPatch(node.data(), rect));
}
