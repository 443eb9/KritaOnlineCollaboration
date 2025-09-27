#include <KoProperties.h>
#include <kis_datamanager.h>
#include <kis_image.h>
#include <kis_layer_utils.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "NetworkPacket.h"
#include "StreamUtils.h"

PaintLayerMetadata::PaintLayerMetadata(const KisPaintLayer *node)
    : nodeId(node->uuid())
    , opacity(node->opacity())
    , alphaInherit(node->alphaChannelDisabled() ? 1 : 0)
    , locked(node->userLocked() ? 1 : 0)
    , visible(node->visible() ? 1 : 0)
    , alphaLocked(node->alphaLocked() ? 1 : 0)
    , compositeOp(node->compositeOpId())
{
}

void PaintLayerMetadata::send(QDataStream &out)
{
    out << nodeId << opacity << alphaInherit << locked << visible << alphaLocked;
    StreamUtils::writeStringToDataStream(compositeOp, out);
}

PaintLayerMetadata::PaintLayerMetadata(QDataStream &s)
{
    s >> nodeId >> opacity >> alphaInherit >> locked >> visible >> alphaLocked;
    compositeOp = StreamUtils::readStringFromDataStream(s);
}

void PaintLayerMetadata::apply(KisImage *image)
{
    auto node = KisLayerUtils::findNodeByUuid(image->root(), nodeId);
    if (node == 0) {
        return;
    }
    auto layer = static_cast<KisPaintLayer *>(node.data());
    layer->setOpacity(opacity);
    layer->disableAlphaChannel(alphaInherit);
    layer->setUserLocked(locked);
    layer->setVisible(visible);
    layer->setAlphaLocked(alphaLocked);
    layer->setCompositeOpId(compositeOp);
    layer->setDirty();
}

NodePixelPatch::NodePixelPatch(const KisNode *node, const QRect &rect)
    : rect(rect)
    , nodeId(node->uuid())
{
    auto device = node->paintDevice();
    // auto pixelSize = device->pixelSize();
    auto pixelSize = device->dataManager()->pixelSize();
    data.resize(pixelSize * rect.width() * rect.height());
    qDebug() << "Created patch " << data.size();

    device->readBytes(data.data(), rect);
}

NodePixelPatch::NodePixelPatch(QDataStream &s)
{
    s >> nodeId;
    s >> rect;
    quint32 size;
    s >> size;
    data.resize(size);
    s.readRawData(reinterpret_cast<char *>(data.data()), size);
}

void NodePixelPatch::send(QDataStream &out)
{
    out << nodeId;
    out << rect;
    out << quint32(data.size());
    qDebug() << "Writing " << data.size();
    out.writeRawData(reinterpret_cast<char *>(data.data()), data.size());
}

void NodePixelPatch::apply(KisImage *image)
{
    auto node = KisLayerUtils::findNodeByUuid(image->root(), nodeId);
    if (node == 0) {
        qDebug() << "Node not found";
        return;
    }
    node->paintDevice()->writeBytes(data.data(), rect);
    node->paintDevice()->setDirty(rect);
}
