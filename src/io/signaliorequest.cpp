#include "signaliorequest.h"

#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include "../core.h"
#include "../signal/mrungroup.h"

/**
 * @brief SignalIORequest::SignalIORequest Constructor
 */
SignalIORequest::SignalIORequest()
{
    // set standard values
    noSignals   = 0;
    noChannels  = 0;

    group_id    = 0;
    run_id      = -1;
    subdir      = false;
    autoheader  = false;
    autoName    = false;    
    headerlines = 0;

    channelPerFile = true;

    datadir     = "";
    runname     = "-";
    runsettings_dirpath = "";

    delimiter   = ";";
    decimal     = ".";
    timeunit    = "s";
}


/**
 * @brief SignalIORequest::toXML saves SignalIORequest to XML-Stream
 * @param w current QXmlStreamWriter
 * @param xmlDir absolute path to directory of the XML-File
 */
void SignalIORequest::toXML(QXmlStreamWriter &w, QString& xmlDir)
{
    w.writeStartElement("SignalIORequest");

    // transform SignalIOtype to SignalIOTypeXML
    QString str;
    if(itype == CUSTOM)         str = QString("%0").arg(XML_CUSTOM);
    else if(itype == CSV)       str = QString("%0").arg(XML_CSV);
    else if(itype == CSV_SCAN)  str = QString("%0").arg(XML_CSV_SCAN);
    else if(itype == MAT)       str = QString("%0").arg(XML_MAT);

    w.writeAttribute("itype", str);
    w.writeAttribute("runname", runname);

    QString runSettingsDir = runsettings_dirpath;
    if(!xmlDir.isEmpty())
    {
        QDir dir(xmlDir);
        runSettingsDir = dir.relativeFilePath(runsettings_dirpath);
    }

    if(!runSettingsDir.isEmpty())
        if(!runSettingsDir.endsWith("/"))
            runSettingsDir.append("/");

    w.writeAttribute( "runsettings_dirpath", runSettingsDir);
    w.writeAttribute( "datadir", datadir);
    w.writeAttribute( "liiSettings",userData.value(3).toString());
    w.writeAttribute( "noChannels",QString("%0").arg(noChannels));
    w.writeAttribute( "subdir", QString("%0").arg((int)subdir));

    MRunGroup* g = Core::instance()->dataModel()->group(group_id);
    w.writeAttribute( "gname",g->title());

    w.writeAttribute( "delimiter", delimiter);
    w.writeAttribute( "decimal", decimal);

    if(itype == CUSTOM)
    {
        w.writeAttribute("fname_txt_1",fname_txt_1);
        w.writeAttribute("fname_txt_2",fname_txt_2);
        w.writeAttribute("fname_txt_3",fname_txt_3);
        w.writeAttribute("fname_txt_4",fname_txt_4);
        w.writeAttribute("fname_var_1",fname_var_1);
        w.writeAttribute("fname_var_2",fname_var_2);
        w.writeAttribute("fname_var_3",fname_var_3);
        w.writeAttribute("extension",extension);
        w.writeAttribute("headerlines",QString("%0").arg(headerlines));
        w.writeAttribute("autoheader",QString("%0").arg((int)autoheader));
        w.writeAttribute("oneChannelPerFile",QString("%0").arg(channelPerFile));
        w.writeAttribute( "timeunit", timeunit);
    }
    else if(itype == CSV)
    {
        w.writeAttribute("copyRawToAbs",userData.value(18,false).toString());
    }
    // else
    for(int i = 0; i < flist.size(); i++)
    {
        flist[i].toXML(w,xmlDir);
    }


    w.writeEndElement(); // SignalIORequest
}


/**
 * @brief SignalIORequest::fromXML loads SignalIORequest from XML-Stream
 * @param r QXmlStreamReader
 * @param xmlFname name of the XML-File
 * @return SignalIORequest read from XML-Stream
 */
SignalIORequest SignalIORequest::fromXML(QXmlStreamReader &r, QString &xmlFname)
{
    SignalIORequest q;

    if(r.tokenType() != QXmlStreamReader::StartElement &&
        r.name() == "SignalIORequest")
    {
        return q;
    }

    QXmlStreamAttributes a = r.attributes();

    int ity = a.value("itype").toInt();

    if(ity == XML_CUSTOM)        q.itype = CUSTOM;
    else if(ity == XML_CSV)      q.itype = CSV;
    else if(ity == XML_CSV_SCAN) q.itype = CSV_SCAN;
    else if(ity == XML_MAT)      q.itype = MAT;

    q.runname = a.value("runname").toString();
    QString dirpath = a.value("runsettings_dirpath").toString();

    if(!dirpath.isEmpty())
    {
        // make path absolute
        dirpath = QDir(xmlFname).absoluteFilePath(dirpath);

        // cleanup path
        q.runsettings_dirpath = QDir(dirpath).canonicalPath().append("/");
    }
    else
    {
        q.runsettings_dirpath = dirpath;
    }

    q.datadir = a.value("datadir").toString();

    // get channel count from LII-Settings
    q.noChannels = a.value("noChannels").toInt();

    q.subdir    = a.value("subdir").toInt();
    q.delimiter = a.value("delimiter").toString();
    q.decimal   = a.value("decimal").toString();

    q.userData.insert(3, a.value("liiSettings").toString());

    // determine group id from name
    QString gname = a.value("gname").toString();
    q.group_id = -1;
    QList<MRunGroup*> gl = Core::instance()->dataModel()->groupList();

    for(int i = 0; i < gl.size(); i++)
        if(gl[i]->title() == gname)
        {
            q.group_id = gl[i]->id();
            break;
        }

    //if group with requested name does not exist, save name to create it later
    if(q.group_id == -1)
    {
        q.userData.insert(5,gname);
    }

    if(q.itype == CUSTOM)
    {
        q.fname_txt_1 = a.value("fname_txt_1").toString();
        q.fname_txt_2 = a.value("fname_txt_2").toString();
        q.fname_txt_3 = a.value("fname_txt_3").toString();
        q.fname_txt_4 = a.value("fname_txt_4").toString();
        q.fname_var_1 = a.value("fname_var_1").toString();
        q.fname_var_2 = a.value("fname_var_2").toString();
        q.fname_var_3 = a.value("fname_var_3").toString();
        q.extension   = a.value("extension").toString();
        q.headerlines = a.value("headerlines").toInt();
        q.autoheader  = a.value("autoheader").toInt();
        if(a.hasAttribute("oneChannelPerFile"))
            q.channelPerFile = a.value("oneChannelPerFile").toInt();
        q.timeunit   = a.value("timeunit").toString();
    }
    else if(q.itype == CSV)
    {
        if(a.hasAttribute("copyRawToAbs")){
            q.userData.insert(18,a.value("copyRawToAbs").toString());
        }
    }

    while( !( (r.name() == "SignalIORequest")  &&
             r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            if(r.name() == "SignalFileInfo")
            {
                SignalFileInfo fi = SignalFileInfo::fromXML(r,xmlFname);
               //  qDebug() << "\tparsed file info: "<<fi.filename;

                fi.decimal = q.decimal;
                fi.delimiter = q.delimiter;
                fi.autoheader = q.autoheader;
                fi.timeunit = q.timeunit;

                q.flist.append(fi);
            }
        }
        r.readNext();
    }
    return q;
}


