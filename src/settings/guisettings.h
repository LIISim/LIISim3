#ifndef GUISETTINGS_H
#define GUISETTINGS_H

#include "settingsbase.h"

/**
 * @brief The GuiSettings class holds states of GUI elements.
 * TODO.
 */
class GuiSettings : public SettingsBase
{
    Q_OBJECT
public:
    explicit GuiSettings(QObject *parent = 0);

    ~GuiSettings(){}

    void init();

    virtual void write(QSettings &dest);

    void resetSplitterPositions();

private:
    QString key_masterWindowTab;
    QString key_IO_csv_path;
    QString key_IO_custom_path;
    QString key_IO_csv_copyRawToAbs;
    bool _resetSplitterPosition;

    static const QString _identifierSplitterSG;

signals:
    void guiSettingsChanged(QString subgroup, QString key, QVariant value);

public slots:

    QVariant value(QString subgroup, QString key, QVariant defaultValue = 0);
    bool hasEntry(QString subgroup, QString key);
    void setValue(QString subgroup, QString key, QVariant value, bool emitSignal = false);

    void setSplitterPosition(QString key, QVariant value);
    bool getSplitterPosition(QString key, QVariant &value);
};

#endif // GUISETTINGS_H
