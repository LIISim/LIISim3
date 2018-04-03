#include "signalfileinfo.h"


#include <QDir>

/**
 * @brief SignalFileInfo::SignalFileInfo Constructor
 */
SignalFileInfo::SignalFileInfo()
{
    filename    = "";
    channelId   = -1;
    signalId    = -1;
    delimiter   = ";";
    decimal     = ".";
    headerlines = 0;
    autoheader  = true;
    fileLineCount = 0;
    stdevFile = false;
}


/**
 * @brief SignalFileInfo::shortFilename Returns the name of the file,
 * excluding the path
 * @return
 */
QString SignalFileInfo::shortFilename()
{
    QFileInfo fi(filename);
    return fi.fileName();
}


/*****************************************
 *
 * SignalFileInfo XML-Serialization
 *
 *****************************************/

/**
 * @brief SignalFileInfo::toXML saves SignalIORequest to XML-Stream
 * @param w QXmlStreamWriter
 * @param xmlDirPath filename will be saved relative to this directory. If left empty,
 * the absolute filepath will be saved
 */
void SignalFileInfo::toXML(QXmlStreamWriter &w, QString& xmlDirPath)
{

    QString fname = filename;
    if(!xmlDirPath.isEmpty())
    {
        QDir dir(xmlDirPath);
        fname = dir.relativeFilePath(filename);
    }

    w.writeStartElement("SignalFileInfo");
    w.writeAttribute("filename",fname);
    w.writeAttribute("channelId",  QString("%0").arg(channelId));
    w.writeAttribute("signalId",   QString("%0").arg(signalId));
    w.writeAttribute("signalType", QString("%0").arg(signalType));

    // transform SignalIOtype to SignalIOTypeXML
    QString str;

    if(itype == CUSTOM)         str = QString("%0").arg(XML_CUSTOM);
    else if(itype == CSV)       str = QString("%0").arg(XML_CSV);
    else if(itype == CSV_SCAN)  str = QString("%0").arg(XML_CSV_SCAN);
    else if(itype == MAT)       str = QString("%0").arg(XML_MAT);

    w.writeAttribute("itype",    str);

    w.writeAttribute("headerlines",QString("%0").arg(headerlines));

    if(stdevFile)
        w.writeAttribute("stdev", QString("%0").arg(true));

    w.writeEndElement(); // SignalFileInfo
}


/**
 * @brief SignalFileInfo::fromXML loads SignalIORequest from XML-Stream
 * @param r QXmlStreamReader
 * @return  SignalFileInfo
 */
SignalFileInfo SignalFileInfo::fromXML(QXmlStreamReader &r, QString &xmlFname)
{
    SignalFileInfo fi;

    if(r.name()!= "SignalFileInfo")
        return fi;

    QXmlStreamAttributes a = r.attributes();

    QString fname =  a.value("filename").toString();

    //if fname is empty, .absoluteFilePath results in path to session file, so skip path building
    //and let the file import handle this as an error (...file not found)
    if(!fname.isEmpty())
    {
        // make path absolute
        fname = QDir(xmlFname).absoluteFilePath(fname);

        // cleanup path
        QString fnametemp = QDir(fname).canonicalPath();

        //.canonicalPath returns empty string if path/file not found, so use path only if not empty,
        //to allow error handling in file import
        if(!fnametemp.isEmpty())
            fname = fnametemp;
    }

    fi.filename  = fname;
    fi.channelId = a.value("channelId").toInt();
    fi.signalId  = a.value("signalId").toInt();

    int stype = a.value("signalType").toInt();

    if(stype  == 0)     fi.signalType = Signal::RAW;
    else if(stype == 1) fi.signalType = Signal::ABS;
    else if(stype == 2) fi.signalType = Signal::TEMPERATURE;

    int ity = a.value("itype").toInt();

    if(ity == XML_CUSTOM)        fi.itype = CUSTOM;
    else if(ity == XML_CSV)      fi.itype = CSV;
    else if(ity == XML_CSV_SCAN) fi.itype = CSV_SCAN;
    else if(ity == XML_MAT)      fi.itype = MAT;

    fi.headerlines = a.value("headerlines").toInt();

    if(!a.value("stdev").isEmpty())
        fi.stdevFile = a.value("stdev").toInt();

    return fi;
}
