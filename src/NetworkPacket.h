#ifndef _NETWORK_PACKET_H_
#define _NETWORK_PACKET_H_

#include <QUuid>

#include <kis_node.h>

#include "CollabNetwork.h"

class NodeAddition : public DataPacket
{
public:
    NodeAddition(const KisNode *node, quint32 type);
    NodeAddition(const KisPaintLayer *node);
    NodeAddition(QDataStream &s);

    QUuid nodeId;
    quint32 actualType;
    QUuid parentId;
    QString name;
    quint32 indexInChildren;
    QVector<quint8> possibleData;

    quint8 packetType() override
    {
        return DataPacket::NodeAddition;
    }

    void send(QDataStream &s) override;
    void apply(KisImage *image) override;
};

class NodeRemoval : public DataPacket
{
public:
    NodeRemoval(const QUuid &id);
    NodeRemoval(QDataStream &s);

    QUuid nodeId;

    quint8 packetType() override
    {
        return DataPacket::NodeRemoval;
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
    QVector<quint8> data;

    quint8 packetType() override
    {
        return DataPacket::NodePixelPatchType;
    }

    void send(QDataStream &s) override;
    void apply(KisImage *image) override;
};

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

    quint8 packetType() override
    {
        return DataPacket::NodeMetadataType;
    }

    void send(QDataStream &s) override;
    void apply(KisImage *image) override;
};

#endif
