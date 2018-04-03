#ifndef SIGNALFILEINFO_H
#define SIGNALFILEINFO_H

#include <QList>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "../signal/signal.h"

/**********************
 * TYPE definitions
 **********************/

/**
 * @brief The SignalIOType enum
 * @ingroup IO
 * lookup table for IOtype IDs (first element has ID 0)
 * *
 * COUNT_ENUM must be the last element !!!
 * *
 * this will represent the order in the select import type GUI (importdialog.cpp/exportdialog.cpp)
 */
enum SignalIOType {CUSTOM, CSV, CSV_SCAN, MAT, XML, COUNT_ENUM};


/**
 * @brief The SignalIOTypeXML enum
 * @ingroup IO
 * this represents the IDs used in XML files fpr itype
 * changing this order will result in problems with XML import!!!
 */
enum SignalIOTypeXML {XML_CUSTOM, XML_CSV, XML_CSV_SCAN, XML_MAT};


/**********************
 * SignalFileInfo
 **********************/

/**
 * @brief The SignalFileInfo class stores meta data of a single
 * datafile of a MRun.
 * @ingroup IO
 * It Contains informations about file location, file format and file
 * format specific properties.
 * Instances of this class are generated during Import operations
 * and are stored within a SignalIORequest.
 *
 */
class SignalFileInfo
{
    public:
        SignalFileInfo();

        void toXML(QXmlStreamWriter& w, QString &xmlDirPath);
        static SignalFileInfo fromXML(QXmlStreamReader& r, QString& xmlFname);

        SignalIOType itype;         ///< @brief IO type

        QString filename;           ///< @brief full filename
        int channelId;              ///< @brief Channel ID
        int signalId;               ///< @brief Signal ID

        Signal::SType signalType;   ///< @brief signal type

        QString delimiter;          ///< @brief delimiter for data (, or ;)
        QString decimal;            ///< @brief decimal mark for numbers
        QString timeunit;           ///< @brief unit of time data

        int headerlines;            ///< @brief number of header lines
        bool autoheader;            ///< @brief if true detects automatically header lines

        int fileLineCount;
        QString runname;
        bool stdevFile;

        QString shortFilename();
};


/**
 * @brief List of SignalFileInfo
 * @ingroup IO
 */
typedef QList<SignalFileInfo> SignalFileInfoList;


#endif // SIGNALFILEINFO_H
