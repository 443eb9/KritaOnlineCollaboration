#include "StreamUtils.h"

namespace StreamUtils
{
void writeStringToDataStream(const QString &s, QDataStream &st)
{
    st << quint32(s.size());
    auto buf = s.toUtf8();
    st.writeRawData(buf.data(), buf.size());
}

QString readStringFromDataStream(QDataStream &st)
{
    quint32 len;
    st >> len;
    QVector<char> buf(len);
    st.readRawData(buf.data(), len);
    return QString::fromUtf8(buf.data(), len);
}
} // namespace StreamUtils
