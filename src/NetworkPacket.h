#ifndef _NETWORK_PACKET_H_
#define _NETWORK_PACKET_H_

#include <QUuid>

#include <kis_node.h>

#include "CollabNetwork.h"

class PaintLayerMetadata : public DataPacket
{
public:
    PaintLayerMetadata(const KisPaintLayer *node);
    PaintLayerMetadata(QDataStream &s);

    QUuid nodeId;
    quint8 opacity;
    quint8 alphaInherit;
    quint8 locked;
    quint8 visible;
    quint8 alphaLocked;
    QString compositeOp;

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
