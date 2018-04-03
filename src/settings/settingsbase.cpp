#include "settingsbase.h"

#include <QDebug>

/**
 * @brief SettingsBase::SettingsBase
 * @param parent
 */
SettingsBase::SettingsBase(QObject *parent) : QObject(parent)
{
    groupName = "None";
}

SettingsBase::~SettingsBase() {}


/**
 * @brief SettingsBase::write
 * @param dest
 */
void SettingsBase::write(QSettings &dest)
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
    dest.endGroup();
}


/**
 * @brief SettingsBase::read
 * @param src
 */
void SettingsBase::read(QSettings &src)
{
    settings.clear();

    src.beginGroup(groupName);
    QStringList keys = src.allKeys();
    for(int i = 0; i < keys.size(); i++)
    {
        settings.insert(keys.at(i),src.value(keys.at(i)));
    }
    src.endGroup();
    init();

    emit settingsChanged();
}


QString SettingsBase::toString()
{
    QString res;

    res.append(groupName+"\n");

    QStringList keys = settings.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        res.append("\t"+keys.at(i)+": "+ settings.value(keys.at(i)).toString()+"\n");
    }
    return res;
}


// -----------------
// XML Serialization
// -----------------


/**
 * @brief SettingsBase::writeToXML writes settings as a xml tag to output stream
 * @param w output xml stream writer
 */
void SettingsBase::writeToXML(QXmlStreamWriter &w)
{
    w.writeStartElement(groupName);
    QStringList keys = settings.keys();

    for(int i = 0; i < keys.size(); i++)
    {
        w.writeStartElement("param");
        w.writeAttribute( "key", keys.at(i) );
        w.writeAttribute( "value", settings.value(keys.at(i)).toString() );
        w.writeEndElement(); //param
    }
    w.writeEndElement(); // groupname
}


/**
 * @brief SettingsBase::readFromXML reads settings from open xml stream if current token
 * name equals group-name
 * @param r
 */
void SettingsBase::readFromXML(QXmlStreamReader &r)
{
    // case: wrong element name or token is end element of settings tag
    if(r.name() != groupName ||
      (r.tokenType() != QXmlStreamReader::StartElement && r.name() == groupName))
        return;

    while( !(r.name() == groupName &&  r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            if(r.name() == "param")
            {
                QXmlStreamAttributes a = r.attributes();
                QString key = a.value("key").toString();
                QString value =a.value("value").toString();
                //qDebug() << "added: "+key+" = " +value;
                settings.insert(key,value);
            }
        }
        r.readNext();
    }
    init();
}
