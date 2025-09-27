#ifndef _STREAM_UTILS_H_
#define _STREAM_UTILS_H_

#include <QDataStream>
#include <QString>

namespace StreamUtils
{
void writeStringToDataStream(const QString &s, QDataStream &st);
QString readStringFromDataStream(QDataStream &st);
} // namespace StreamUtils

#endif
