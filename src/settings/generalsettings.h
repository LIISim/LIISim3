#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QLocale>

#include "settingsbase.h"

class GeneralSettings : public SettingsBase
{
    Q_OBJECT

public:
    explicit GeneralSettings(QObject *parent = 0);

    ~GeneralSettings();


    void setDataDirectory(const QString & path);
    void setDatabaseDirectory(const QString & path);
    void setProcChainDirectory(const QString & path);
    void setLoadDataAtStartup(bool state);
    void setInitDataStateFilePath(const QString & path);

    void setCoreCountImport(int count);

    void setLocale(QLocale locale);

    QString dataDirectory();
    QString databaseDirectory();
    QString procChainDirectory();
    QString initDataStateFilePath();

    int coreCountImport();

    QLocale getLocale();

    bool loadDataAtStartup();

    void setAutoSaveSettings(bool enabled);
    bool getAutoSaveSettings();

    void setAutoSaveSettingsTime(int minutes);
    int getAutoSaveSettingsTime();

protected:

private:
    void init();

    QString key_dataDir;
    QString key_databaseDir;
    QString key_procChainDir;
    QString key_programVersion;
    QString key_loadDataAtStartup;
    QString key_coreCountImport;
    QString key_initDataStateFile;
    QString key_locale;
    QString key_autoSaveSettings;
    QString key_autoSaveSettingsTime;

signals:

    void currentLocaleChanged(QLocale locale);

public slots:

};

#endif // GENERALSETTINGS_H
