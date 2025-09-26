#include <kis_paint_device.h>

#include "NodeState.h"

NodeMetadata::NodeMetadata(QDataStream &s)
{
    s >> opacity;
    int propsLen;
    s >> propsLen;

    QVector<QPair<QString, QVariant>> props(propsLen);
    for (int _ = 0; _ < propsLen; _++) {
        int nameLen;
        s >> nameLen;
        char nameBytes[nameLen];
        s.readRawData(nameBytes, nameLen);
        auto name = QString::fromUtf8(nameBytes, nameLen);

        quint32 valueType;
        s >> valueType;
        QVariant value(valueType);
        switch (valueType) {
        case QVariant::String: {
            int valueLen;
            s >> valueLen;
            char valueBytes[valueLen];
            s.readRawData(valueBytes, valueLen);
            value.setValue(QString::fromUtf8(valueBytes, valueLen));
            break;
        }
        case QVariant::Bool: {
            quint8 b;
            s >> b;
            value.setValue(b);
        }
        default: {
            break;
        }
        }

        props.push_back(qMakePair(name, value));
    }
}

NodeMetadata::NodeMetadata(const KisNode *node)
    : opacity(node->opacity())
{
    auto props = node->sectionModelProperties();
    this->props = QVector<QPair<QString, QVariant>>(props.size());
    for (int i = 0; i < props.size(); i++) {
        this->props[i] = qMakePair(props[i].id, props[i].state);
    }
}

void NodeMetadata::send(QDataStream &out)
{
    out << quint8(opacity);

    out << int(props.size());
    for (const auto &[name, value] : props) {
        auto nameBytes = name.toUtf8();
        out << int(nameBytes.size());
        out << nameBytes;

        out << quint32(value.type());
        switch (value.type()) {
        case QVariant::String: {
            auto strBytes = value.toString().toUtf8();
            out << strBytes.size();
            out << strBytes;
            break;
        }
        case QVariant::Bool: {
            auto b = value.toBool();
            out << quint8(b);
        }
        default: {
            break;
        }
        }
    }
}

void NodeMetadata::apply(QDataStream &s, KisImage *image)
{
    // TODO
}

NodePixelPatch::NodePixelPatch(QDataStream &s)
{
    s >> rect;
    int size;
    s >> size;
    data.reserve(size);
    s.readRawData(data.data(), size);
}

NodePixelPatch::NodePixelPatch(const KisNode *node, const QRect &rect)
    : rect(rect)
{
    auto device = node->paintDevice();
    auto pixelSize = device->pixelSize();
    data.reserve(pixelSize * rect.width() * rect.height());
    device->readBytes(reinterpret_cast<quint8 *>(data.data_ptr()), rect);
}

void NodePixelPatch::send(QDataStream &out)
{
    out << rect;
    out << int(data.size());
    out << data;
}

void NodePixelPatch::apply(QDataStream &s, KisImage *image)
{
    // TODO
}
