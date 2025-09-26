#include <kis_paint_device.h>

#include "NetworkPacket.h"

NodeMetadata::NodeMetadata(const KisNode *node)
    : opacity(node->opacity())
    , nodeId(node->uuid())
{
    auto props = node->sectionModelProperties();
    this->props = QVector<QPair<QString, QVariant>>(props.size());
    for (int i = 0; i < props.size(); i++) {
        this->props[i] = qMakePair(props[i].id, props[i].state);
    }
}

void NodeMetadata::send(QDataStream &out)
{
    out << nodeId;
    out << quint8(opacity);

    out << quint32(props.size());
    for (const auto &[name, value] : props) {
        QByteArray nameBytes = name.toUtf8();
        out << quint32(nameBytes.size());
        out.writeRawData(nameBytes.constData(), nameBytes.size());

        out << quint32(value.type());
        switch (value.type()) {
        case QVariant::String: {
            QByteArray strBytes = value.toString().toUtf8();
            out << quint32(strBytes.size());
            out.writeRawData(strBytes.constData(), strBytes.size());
            break;
        }
        case QVariant::Bool: {
            out << quint8(value.toBool() ? 1 : 0);
            break;
        }
        default:
            break;
        }
    }
}

NodeMetadata::NodeMetadata(QDataStream &s)
{
    s >> nodeId;
    s >> opacity;
    quint32 propsLen;
    s >> propsLen;

    qDebug() << "Props len:" << propsLen;

    props.reserve(propsLen);
    for (quint32 i = 0; i < propsLen; i++) {
        quint32 nameLen;
        s >> nameLen;
        QByteArray nameBytes(nameLen, Qt::Uninitialized);
        s.readRawData(nameBytes.data(), nameLen);
        QString name = QString::fromUtf8(nameBytes);

        qDebug() << "Name len:" << nameLen << ", Name:" << name;

        quint32 valueType;
        s >> valueType;

        QVariant value;
        switch (valueType) {
        case QVariant::String: {
            quint32 valueLen;
            s >> valueLen;
            QByteArray valueBytes(valueLen, Qt::Uninitialized);
            s.readRawData(valueBytes.data(), valueLen);
            value = QString::fromUtf8(valueBytes);
            break;
        }
        case QVariant::Bool: {
            quint8 b;
            s >> b;
            value = QVariant(bool(b));
            break;
        }
        default:
            qWarning() << "Unknown value type:" << valueType;
            break;
        }

        props.push_back(qMakePair(name, value));
    }
}

void NodeMetadata::apply(KisImage *image)
{
    qDebug() << opacity << endl << props;
    // TODO
}

NodePixelPatch::NodePixelPatch(const KisNode *node, const QRect &rect)
    : rect(rect)
    , nodeId(node->uuid())
{
    auto device = node->paintDevice();
    auto pixelSize = device->pixelSize();
    data.reserve(pixelSize * rect.width() * rect.height());
    device->readBytes(reinterpret_cast<quint8 *>(data.data_ptr()), rect);
}

NodePixelPatch::NodePixelPatch(QDataStream &s)
{
    s >> nodeId;
    s >> rect;
    int size;
    s >> size;
    data.reserve(size);
    s.readRawData(data.data(), size);
}

void NodePixelPatch::send(QDataStream &out)
{
    out << nodeId;
    out << rect;
    out << int(data.size());
    out << data;
}

void NodePixelPatch::apply(KisImage *image)
{
    // TODO
}
