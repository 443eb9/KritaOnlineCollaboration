#ifndef _COLLAB_NETWORK_H_
#define _COLLAB_NETWORK_H_

#include <QTcpSocket>
#include <QDataStream>

#include <kis_node.h>

class DataPacket
{
public:
    static const quint8 NodeMetadataType = 0;
    static const quint8 NodePixelPatchType = 1;

    virtual void send(QDataStream &s);
};

class CollabNetworkConfig
{
public:
    // TODO
    // int debounceMs;
};

class CollabNetwork
{
public:
    CollabNetwork(KisImage *image);
    ~CollabNetwork();

    CollabNetworkConfig m_config;

    void sendPacket(DataPacket *p);

private:
    KisImage *m_image;
    QTcpSocket *m_socket;

    quint32 m_curPacketExpectedSize;
    QByteArray m_curPacketBuffer;

    void receiveBytes();
};

#endif
