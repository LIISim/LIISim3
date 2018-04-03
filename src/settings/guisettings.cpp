#include "guisettings.h"

#include <QDir>

#include "../core.h"
#include "../logging/msghandlerbase.h"


const QString GuiSettings::_identifierSplitterSG = "splitter_position/";


GuiSettings::GuiSettings(QObject *parent) : SettingsBase(parent)
{
    MSG_DETAIL_1("init GuiSettings");
    groupName = "GuiSettings";

    key_masterWindowTab = "MasterWindow/SelectedTab";

    key_IO_csv_path     = "idiag/csv/lastDir";
    key_IO_custom_path  = "idiag/custom/lastDir";

    key_IO_csv_copyRawToAbs  = "idiag/csv/copyRawToAbs";

    _resetSplitterPosition = false;

    init();
}


void GuiSettings::init()
{
    if(!settings.contains(key_masterWindowTab))
        settings.insert(key_masterWindowTab, 0); // 0 = HOME tab

    QDir curDir = QDir(Core::rootDir + "exampleData/");

    if(!settings.contains(key_IO_csv_path))
        settings.insert(key_IO_csv_path, curDir.absolutePath());

    if(!settings.contains(key_IO_custom_path))
        settings.insert(key_IO_custom_path, curDir.absolutePath());

    if(!settings.contains(key_IO_csv_copyRawToAbs))
        settings.insert(key_IO_csv_copyRawToAbs, true);
}


void GuiSettings::write(QSettings &dest)
{
    if(!dest.isWritable())
    {
        qDebug() << "cannot write settings to " + dest.fileName();
        return;
    }

    dest.beginGroup(groupName);

    QStringList keys = settings.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        dest.setValue(keys.at(i),settings.value(keys.at(i)));
    }

    if(_resetSplitterPosition)
        dest.remove(_identifierSplitterSG);

    dest.endGroup();
}


void GuiSettings::resetSplitterPositions()
{
    _resetSplitterPosition = true;
}


/**
 * @brief GuiSettings::value
 * @param subgroup
 * @param key
 * @return
 */
QVariant GuiSettings::value(QString subgroup, QString key, QVariant defaultValue)
{
    QVariant v = settings.value(subgroup+"/"+key,defaultValue);
    return v;
}


bool GuiSettings::hasEntry(QString subgroup, QString key)
{
    return settings.contains(subgroup+"/"+key);
}


/**
 * @brief GuiSettings::setValue
 * @param subgroup
 * @param key
 * @param value
 */
void GuiSettings::setValue(QString subgroup, QString key, QVariant value, bool emitSignal)
{
    settings.insert(subgroup+"/"+key,value);

    if(emitSignal)
       emit guiSettingsChanged(subgroup, key, value);
}


void GuiSettings::setSplitterPosition(QString key, QVariant value)
{
    settings.insert(_identifierSplitterSG + key, value);
}


bool GuiSettings::getSplitterPosition(QString key, QVariant &value)
{
    if(settings.contains(_identifierSplitterSG + key))
    {
        value = settings.value(_identifierSplitterSG + key);
        return true;
    }
    return false;
}
