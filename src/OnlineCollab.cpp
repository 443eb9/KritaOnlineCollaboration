#include "OnlineCollab.h"
#include "OnlineCollabDock.h"

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <KoDockFactoryBase.h>
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY_WITH_JSON(OnlineCollabPluginFactory, "onlinecollab.json", registerPlugin<OnlineCollabPlugin>();)

class OnlineCollabDockFactory : public KoDockFactoryBase
{
public:
    OnlineCollabDockFactory()
    {
    }

    QString id() const override
    {
        return QString("OnlineCollab");
    }

    QDockWidget *createDockWidget() override
    {
        OnlineCollabDock *dockWidget = new OnlineCollabDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override
    {
        return DockRight;
    }
};

OnlineCollabPlugin::OnlineCollabPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new OnlineCollabDockFactory());
}

OnlineCollabPlugin::~OnlineCollabPlugin()
{
}

extern "C" {
Q_DECL_EXPORT void load_online_collab_plugin()
{
    qDebug() << "Start loading online collab plugin...";
    OnlineCollabPlugin plugin(nullptr, QVariantList());
}
}

#include "OnlineCollab.moc"
