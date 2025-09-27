#ifndef _IMAGE_CHANGE_ANALYZER_H_
#define _IMAGE_CHANGE_ANALYZER_H_

#include <QObject>
#include <QUuid>

#include <kis_canvas2.h>
#include <kis_node.h>
#include <kundo2command.h>

class ImageChangeAnalyzer : public QObject
{
    Q_OBJECT

public:
    ImageChangeAnalyzer(KisCanvas2 *canvas);

Q_SIGNALS:
    void sigNodeChanged(KisNodeSP node);
    void sigNodeAdded(const QUuid &uuid);
    void sigNodeRemoved(const QUuid &uuid);
    void sigNodePixelChanged(KisNodeSP node, const QRect &rect);

private:
    KisCanvas2 *m_canvas;

    int m_lastUndoIndex;
    KisNodeSP m_changedNode;
    KisNodeSP m_addedNode;
    QUuid m_removedNode;
    QRect m_updateRect;

    QHash<const KUndo2Command *, QRect> m_updateRectHistory;
    QHash<const KUndo2Command *, QUuid> m_addedNodeHistory;

    void resetState();

private Q_SLOTS:
    void undoIndexChanged(int index);
};

#endif
