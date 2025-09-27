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
    auto curNode = m_canvas->imageView()->currentNode();
    qDebug() << "Cur cmd: " << command->text() << ", is undo: " << isUndo;

    auto cmdSaved = dynamic_cast<const KisSavedCommand *>(command);
    if (cmdSaved) {
        if (isUndo) {
            const auto [node, rect] = m_updateRectHistory[command];
            // TODO check if the node still valid
            qDebug() << "Reverting pixel change on: " << node->name() << rect;
            emit sigNodePixelChanged(node, rect);
        } else {
            emit sigNodePixelChanged(curNode, m_updateRect);
            qDebug() << "Captured pixel change on: " << curNode->name() << m_updateRect;
            m_updateRectHistory.insert(command, qMakePair(curNode, m_updateRect));
        }
    }

    auto cmdSavedMacro = dynamic_cast<const KisSavedMacroCommand *>(command);
    if (cmdSavedMacro) {
        if (isUndo) {
            if (auto changed = m_changedNodeHistory.find(command); changed != m_changedNodeHistory.end()) {
                auto node = changed.value();
                // TODO check if the node still valid
                emit sigNodeChanged(changed.value());
                qDebug() << "Reverting property change on: " << changed.value()->name();
            } else if (auto added = m_addedNodeHistory.find(command); added != m_addedNodeHistory.end()) {
                auto node = added.value();
                emit sigNodeRemoved(node);
                qDebug() << "Reverting node added: " << node;
            } else if (auto removed = m_removedNodeHistory.find(command); removed != m_removedNodeHistory.end()) {
                auto node = removed.value();
                emit sigNodeAdded(m_removedNode);
                qDebug() << "Reverting node removed: " << node;
            } else {
                const auto [node, rect] = m_updateRectHistory[command];
                // TODO check if the node still valid
                qDebug() << "Reverting pixel change on: " << node->name() << rect;
                emit sigNodePixelChanged(node, rect);
            }
        } else {
            if (m_changedNode) {
                emit sigNodeChanged(m_changedNode);
                m_changedNodeHistory.insert(command, m_changedNode);
                qDebug() << "Captured property change on: " << m_changedNode->name();
            } else if (m_addedNode) {
                emit sigNodeAdded(m_addedNode->uuid());
                m_addedNodeHistory.insert(command, m_addedNode->uuid());
                qDebug() << "Node added: " << m_addedNode->name();
            } else if (!m_removedNode.isNull()) {
                emit sigNodeRemoved(m_removedNode);
                m_removedNodeHistory.insert(command, m_removedNode);
                qDebug() << "Node removed: " << m_removedNode;
            } else {
                if (isUndo) {
                } else {
                    emit sigNodePixelChanged(curNode, m_updateRect);
                    qDebug() << "Captured pixel change on: " << curNode->name() << m_updateRect;
                    m_updateRectHistory.insert(command, qMakePair(curNode, m_updateRect));
                }
            }
        }
    }

    auto cmdNodePropList = dynamic_cast<const KisNodePropertyListCommand *>(command);
    if (cmdNodePropList) {
        emit sigNodeChanged(m_changedNode);
        qDebug() << "Captured property change on: " << m_changedNode->name();
    }

    resetState();
}
