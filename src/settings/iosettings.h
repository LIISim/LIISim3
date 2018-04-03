#ifndef IOSETTINGS_H
#define IOSETTINGS_H

#include "settingsbase.h"

class IOSettings : public SettingsBase
{
    Q_OBJECT

public:
    explicit IOSettings(QObject *parent = 0);

    ~IOSettings();

    void init();

signals:

public slots:

    QVariant value(QString subgroup, QString key);
    bool hasEntry(QString subgroup, QString key);
    void setValue(QString subgroup, QString key, QVariant value);
};

#endif // IOSETTINGS_H
