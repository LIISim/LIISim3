#ifndef SETTINGSBASE_H
#define SETTINGSBASE_H

#include <QObject>
#include <QMap>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/**
 * @brief The SettingsBase class serves as a abstract base class for various application settings.
 * It provides functionality for reading/writing stored values to a QSettings Object, which
 * is responsible for serialization
 */
class SettingsBase : public QObject
{
    Q_OBJECT

public:
    explicit SettingsBase(QObject *parent = 0);
    virtual ~SettingsBase() ;

    virtual QString toString();
    virtual void write(QSettings& dest);
    virtual void read(QSettings& src);

    virtual void readFromXML(QXmlStreamReader &r);
    virtual void writeToXML(QXmlStreamWriter &w);

protected:

    QString groupName;
    QMap<QString, QVariant> settings;

    /**
     * @brief init
     */
    virtual void init() = 0;

private:


signals:

    void settingsChanged();

public slots:

};

#endif // SETTINGSBASE_H
