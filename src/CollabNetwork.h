#ifndef _COLLAB_NETWORK_H_
#define _COLLAB_NETWORK_H_

#include <QDataStream>
#include <QObject>
#include <QQueue>
#include <QTcpServer>
#include <QTcpSocket>

#include "kis_shared_ptr.h"
#include <kis_node.h>

class DataPacket : public KisShared
{
public:
    static const quint8 NodeMetadataType = 0;
    static const quint8 NodePixelPatchType = 1;

    virtual void send(QDataStream &s);
};

class CollabClient
{
public:
    CollabClient(KisImage *image);
    ~CollabClient();

    void connectTo();
    void queuePacket(KisSharedPtr<DataPacket> p);
    void sendPacket(KisSharedPtr<DataPacket> p);
    void sendQueue();

private:
    KisImage *m_image;
    QTcpSocket m_socket;
    QQueue<KisSharedPtr<DataPacket>> m_queue;

    quint32 m_curPacketExpectedSize;
    QByteArray m_curPacketBuffer;

    void receiveBytes();
};

class CollabServer : public QObject
{
    Q_OBJECT

public:
    CollabServer();
    ~CollabServer();

    bool start(const QHostAddress &addr, quint16 port);
    void broadcast(QByteArray data, QTcpSocket *src);

private:
    QTcpServer m_server;
    QVector<QTcpSocket *> m_clients;

private Q_SLOTS:
    void onNewConnection();
};

#endif
