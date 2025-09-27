#include "NodeUtils.h"

namespace LayerType
{
quint32 resolveNodeLayerType(const KisNode *node)
{
    if (node->inherits("KisPaintLayer")) {
        return LayerType::PaintLayer;
    }

    return 0xffffffff;
}
} // namespace LayerType
