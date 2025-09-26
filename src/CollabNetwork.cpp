#include <KoProperties.h>
#include <kis_datamanager.h>
#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "CollabNetwork.h"
#include "NodeState.h"

CollabNetwork::CollabNetwork(KisImage *image)
    : m_image(image)
    , m_socket()
    , m_curPacketBuffer()
{
}

CollabNetwork::~CollabNetwork()
{
    delete m_socket;
}

void CollabNetwork::sendPacket(DataPacket *p)
{
    QByteArray buf;
    QDataStream s(buf);
    p->send(s);
    m_socket->write(buf);
}

void CollabNetwork::receiveBytes()
{
    if (m_curPacketExpectedSize) {
        if (m_socket->bytesAvailable() < sizeof(quint32)) {
            return;
        }

        QDataStream in(m_socket);
        quint32 size;
        in >> size;

        m_curPacketExpectedSize = size;
        m_curPacketBuffer.reserve(size);
    } else if (m_curPacketBuffer.size() != m_curPacketExpectedSize) {
        auto bytesToRead = qMin(m_socket->bytesAvailable(), qint64(m_curPacketExpectedSize - m_curPacketBuffer.size()));
        m_curPacketBuffer.append(m_socket->read(bytesToRead));
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
