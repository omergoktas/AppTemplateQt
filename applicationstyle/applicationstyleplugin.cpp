#include "applicationstyleplugin.h"
#include "applicationstyle.h"

QStyle* ApplicationStylePlugin::create(const QString& key)
{
    if (key.toLower() == "applicationstyle"_L1)
        return new ApplicationStyle();
    return nullptr;
}
