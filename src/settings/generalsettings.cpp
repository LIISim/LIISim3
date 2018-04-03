

#include "generalsettings.h"
#include <QDebug>
#include <QThread>

#include "../core.h"

GeneralSettings::GeneralSettings(QObject *parent) :  SettingsBase(parent)
{
    groupName = "General";

    key_dataDir ="dataPath";
    key_databaseDir = "databasePath";
    key_procChainDir = "procChainPath";
    key_programVersion = "programVersion";
    key_loadDataAtStartup = "loadDataAtStartup";
    key_coreCountImport = "coreCountImport";
    key_initDataStateFile = "initDataStateFile";
    key_locale = "locale";
    key_autoSaveSettings = "autoSaveSettings";
    key_autoSaveSettingsTime = "autoSaveSettingsTime";

    init();
}

GeneralSettings::~GeneralSettings()
{
}


void GeneralSettings::init()
{
    if(!settings.contains(key_dataDir))
        settings.insert(key_dataDir,Core::rootDir + "data/");

    if(!settings.contains(key_databaseDir))
        settings.insert(key_databaseDir,Core::rootDir + "data/database/");

    if(!settings.contains(key_procChainDir))
        settings.insert(key_procChainDir,Core::rootDir + "data/processingChains/");

    if(!settings.contains(key_initDataStateFile))
        settings.insert(key_initDataStateFile,Core::rootDir + "initSession.xml");

    if(!settings.contains(key_loadDataAtStartup))
        settings.insert(key_loadDataAtStartup, true);

    if(!settings.contains(key_programVersion))
    {        
        settings.insert(key_programVersion,Core::LIISIM_VERSION);
    }
    else if(settings.value(key_programVersion) != Core::LIISIM_VERSION)
    {
        qDebug() << "Program version does not match settings version!";
    }

    if(!settings.contains(key_coreCountImport))
        settings.insert(key_coreCountImport, QThread::idealThreadCount());
    else
    {
        int count = settings.value(key_coreCountImport).toInt();
        if(count < 1)
            count = 1;

        if(count > QThread::idealThreadCount())
            count = QThread::idealThreadCount();
        settings.insert(key_coreCountImport, count);
    }

    if(!settings.contains(key_locale))
        settings.insert(key_locale, 0);

    if(!settings.contains(key_autoSaveSettings))
        settings.insert(key_autoSaveSettings, true);

    if(!settings.contains(key_autoSaveSettingsTime))
        settings.insert(key_autoSaveSettingsTime, 5);
}


void GeneralSettings::setDataDirectory(const QString &path)
{
    settings.insert(key_dataDir, path);
    emit settingsChanged();
}


void GeneralSettings::setDatabaseDirectory(const QString &path)
{
    settings.insert(key_databaseDir, path);
    emit settingsChanged();
}


void GeneralSettings::setProcChainDirectory(const QString &path)
{
    settings.insert(key_procChainDir, path);
    emit settingsChanged();
}


void GeneralSettings::setLoadDataAtStartup(bool state)
{
    settings.insert(key_loadDataAtStartup, state);
    emit settingsChanged();
}


void GeneralSettings::setInitDataStateFilePath(const QString& path)
{
    settings.insert(key_initDataStateFile,path);
    emit settingsChanged();
}


void GeneralSettings::setCoreCountImport(int count)
{
    if(count < 1)
        count = 1;

    if(count > QThread::idealThreadCount())
        count = QThread::idealThreadCount();

    settings.insert(key_coreCountImport,count);
    emit settingsChanged();
}

void GeneralSettings::setAutoSaveSettings(bool enabled)
{
    settings.insert(key_autoSaveSettings, enabled);
}


void GeneralSettings::setAutoSaveSettingsTime(int minutes)
{
    settings.insert(key_autoSaveSettingsTime, minutes);
}


QString GeneralSettings::dataDirectory()
{
    return settings.value(key_dataDir).toString();
}


QString GeneralSettings::databaseDirectory()
{
    return settings.value(key_databaseDir).toString();
}


QString GeneralSettings::procChainDirectory()
{
    return settings.value(key_procChainDir).toString();
}


bool GeneralSettings::loadDataAtStartup()
{
    return settings.value(key_loadDataAtStartup).toBool();
}


int GeneralSettings::coreCountImport()
{
    return settings.value(key_coreCountImport).toInt();
}


QString GeneralSettings::initDataStateFilePath()
{
    return settings.value(key_initDataStateFile).toString();
}


void GeneralSettings::setLocale(QLocale locale)
{
    if(locale.language() == QLocale::German)
        settings.insert(key_locale, 1);
    else
        settings.insert(key_locale, 0);

    emit currentLocaleChanged(getLocale());
}


QLocale GeneralSettings::getLocale()
{
    if(settings.value(key_locale, 0) == 1)
        return QLocale(QLocale::German);
    else
        return QLocale(QLocale::C);
}

bool GeneralSettings::getAutoSaveSettings()
{
    return settings.value(key_autoSaveSettings).toBool();
}


int GeneralSettings::getAutoSaveSettingsTime()
{
    return settings.value(key_autoSaveSettingsTime).toInt();
}










