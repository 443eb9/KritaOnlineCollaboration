#ifndef _NETWORK_PACKET_H_
#define _NETWORK_PACKET_H_

#include <QUuid>

#include <kis_node.h>

#include "CollabNetwork.h"

class NodeMetadata : public DataPacket
{
public:
    NodeMetadata(const KisNode *node);
    NodeMetadata(QDataStream &s);

    QUuid nodeId;
    quint8 opacity;
    QVector<QPair<QString, QVariant>> props;

    virtual quint8 packetType()
    {
        return DataPacket::NodeMetadataType;
    }

    void send(QDataStream &s) override;
    void apply(KisImage *image) override;
};

class NodePixelPatch : public DataPacket
{
public:
    NodePixelPatch(const KisNode *node, const QRect &rect);
    NodePixelPatch(QDataStream &s);

    QUuid nodeId;
    QRect rect;
    QByteArray data;

    virtual quint8 packetType()
    {
        return DataPacket::NodePixelPatchType;
    }

    void send(QDataStream &s) override;
    void apply(KisImage *image) override;
};

#endif
