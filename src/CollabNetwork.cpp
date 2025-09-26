#include <KoProperties.h>
#include <kis_datamanager.h>
#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "CollabNetwork.h"
#include "NodeState.h"

CollabClient::CollabClient(KisImage *image)
    : m_image(image)
{
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
    qDebug() << "Sending packet";
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        QByteArray buf;
        QDataStream s(buf);
        qDebug() << "Writing blob";
        p->send(s);
        m_socket.write(buf);
    } else {
        qDebug() << "Enqueued for later send";
        m_queue.enqueue(p);
    }
}

void CollabClient::sendQueue()
{
    if (m_socket.state() != QAbstractSocket::ConnectedState) {
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
    if (m_curPacketExpectedSize) {
        if (m_socket.bytesAvailable() < qint64(sizeof(quint32))) {
            return;
        }

        QDataStream in(&m_socket);
        quint32 size;
        in >> size;

        m_curPacketExpectedSize = size;
        m_curPacketBuffer.reserve(size);
    } else if (quint32(m_curPacketBuffer.size()) != m_curPacketExpectedSize) {
        auto bytesToRead = qMin(m_socket.bytesAvailable(), qint64(m_curPacketExpectedSize - m_curPacketBuffer.size()));
        m_curPacketBuffer.append(m_socket.read(bytesToRead));
    } else {
        QDataStream in(m_curPacketBuffer);
        quint8 type;
        in >> type;

        switch (type) {
        case DataPacket::NodeMetadataType: {
            NodeMetadata::apply(in, m_image);
            break;
        }
        case DataPacket::NodePixelPatchType: {
            break;
        }
        default: {
            qDebug() << "Unknown packet type: " << type;
            break;
        }
        }
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

void CollabServer::broadcast(QByteArray data, QTcpSocket *src)
{
    for (auto conn : m_clients) {
        if (conn != src) {
            conn->write(data);
        }
    }
}
