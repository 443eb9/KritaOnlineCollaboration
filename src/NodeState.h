#ifndef _NODE_STATE_H_
#define _NODE_STATE_H_

#include <kis_node.h>

#include "CollabNetwork.h"

class NodeMetadata : public DataPacket
{
public:
    NodeMetadata(const KisNode *node);
    NodeMetadata(QDataStream &s);

    quint8 opacity;
    QVector<QPair<QString, QVariant>> props;

    virtual quint8 packetType()
    {
        return DataPacket::NodeMetadataType;
    }

    void send(QDataStream &s) override;
    void static apply(QDataStream &s, KisImage *image);
};

class NodePixelPatch : public DataPacket
{
public:
    NodePixelPatch(const KisNode *node, const QRect &rect);
    NodePixelPatch(QDataStream &s);

    QRect rect;
    QByteArray data;

    virtual quint8 packetType()
    {
        return DataPacket::NodePixelPatchType;
    }

    void send(QDataStream &s) override;
    void static apply(QDataStream &s, KisImage *image);
};

#endif
