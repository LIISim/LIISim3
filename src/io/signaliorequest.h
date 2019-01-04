#ifndef SIGNALIOREQUEST_H
#define SIGNALIOREQUEST_H

#include <QVariant>
#include <QMap>
#include "signalfileinfo.h"

/**********************
 * SignalIORequest
 **********************/

/**
 * @brief The SignalIORequest class stores Metadata about the MRun's
 * Datafiles used during IO-Operations.
 * @ingroup IO
 * This comprises a set of SignalFileInfo,
 * informations about file locations, file formats, format specific
 * properties/flags and the mapping to the internal MRun Datastructure.
 * This class is used during all kinds of Data-Import/Export Operations
 * to define how the data of a Run should be treated: Eg. what files or
 * signal types should be exported/imported,etc.
 *
 * The main purpose of the SignalIORequest class is to store the
 * procedure which has been performed by the user during an
 * Data-Import/Export Operation. This allows to replicate a specific
 * IO-Operation without requiring any additional user interaction.
 * This feature is heavily used when saving XML-Sessions of the current
 * program state (also see IOxml, ImportDialog, ExportDialog).
 */
class SignalIORequest
{    
public:
    SignalIORequest();

    void toXML(QXmlStreamWriter& w, QString &xmlDir);
    static SignalIORequest fromXML(QXmlStreamReader& r, QString& xmlFname);

    SignalIOType itype;         ///< @brief IO-Type, defines data format

    int noChannels;             ///< @brief number of channels
    int noSignals;              ///< @brief number of signals
    int run_id;                 ///< @brief id of measurement run
    int group_id;               ///< @brief id of mrungroup

    QString runsettings_dirpath;///< @brief path to MRunSettings file
    QString runsettings_filename;
    QString runname;            ///< @brief name of MRun
    QString description;        ///< @brief description of MRun
    QString datadir;            ///< @brief data directory (for eventual scan)
    bool    subdir;             ///< @brief include subdirectories (for eventual scan)

    bool channelPerFile;        ///< @brief if true only one channel per file

    QString delimiter;          ///< @brief delimiter for data (, or ;)
    QString decimal;            ///< @brief decimal mark for numbers
    QString timeunit;           ///< @brief unit of time data
    SignalFileInfoList flist;   ///< @brief list of signal file informations

    /** @brief Map of additional data/flags
    * - 0: filename/directorypath/runname (depends  on iotype)
    * - 1: run description (for some iotypes only)
    * - 2: xml io: relative path flag
    * - 3: LIISettings filename (selection of user)
    * - 4:
    * - 5: group id of mrun (redundant should be removed in the future)
    * - 6: csv-auto: load-raw flag
    * - 7: csv-auto: load-abs flag
    * - 8: csv-export/xml io: list of checked run ids
    * - 9: xml io: include modeling settings
    * - 10: xml io: include gui settings
    * - 11: xml io: include general settings
    * - 12: xml io: include data
    * - 13: export flag: [true=] save raw data during export
    * - 14: export flag: [true=] save absolute data during export
    * - 15: export flag: [true=] save temperature data during export
    * - 16: export flag: [true=] export raw POSTprocessed data (else unprocessed)
    * - 17: export flag: [true=] export abs POSTprocessed flag (else unprocessed)
    * - 18: csv-auto import flag: [true=] copy raw data to absolute data
    * - 19: xml import: defines how to handle existing data
    *       (0: clear, 1: add data, ignore psteps, 2: add data, overwrite psteps)
    * - 20: export flag: [true=] use filenameBase for export
    *
    * - 25: export flag: [true=] save POSTprocessed temperature data
    * - 26: export flag: [true=] save standard deviation temperature data
    *
    * - 28: export flag: [true=] save standard deviation raw data during export
    * - 29: export flag: [true=] save standard deviation absolute data during export
    *
    * --- Additional data for directory scans ---
    * - 30: Bool    | [true =] MRunSettings-file found
    * - 31: QString | Contains run name saved in settings file
    *
    * - 35: export flag: [true=] matlab export compression enabled
    */
    QMap<int,QVariant> userData;

    //custom fields for custom import
    QString fname_txt_1;        ///< @brief allows any custom filename
    QString fname_txt_2;
    QString fname_txt_3;
    QString fname_txt_4;
    QString fname_var_1;        ///< @brief specifies variable parts of filename
    QString fname_var_2;
    QString fname_var_3;
    QString extension;          ///< @brief file extension

    int headerlines;            ///< @brief number of header lines
    bool autoheader;            ///< @brief if true detects automatically header lines
    bool autoName;              ///< @brief generates runname automatically
};

#endif // SIGNALIOREQUEST_H
