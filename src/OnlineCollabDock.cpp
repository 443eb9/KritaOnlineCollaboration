#include "OnlineCollabDock.h"

#include <KoDocumentResourceManager.h>
#include <kis_config.h>
#include <kis_icon_utils.h>

#include <QDebug>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QVBoxLayout>

OnlineCollabDock::OnlineCollabDock()
    : QDockWidget()
{
    setWindowTitle(i18n("Online Collab"));
}

void OnlineCollabDock::setCanvas(KoCanvasBase *canvas)
{
    setEnabled(canvas != 0);
    QPointer<KisCanvas2> myCanvas = dynamic_cast<KisCanvas2 *>(canvas);
    if (myCanvas && myCanvas->shapeController() && myCanvas->shapeController()->resourceManager()
        && myCanvas->shapeController()->resourceManager()->undoStack()) {
        m_undoStack = myCanvas->shapeController()->resourceManager()->undoStack();
    }
}

void OnlineCollabDock::unsetCanvas()
{
    setEnabled(false);
    m_undoStack = 0;
}
