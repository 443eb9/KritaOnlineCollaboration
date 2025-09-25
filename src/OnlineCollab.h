#ifndef _ONLINE_COLLAB_H_
#define _ONLINE_COLLAB_H_

#include <QObject>
#include <QVariant>

class OnlineCollabPlugin : public QObject
{
    Q_OBJECT
public:
    OnlineCollabPlugin(QObject *parent, const QVariantList &);
    ~OnlineCollabPlugin() override;
};

#endif
