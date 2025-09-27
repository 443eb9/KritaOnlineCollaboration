#ifndef _NODE_UTILS_H_
#define _NODE_UTILS_H_

#include <qnumeric.h>

#include <kis_node.h>

namespace LayerType
{
static const quint32 PaintLayer = 0;

quint32 resolveNodeLayerType(const KisNode *node);
} // namespace LayerType

#endif
