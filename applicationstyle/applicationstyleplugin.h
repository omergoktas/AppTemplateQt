#ifndef APPLICATIONSTYLEPLUGIN_H
#define APPLICATIONSTYLEPLUGIN_H

#include <QStylePlugin>

class ApplicationStylePlugin final : public QStylePlugin
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationStylePlugin)
    Q_PLUGIN_METADATA(IID QStyleFactoryInterface_iid FILE "applicationstyle.json")

public:
    ApplicationStylePlugin() = default;
    ~ApplicationStylePlugin() override = default;
    QStyle* create(const QString& key) override;
};

#endif // APPLICATIONSTYLEPLUGIN_H
