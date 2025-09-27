#include <QIODevice>

#include <KoProperties.h>
#include <kis_datamanager.h>
#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "CollabNetwork.h"
#include "NetworkPacket.h"

CollabClient::CollabClient(QObject *parent, KisImage *image)
    : QObject(parent)
    , m_image(image)
    , m_socket(this)
    , m_socketConnected(false)
    , m_curPacketExpectedSize(0)
{
    connect(&m_socket, &QTcpSocket::connected, [this]() {
        m_socketConnected = true;
    });
    connect(&m_socket, &QTcpSocket::readyRead, this, &CollabClient::receiveBytes);
}

CollabClient::~CollabClient()
{
}

void CollabClient::connectTo(const QHostAddress &addr, quint16 port)
{
    m_socket.connectToHost(addr, port);
}

void CollabClient::sendPacket(KisSharedPtr<DataPacket> p)
{
    qDebug() << "Sending packet, connect state: " << m_socketConnected;
    if (m_socketConnected) {
        m_socket.write(p->toNetworkPacket());
    } else {
        m_queue.enqueue(p);
    }
}

void CollabClient::sendQueue()
{
    if (!m_socketConnected) {
        return;
    }

    qDebug() << "Sending queue with " << m_queue.size() << " elements";
    while (!m_queue.isEmpty()) {
        sendPacket(m_queue.dequeue());
    }
}

QTcpSocket *CollabClient::socket()
{
    return &m_socket;
}

void CollabClient::receiveBytes()
{
    while (m_socket.bytesAvailable()) {
        // qDebug() << "Parsing bytes: " << m_socket.bytesAvailable();

        if (m_curPacketExpectedSize == 0) {
            if (m_socket.bytesAvailable() < qint64(sizeof(quint32))) {
                break;
            }

            QDataStream sizeHeaderStream(m_socket.read(4));
            quint32 size;
            sizeHeaderStream >> size;

            // qDebug() << "Received packet with size of: " << size;

            m_curPacketExpectedSize = size;
            m_curPacketBuffer.reserve(size);
        }

        if (quint32(m_curPacketBuffer.size()) != m_curPacketExpectedSize) {
            auto bytesToRead =
                qMin(m_socket.bytesAvailable(), qint64(m_curPacketExpectedSize - m_curPacketBuffer.size()));
            // qDebug() << "Reading extra bytes to complete packet: " << bytesToRead;
            m_curPacketBuffer.append(m_socket.read(bytesToRead));
        }

        // qDebug() << "Current bytes: " << m_curPacketBuffer.size() << ", expecting " << m_curPacketExpectedSize;

        if (quint32(m_curPacketBuffer.size()) != m_curPacketExpectedSize) {
            break;
        }

        QDataStream in(m_curPacketBuffer);
        quint8 type;
        in >> type;

        DataPacket *p = nullptr;

        switch (type) {
        case DataPacket::NodeMetadataType: {
            qDebug("Received NodeMetadataType");
            p = new PaintLayerMetadata(in);
            break;
        }
        case DataPacket::NodePixelPatchType: {
            qDebug("Received NodePixelPatchType");
            p = new NodePixelPatch(in);
            break;
        }
        case DataPacket::NodeAddition: {
            qDebug("Received NodeAddition");
            p = new NodeAddition(in);
            break;
        }
        // case DataPacket::NodeRemoval: {
        //     qDebug("Received NodeRemoval");
        //     p = new NodeRemoval(in);
        //     break;
        // }
        default: {
            qDebug() << "Unknown packet type: " << type;
            return;
        }
        }

        p->apply(m_image);

        m_curPacketExpectedSize = 0;
        m_curPacketBuffer.clear();
    }
}

CollabServer::~CollabServer()
{
    for (auto conn : m_clients) {
        conn->close();
        delete conn;
    }
}

bool CollabServer::start(const QHostAddress &addr, quint16 port)
{
    if (!m_server.listen(addr, port)) {
        return false;
    }

    connect(&m_server, &QTcpServer::newConnection, this, &CollabServer::onNewConnection);
    return true;
}

void CollabServer::onNewConnection()
{
    auto conn = m_server.nextPendingConnection();
    m_clients.push_back(conn);
    connect(conn, &QTcpSocket::readyRead, this, [conn, this]() {
        this->broadcast(conn->readAll(), conn);
    });
}

void CollabServer::broadcast(QByteArray data, QTcpSocket *src = nullptr)
{
    // qDebug() << "Broadcasting data with size: " << data.size();

    for (auto conn : m_clients) {
        if (conn != src) {
            conn->write(data);
        }
    }
}
