#include "iosettings.h"
#include "../logging/msghandlerbase.h"

IOSettings::IOSettings(QObject *parent) : SettingsBase(parent)
{
    MSG_DETAIL_1("init IOSettings");
    groupName = "IOSettings";

    init();
}

IOSettings::~IOSettings() {}

void IOSettings::init() {}

/**
 * @brief IOSettings::value
 * @param subgroup
 * @param key
 * @return
 */
QVariant IOSettings::value(QString subgroup, QString key)
{
    QVariant v = settings.value(subgroup+"/"+key);
    return v;
}


bool IOSettings::hasEntry(QString subgroup, QString key)
{
    return settings.contains(subgroup+"/"+key);
}


/**
 * @brief IOSettings::setValue
 * @param subgroup
 * @param key
 * @param value
 */
void IOSettings::setValue(QString subgroup, QString key, QVariant value)
{
    settings.insert(subgroup+"/"+key,value);
    emit settingsChanged();
}
