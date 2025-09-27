#include <KisDocument.h>
#include <image/commands/kis_node_property_list_command.h>
#include <image/commands_new/kis_saved_commands.h>
#include <kis_canvas2.h>
#include <kis_transaction.h>

#include "ImageChangeAnalyzer.h"

ImageChangeAnalyzer::ImageChangeAnalyzer(KisCanvas2 *canvas)
    : m_canvas(canvas)
{
    auto undoStack = canvas->imageView()->document()->undoStack();
    m_lastUndoIndex = undoStack->index();
    resetState();

    connect(undoStack, &KUndo2Stack::indexChanged, this, &ImageChangeAnalyzer::undoIndexChanged);

    connect(canvas->image(), &KisImage::sigImageUpdated, [this](const QRect &rect) {
        if (m_updateRect.isNull()) {
            m_updateRect = rect;
        } else {
            m_updateRect = m_updateRect.united(rect);
        }
    });

    connect(canvas->image(), &KisImage::sigNodeChanged, [this](KisNodeSP node) {
        m_changedNode = node;
    });

    connect(canvas->image(), &KisImage::sigNodeAddedAsync, [this](KisNodeSP node) {
        m_addedNode = node;
    });

    connect(canvas->image(), &KisImage::sigRemoveNodeAsync, [this](KisNodeSP node) {
        m_removedNode = node->uuid();
    });
}

void ImageChangeAnalyzer::resetState()
{
    m_updateRect = QRect();
    m_changedNode = 0;
    m_addedNode = 0;
    m_removedNode = QUuid();
}

void ImageChangeAnalyzer::undoIndexChanged(int index)
{
    auto undoStack = m_canvas->imageView()->document()->undoStack();
    bool isUndo = index < m_lastUndoIndex;
    int curIndex = isUndo ? index : index - 1;
    qDebug() << "Current index: " << curIndex << ", total: " << undoStack->count();
    auto command = undoStack->command(curIndex);
    m_lastUndoIndex = index;

    if (!command) {
        return;
    }

    // KisSavedCommand
    // KisSavedMacroCommand
    // KisNodePropertyListCommand
    qDebug() << "Cur cmd: " << command->text();

    auto cmdSaved = dynamic_cast<const KisSavedCommand *>(command);
    if (cmdSaved) {
        if (isUndo) {
        } else {
            emit sigNodePixelChanged(m_canvas->imageView()->currentNode(), m_updateRect);
            qDebug() << "Captured pixel change on: " << m_canvas->imageView()->currentNode()->name() << m_updateRect;
        }
    }

    auto cmdSavedMacro = dynamic_cast<const KisSavedMacroCommand *>(command);
    if (cmdSavedMacro) {
        if (m_changedNode) {
            if (isUndo) {
            } else {
                emit sigNodeChanged(m_changedNode);
                qDebug() << "Captured property change on: " << m_changedNode->name();
            }
        } else {
            if (isUndo) {
            } else {
                emit sigNodePixelChanged(m_canvas->imageView()->currentNode(), m_updateRect);
                qDebug() << "Captured pixel change on: " << m_canvas->imageView()->currentNode()->name()
                         << m_updateRect;
            }
        }
    }

    auto cmdNodePropList = dynamic_cast<const KisNodePropertyListCommand *>(command);
    if (cmdNodePropList) {
        if (isUndo) {
        } else {
            emit sigNodeChanged(m_changedNode);
            qDebug() << "Captured property change on: " << m_changedNode->name();
            m_changedNodeHistory.insert(command, m_changedNode);
        }
    }

    resetState();
}
