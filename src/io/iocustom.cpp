#include "iocustom.h"

#include <QDir>
#include <QDirIterator>
#include <QMap>
#include <QStack>
#include <QRegExp>

#include "../core.h"
#include "../settings/mrunsettings.h"

IOcustom::IOcustom(QObject *parent) :   IOBase(parent)
{
    // set parallelization to one channel per worker
    // note: per-file method does not make sense for those small datafiles,
    // furthermore the per-file method cannot ensure correct order of signal files!
    // Thus the parallelization mode is set to per-channel mode, this setting should not
    // changed.
    p_mode = PM_PERCHANNEL;
}


// ------------------------
// IMPLEMENTATION OF IOBASE
// ------------------------

void IOcustom::setupImport()
{
    if(m_initialRequest.itype == CUSTOM)
    {
        // change import parallelization method
        if(m_initialRequest.channelPerFile == false)
             p_mode = PM_PERMRUN;

        try
        {
            bool subdir = m_initialRequest.subdir;

            if(subdir == true)
                scanMainDirectory();
            else
            {
                // check if we already have a list of files.
                // in this case we do not need to scan the directory again!
                if(m_initialRequest.flist.isEmpty())
                {
                    //qDebug() << "custom: simple dir scan";
                    scanDirectory(m_initialRequest);
                }
                else
                {
                    //qDebug() << "custom: no scan";
                    m_generatedRequests.append(m_initialRequest);
                }
            }

        }
        catch(LIISimException e)
        {
            emit logMessage(e.what(),e.type());
        }
    }
    else
    {
        logMessage("IOcustom: cannot handle SignalIORequest: wrong IOtype!");
    }

    emit importSetupFinished();
}


/**
 * @brief IOcustom::importStep this method loads a list of data files to the given
 * mrun object. This Method is called multiple times, concurrently! Programmer must
 * ensure thread safety of code!
 * @param mrun
 * @param fileInfos
 */
void IOcustom::importStep(MRun *mrun, SignalFileInfoList fileInfos)
{
    int noSignals = 0;

    // assume that the file info list contains all signal files of one mrun-channel
    // (this is provided by the PM_PERCHANNEL parallelization mode)
    // and assume that all signal file infos are stored in correct order within the list
    // (this is ensured by the scanDirectory() method)

    for(int i = 0; i < fileInfos.size(); i++)
    {

        // check if abort flag is set
        if(abort_flag)
        {
            //qDebug() << "IOcustom::importStep of run " << mrun->getName()<< " aborted!";
            importStepFinished(0);
            return;
        }

        SignalFileInfo fi = fileInfos.at(i);

        try
        {
            noSignals += loadFromCustomFile(mrun,fi);
        }
        catch(LIISimException e)
        {
            logMessage(e.what(),e.type());
        }
    }
    emit importSuccess(mrun->importRequest(), fileInfos);
    emit importStepFinished(noSignals);
}


void IOcustom::exportImplementation(const SignalIORequest &rq)
{
    logMessage("Custom-IO: EXPORT NOT IMPLEMENTED",INFO);
    emit exportImplementationFinished(0.0);
}


/**
 * @brief IOcustom::checkFiles
 */
void IOcustom::checkFiles()
{
    QString dirname = m_initialRequest.datadir;

    QDir datdir(dirname);
    if(!datdir.exists())
    {
        throw LIISimException("Data directory does not exist ("+dirname+")",ERR_IO);
    }

    datdir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QDirIterator::IteratorFlag it_flag;

    if(m_initialRequest.subdir)
    {
        logMessage(QString("CustomImport: Scan subdirectories of ").append(dirname), DEBUG);
        it_flag = QDirIterator::Subdirectories;
    }
    else
    {
        logMessage(QString("CustomImport: Scan directory ").append(dirname), DEBUG);
        it_flag = QDirIterator::NoIteratorFlags;
    }

    QDirIterator it(datdir, it_flag);
    QStack<QString> filesStack;

    while(it.hasNext())
        filesStack.push(it.next().remove(dirname));

    QList<SignalIORequest> requestList;
    SignalFileInfoList importFlist;

    // throws ERROR: check if channel is defined in file pattern
    if(m_initialRequest.fname_var_1 != "<CHANNEL>"
            && m_initialRequest.fname_var_2 != "<CHANNEL>"
            && m_initialRequest.fname_var_3 != "<CHANNEL>"
            && m_initialRequest.channelPerFile == true)
    {
        emit checkFilesResult(requestList);
        emit logMessage("<CHANNEL> not defined in file pattern: Import only single channel", ERR_IO);
        throw LIISimException("<CHANNEL> not defined in file pattern: Import only single channel", ERR_IO);
    }

    // throws WARNING: import only one run
    if(m_initialRequest.fname_var_1 == "<RUN>"
            && m_initialRequest.fname_var_2 == "<RUN>"
            && m_initialRequest.fname_var_3 == "<RUN>")
    {
       emit logMessage("<RUN> not defined in file pattern: Import only single Run", WARNING);
    }

    // throws WARNING: import only one signal
    if(m_initialRequest.fname_var_1 == "<SIGNAL>"
            && m_initialRequest.fname_var_2 == "<SIGNAL>"
            && m_initialRequest.fname_var_3 == "<SIGNAL>")
    {
       emit logMessage("Signal not in file pattern defined: Import only single signal", WARNING);
    }

    QRegExp reg_exp;
    reg_exp.setPattern(
                 "^(" + m_initialRequest.fname_txt_1 + ")"
                + "(" + getFnamePatternRegExp(m_initialRequest.fname_var_1) + ")"
                + "(" + m_initialRequest.fname_txt_2 + ")"
                + "(" + getFnamePatternRegExp(m_initialRequest.fname_var_2) + ")"
                + "(" + m_initialRequest.fname_txt_3 + ")"
                + "(" + getFnamePatternRegExp(m_initialRequest.fname_var_3) + ")"
                + "(" + m_initialRequest.fname_txt_4 + ")"
                + "(\\" + m_initialRequest.extension + ")"
                "$");

    logMessage("CustomImport: Search pattern: "+reg_exp.pattern());

    for(int i = 0; i < filesStack.size(); i++)
    {
        QString filename = filesStack.at(i).split("/").last();
        int chId;
        int signalId;
        QString run_name;

        //qDebug() << "checking" << filename;
        if(reg_exp.indexIn(filename) != -1)
        {
            //qDebug() << "regexp ok";
            logMessage("Found file: " + filename);

            if(m_initialRequest.channelPerFile == true)
            {
                if(m_initialRequest.fname_var_1 == "<CHANNEL>")
                    chId = reg_exp.cap(2).toInt();
                else if(m_initialRequest.fname_var_2 == "<CHANNEL>")
                    chId = reg_exp.cap(4).toInt();
                else if(m_initialRequest.fname_var_3 == "<CHANNEL>")
                    chId = reg_exp.cap(6).toInt();
            }

            if(m_initialRequest.fname_var_1 == "<RUN>")
                run_name = reg_exp.cap(2);
            else if(m_initialRequest.fname_var_2 == "<RUN>")
                run_name = reg_exp.cap(4);
            else if(m_initialRequest.fname_var_3 == "<RUN>")
                run_name = reg_exp.cap(6);
            else
            {
               // import only one run
                run_name = "CUSTOM_notset";
            }

            if(m_initialRequest.fname_var_1 == "<SIGNAL>")
                signalId = reg_exp.cap(2).toInt();
            else if(m_initialRequest.fname_var_2 == "<SIGNAL>")
                signalId = reg_exp.cap(4).toInt();
            else if(m_initialRequest.fname_var_3 == "<SIGNAL>")
                signalId = reg_exp.cap(6).toInt();
            else
            {
               // import only one signal
                signalId = 1;
                emit logMessage("Signal not in file pattern defined: Import only single signal", WARNING);
            }

            SignalFileInfo fileinfo;
            fileinfo.runname     = run_name;
            fileinfo.itype       = CUSTOM;
            fileinfo.filename    = m_initialRequest.datadir + filesStack.at(i);
            fileinfo.delimiter   = m_initialRequest.delimiter; // assign general delimiter to each file
            fileinfo.decimal     = m_initialRequest.decimal;   // assign general decimal to each file
            fileinfo.timeunit    = m_initialRequest.timeunit;
            fileinfo.headerlines = m_initialRequest.headerlines;
            fileinfo.autoheader  = m_initialRequest.autoheader;

            if(m_initialRequest.channelPerFile)
                fileinfo.channelId = chId;
            else
                fileinfo.channelId = 0;

            fileinfo.signalId = signalId;
            fileinfo.signalType = Signal::RAW;

            importFlist.push_back(fileinfo);
        }
    }

    //filter for unwanted files
    for(int i = 0; i < importFlist.size();)
    {
        //remove files which contain '*_settings.txt' in the filename,
        //since the settings files will be loaded later
        if(importFlist.at(i).filename.contains("_settings.txt"))
        {
            importFlist.removeAt(i);
            continue;
        }
        //remove database files if detected by mistake
        else
        {
            QFile file(importFlist.at(i).filename);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QString line = file.readLine();
                file.close();

                QStringList token = line.split(";");

                if(token.size() > 1)
                {
                    if(token.at(1).contains("gas", Qt::CaseInsensitive) ||
                            token.at(1).contains("material", Qt::CaseInsensitive) ||
                            token.at(1).contains("liisettings", Qt::CaseInsensitive) ||
                            token.at(1).contains("gasmixture", Qt::CaseInsensitive) ||
                            token.at(1).contains("laserenergy", Qt::CaseInsensitive) ||
                            token.at(1).contains("spectrum", Qt::CaseInsensitive) ||
                            token.at(1).contains("transmission", Qt::CaseInsensitive))
                    {
                        importFlist.removeAt(i);
                        continue;
                    }
                }
            }
        }
        i++;
    }

    if(m_initialRequest.channelPerFile)
    {
        for(int i = 0; i < importFlist.size(); i++)
        {
            int index = -1;
            for(int j = 0; j < requestList.size(); j++)
                if(requestList.at(j).runname == importFlist.at(i).runname)
                    index = j;

            if(index == -1)
            {
                SignalIORequest request;
                request.runname     = importFlist.at(i).runname;
                request.itype       = CUSTOM;
                request.delimiter   = m_initialRequest.delimiter; // assign general delimiter to each file
                request.decimal     = m_initialRequest.decimal;   // assign general decimal to each file
                request.timeunit    = m_initialRequest.timeunit;
                request.headerlines = m_initialRequest.headerlines;
                request.autoheader  = m_initialRequest.autoheader;
                request.noChannels  = 0;
                request.noSignals   = 0;
                request.datadir     = m_initialRequest.datadir;
                request.runsettings_dirpath = importFlist.at(i).filename;
                request.runsettings_dirpath.remove(importFlist.at(i).filename.split("/").last());
                request.runsettings_filename = request.runname;
                request.runsettings_filename.append("_settings.txt");
                request.channelPerFile = m_initialRequest.channelPerFile;
                request.flist.push_back(importFlist.at(i));

                if(request.noChannels < importFlist.at(i).channelId)
                    request.noChannels = importFlist.at(i).channelId;
                if(request.noSignals < importFlist.at(i).signalId)
                    request.noSignals = importFlist.at(i).signalId;

                QString settingsFile = QString(request.runsettings_dirpath).append(request.runsettings_filename);
                QFileInfo fi(settingsFile);
                if(fi.exists())
                {
                    //request.userData.insert(30, true);
                    //request.userData.insert(31, request.runsettings_filename);

                    MRunSettings ms(settingsFile);

                    if(!ms.liiSettingsFname().isEmpty())
                    {
                        request.userData.insert(3, ms.liiSettingsFname());
                        LIISettings mls = IOBase::getLIISettings(request);
                    }

                    request.userData.insert(30, true);
                    request.userData.insert(31, ms.runName());
                }

                requestList.push_back(request);
            }
            else
            {
                requestList[index].flist.push_back(importFlist.at(i));
                if(requestList.at(index).noChannels < importFlist.at(i).channelId)
                    requestList[index].noChannels = importFlist.at(i).channelId;
                if(requestList.at(index).noSignals < importFlist.at(i).signalId)
                    requestList[index].noSignals = importFlist.at(i).signalId;
            }
        }

        // check if the signalid starts at zero and correct it,
        // since we assume everywhere else a value > 0
        for(int i = 0; i < requestList.size(); i++)
        {
            int signalMinID = INT_MAX;

            for(SignalFileInfo file : requestList.at(i).flist)
                if(file.signalId < signalMinID)
                    signalMinID = file.signalId;

            if(signalMinID == 0)
                for(int j = 0; j < requestList[i].flist.size(); j++)
                    requestList[i].flist[j].signalId++;
        }
    }
    else
    {
        for(int i = 0; i < importFlist.size(); i++)
        {
            SignalIORequest request;
            request.runname = importFlist.at(i).runname;
            request.itype = CUSTOM;
            request.delimiter   = m_initialRequest.delimiter; // assign general delimiter to each file
            request.decimal     = m_initialRequest.decimal;   // assign general decimal to each file
            request.timeunit    = m_initialRequest.timeunit;
            request.headerlines = m_initialRequest.headerlines;
            request.autoheader  = m_initialRequest.autoheader;
            request.datadir     = m_initialRequest.datadir;
            request.runsettings_dirpath = importFlist.at(i).filename;
            request.runsettings_dirpath.remove(importFlist.at(i).filename.split("/").last());
            request.runsettings_filename = request.runname;
            request.runsettings_filename.append("_settings.txt");
            request.channelPerFile = m_initialRequest.channelPerFile;
            request.flist.push_back(importFlist.at(i));
            request.noChannels = m_initialRequest.noChannels;
            request.noSignals = 0;

            QFile file(request.flist.first().filename);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QString line = file.readLine();
                file.close();

                QString delimiter = request.delimiter;
                if(delimiter == "tab")
                    delimiter = "\t";

                request.noChannels = line.split(delimiter).size() - 1;
            }

            QString settingsFile = QString(request.runsettings_dirpath).append(request.runsettings_filename);
            QFileInfo fi(settingsFile);
            if(fi.exists())
            {
                //request.userData.insert(30, true);
                //request.userData.insert(31, request.runsettings_filename);

                MRunSettings ms(settingsFile);

                if(!ms.liiSettingsFname().isEmpty())
                {
                    request.userData.insert(3, ms.liiSettingsFname());
                    LIISettings mls = IOBase::getLIISettings(request);
                }

                request.userData.insert(30, true);
                request.userData.insert(31, ms.runName());
            }

            requestList.push_back(request);
        }
    }
    emit checkFilesResult(requestList);
}


// ---------------
// PRIVATE HELPERS
// ---------------


/**
 * @brief IOcustom::scanMainDirectory Scans main directory for
 * subdirectories and creates import requests for each mrun found
 */
void IOcustom::scanMainDirectory()
{
    // THIS IS A VERY SIMPLE SOLUTION:
    // runs need to be in a subdirectory
    // custom import is executed for each subdirectory

    // take base dir as starting point
    QString dirname = m_initialRequest.datadir;

    //check if directory exists
    QDir datdir(dirname);
    if(!datdir.exists())
    {
        throw LIISimException("Data directory does not exist ("+dirname+")",ERR_IO);
    }

    // scan subdirectories
    datdir.setFilter(QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
    QDirIterator::IteratorFlag it_flag = QDirIterator::Subdirectories;

    // iterate through all subdirectories and push all files on stack
    QDirIterator it(datdir, it_flag);
    QStack<QString> filesStack ;

    while(it.hasNext()) {
       filesStack.push(it.next().remove(dirname));
    }

    // generate a list of import requests for each directory
    for(int i=0; i < filesStack.size(); i++)
    {
        // copy all properties from parent import request
        SignalIORequest irq = m_initialRequest;
        // modfify directory information
        irq.subdir = false;
        irq.autoName = true;
        irq.datadir = dirname + filesStack.at(i) + "/";

        // get settings file in each directory
        irq.runsettings_dirpath = irq.datadir;

        scanDirectory(irq);
    }
}


/**
 * @brief IOcustom::scanDirectory scans directory for single mrun
 */
void IOcustom::scanDirectory(SignalIORequest irq)
{    
    QString dirname = irq.datadir;

    logMessage("SignalManager: scan " + dirname);

    // update default data directory
    // Core::instance()->generalSettings->setDataDirectory(dirname);

    SignalFileInfoList importFlist;
    QList<int> importChannelIdList;
    QList<int> importSignalIdList;

    try
    {
        //check if directory exists
        QDir datdir(dirname);
        if(!datdir.exists())
        {
            throw LIISimException("Data directory does not exist ("+dirname+")",ERR_IO);
        }

        datdir.setFilter(QDir::Files | QDir::NoDot | QDir::NoDotDot);
        QDirIterator::IteratorFlag it_flag = QDirIterator::NoIteratorFlags;

        datdir.setNameFilters(QStringList("*" + irq.extension));

        // iterate through all subdirectories an push all files on stack
        QDirIterator it(datdir, it_flag);
        QStack<QString> filesStack ;

        while(it.hasNext()) {
           filesStack.push(it.next().remove(dirname));
        }

        QRegExp reg_exp;

        reg_exp.setPattern(
                     "^(" + irq.fname_txt_1 + ")"
                    + "(" + getFnamePatternRegExp(irq.fname_var_1) + ")"
                    + "(" + irq.fname_txt_2 + ")"
                    + "(" + getFnamePatternRegExp(irq.fname_var_2) + ")"
                    + "(" + irq.fname_txt_3 + ")"
                    + "(" + getFnamePatternRegExp(irq.fname_var_3) + ")"
                    + "(" + irq.fname_txt_4 + ")"
                    + "(\\" + irq.extension + ")"
                    "$");

        //DEBUG:
        //reg_exp.setPattern(irq.fname_txt_1);

        logMessage("Search pattern: "+reg_exp.pattern());

        // throws ERROR: check if channel is defined in file pattern
        if(irq.fname_var_1 != "<CHANNEL>"
                && irq.fname_var_2 != "<CHANNEL>"
                && irq.fname_var_3 != "<CHANNEL>"
                && irq.channelPerFile == true)
        {            
            throw LIISimException("<CHANNEL> not defined in file pattern: Import only single channel",ERR_IO);
        }

        // throws WARNING: import only one run
        if(irq.fname_var_1 == "<RUN>"
                && irq.fname_var_2 == "<RUN>"
                && irq.fname_var_3 == "<RUN>")
        {
           emit logMessage("<RUN> not defined in file pattern: Import only single Run", WARNING);
        }

        // throws WARNING: import only one signal
        if(irq.fname_var_1 == "<SIGNAL>"
                && irq.fname_var_2 == "<SIGNAL>"
                && irq.fname_var_3 == "<SIGNAL>")
        {
           emit logMessage("Signal not in file pattern defined: Import only single signal", WARNING);
        }


        // search for settings file
        for(int i=0; i<filesStack.size(); i++)
        {
            QString filename = filesStack.at(i);
            if(filename.endsWith("_settings.txt"))
            {
                emit logMessage("IOCustom: MRunDetails found: " + filename);
                break;
            }
        }


        // generate a list of import requests from filenames
        for(int i=0; i<filesStack.size(); i++)
        {
            int chId;
            int signalId;
            QString run_name;

            // only process if file matches the pattern
            if(reg_exp.indexIn(filesStack.at(i)) != -1)
            {
                QString filename = filesStack.at(i);
                logMessage("Found: "+filename);

//                qDebug() << reg_exp.cap(1);
//                qDebug() << reg_exp.cap(2); //var 1
//                qDebug() << reg_exp.cap(3);
//                qDebug() << reg_exp.cap(4); //var 1
//                qDebug() << reg_exp.cap(5);
//                qDebug() << reg_exp.cap(6); //var 1
//                qDebug() << reg_exp.cap(7);
//                qDebug() << reg_exp.cap(8);

                if(irq.channelPerFile == true)
                {
                    if(irq.fname_var_1 == "<CHANNEL>")
                        chId = reg_exp.cap(2).toInt();
                    else if(irq.fname_var_2 == "<CHANNEL>")
                        chId = reg_exp.cap(4).toInt();
                    else if(irq.fname_var_3 == "<CHANNEL>")
                        chId = reg_exp.cap(6).toInt();
                }

                if(irq.fname_var_1 == "<RUN>")
                    run_name = reg_exp.cap(2);
                else if(irq.fname_var_2 == "<RUN>")
                    run_name = reg_exp.cap(4);
                else if(irq.fname_var_3 == "<RUN>")
                    run_name = reg_exp.cap(6);
                else
                {
                   // import only one run
                    run_name = "CUSTOM_notset";
                }

                // save run_name also in general request
                if(irq.autoName)
                    irq.runname     = run_name;

                if(irq.fname_var_1 == "<SIGNAL>")
                    signalId = reg_exp.cap(2).toInt();
                else if(irq.fname_var_2 == "<SIGNAL>")
                    signalId = reg_exp.cap(4).toInt();
                else if(irq.fname_var_3 == "<SIGNAL>")
                    signalId = reg_exp.cap(6).toInt();
                else
                {
                   // import only one signal
                    signalId = 0;
                    emit logMessage("Signal not in file pattern defined: Import only single signal", WARNING);
                }

                SignalFileInfo finfo;
                finfo.itype     = CUSTOM;
                finfo.filename  = dirname + filename;
          //      finfo.sfname     = filename;
                finfo.delimiter = irq.delimiter; // assign general delimiter to each file
                finfo.decimal   = irq.decimal;   // assign general decimal to each file
                finfo.timeunit   = irq.timeunit;
                finfo.headerlines = irq.headerlines;
                finfo.autoheader = irq.autoheader;

                if(irq.channelPerFile)
                {
                    // put chId in consecutive lookup table
                    if(importChannelIdList.indexOf(chId) == -1)
                        importChannelIdList.append(chId);
                    finfo.channelId = chId;
                 //   finfo.noChannels = 1;
                }
                else
                {
                //    finfo.noChannels = irq.noChannels;
                }

                finfo.signalId = signalId;
                finfo.signalType = Signal::RAW;

                // add the file import information on the import requests file list
                importFlist.push_back(finfo);
            }
        }


        /***********************************
        *  now postprocess all found files if one channel per file:
        * -> reorder importFlist
        ***********************************/
        if(irq.channelPerFile)
        {
            // update m_generatedRequests with new signalIds for all channels
            // signalId in loadingtask needs to be consecutive

            int noChannels = 0; //  = irq.noChannels;


            // first int= channel, second int, signalId
            QMap <int, SignalFileInfo> importMap;


            noChannels = importChannelIdList.size();

            // add file into sortable map
            for(int i = 0; i < importFlist.size(); i++)
            {
                importMap.insertMulti(importFlist[i].signalId, importFlist[i]);
            }


            // check if all sorted(!!!) signals are complete
            // if channel is missing => skip signal
            foreach(SignalFileInfo file, importMap)
            {
                //qDebug() << importMap.values(file.signalId).size() << noChannels << file.signalId << file.filename;
                if(importMap.values(file.signalId).size() < noChannels)
                {
                    emit logMessage(QString::number(file.signalId) + " was removed from import list (Channels are not complete)");
                    importMap.remove(file.signalId);
                    importSignalIdList.removeAll(file.signalId);
                }
                else
                {
                    // put signalId in consecutive lookup table
                    if(importSignalIdList.indexOf(file.signalId) == -1)
                        importSignalIdList.append(file.signalId);
                }
            }

            importFlist = importMap.values();
            importMap.clear();

            // reorder all entries
            for(int i = 0; i < importFlist.size(); i++)
            {
                // replace signalId/channelId by consecutive ID
                importFlist[i].signalId = importSignalIdList.indexOf(importFlist[i].signalId);
                importFlist[i].channelId = importChannelIdList.indexOf(importFlist[i].channelId)+1;
                // add file into sortable map
                importMap.insertMulti(importFlist[i].signalId, importFlist[i]);
            }

            // flist is now sorted and consecutive
            importFlist.clear();
            importFlist = importMap.values();
            irq.noSignals = importMap.uniqueKeys().size();

            if(importFlist.isEmpty())
                emit logMessage("Skipping import (no datafiles!)", WARNING);

            // add new import request for this run
            m_generatedRequests.push_back(irq);
            m_generatedRequests.last().flist = importFlist;
        }
        else
        {
            m_generatedRequests.push_back(irq);
            m_generatedRequests.last().flist = importFlist;
        }

    }
    catch(LIISimException e)
    {
        emit logMessage(e.what(),e.type());
    }
}


/**
 * @brief IOcustom::loadFromCustomFile load file of CUSTOM format
 * @param mrun measurement run the data should be loaded to
 * @param fi informations about data file
 * @return number of read signals
 */
int IOcustom::loadFromCustomFile(MRun *mRun, SignalFileInfo fi)
{
    // for multi-column files: get number of channels from parent SignalIORequest
    int noChannels = 1;
    if(!m_initialRequest.channelPerFile)
        noChannels = m_initialRequest.noChannels;


    if(mRun == NULL)
        throw LIISimException(" IOcustom::loadFromCustom on mrun", ERR_NULL);

    QString filename = fi.filename;

    if(filename == "not set")
        return 0;

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // first line: col description
        QString line = file.readLine();

        // check file ...
        if(line.isEmpty())
        {
            emit importError(mRun->importRequest(), fi, "File empty");
            throw LIISimException("Custom-Import: empty file ("+filename+")", ERR_IO);
        }

        // split by any whitespace
        if(fi.delimiter == "tab")
            fi.delimiter = "\\s+";

        // set time unit modifier
        double tmod = 1.0;
        if(fi.timeunit == "ns") tmod = 1E-9;
        else if(fi.timeunit == "µs") tmod = 1E-6;
        else if(fi.timeunit == "ms") tmod = 1E-3;

        QStringList strings;
        double t1, t2, dt;
        int linecount = 0;
        QVector<Signal> sig_raw;
        QVector<Signal> sig_abs;

        sig_raw.resize(noChannels);
        sig_abs.resize(noChannels);

        // timebase
        QString str = "^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";

        // for each channel
        for(int i=0; i<noChannels; i++)
        {
            str = str + "["
                    + fi.delimiter
                    + "]{1}"
                    + "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?";
        }
        // end of line
        str = str + "\\s?$";

        //line should match the following RegExp
        QRegExp reg_exp = QRegExp(str);
        //qDebug() << reg_exp.pattern();

        int count_header = 0;

        while (!file.atEnd())
        {
            // read line by line (first line is read already above/check for empty file)
            if(linecount > 0)
                line = file.readLine();

            //skip header lines
            if(linecount < fi.headerlines)
            {
                linecount++;
                count_header++;
                continue;
            }

            // workaround
            line.replace("Infinity","0.0");

            // removes whitespace and replace with single whitespace
            line = line.simplified();

            // replace comma by dot
            if(fi.decimal == ",")
                line.replace(',',".");

            // auto detect header lines
            if(fi.autoheader == true)
            {
                if(reg_exp.indexIn(line) == -1)
                {                    
                    //emit logMessage(fi.sfname +" - "+line);
                    fi.headerlines++;
                    linecount++;
                    count_header++;
                    continue;
                }
            }

            // split line
            strings = line.split(QRegExp(fi.delimiter));

            if(strings.size() < noChannels+1)
                throw LIISimException("IOCustom: not enough cols ("+filename+")", ERR_IO);

            //do this for the first line after header
            if(linecount == fi.headerlines)
            {
                t1 = strings.at(0).toDouble()*tmod;

                for(int i=0; i<noChannels; i++)
                {
                    sig_raw[i].channelID = i+1;
                    sig_raw[i].type = Signal::RAW;


                    sig_abs[i].channelID = i+1;
                    sig_abs[i].type = Signal::ABS;

                }
            }
            else if(linecount == fi.headerlines+1)
            {
                t2 = strings.at(0).toDouble()*tmod;

                // adjust dt to 0.1 ns precision
                t1 = round(t1 * 1E10);
                t1 = t1 *1E-10;

                t2 = round(t2 * 1E10);
                t2 = t2 *1E-10;

                dt = round((t2-t1)*1E10);
                dt = dt*1E-10;

                //qDebug() << QString::number(t2) << " - " << QString::number(t1) << " = " << QString::number(dt);

                emit logMessage("IOCustom: Signal:" + filename + " calculated dt: " + QString::number(dt));

                for(int i=0; i<noChannels; i++)
                {
                    sig_raw[i].dt = dt;
                    sig_raw[i].start_time = t2;

                    sig_abs[i].dt = dt;
                    sig_abs[i].start_time = t2;
                }
            }

            for(int i=0; i<noChannels; i++)
            {
                sig_raw[i].data.push_back(strings.at(i+1).toDouble());
                // ABS = RAW
                sig_abs[i].data.push_back(strings.at(i+1).toDouble());
            }

            linecount++;
            strings.clear();
        }

        if(noChannels == 1)
        {
            // check if fi.signalId is consecutive!!! -> SignalManager

            MPoint* mp = mRun->getCreatePre(fi.signalId-1);

            if(fi.channelId == -1)
                    fi.channelId = 1;

            sig_raw[0].channelID = fi.channelId;
            sig_abs[0].channelID = fi.channelId;

            mp->setSignal(sig_raw[0],fi.channelId,Signal::RAW);
            mp->setSignal(sig_abs[0],fi.channelId,Signal::ABS);

            mp = mRun->getPost(fi.signalId-1);

            mp->setSignal(sig_raw[0],fi.channelId,Signal::RAW);
            mp->setSignal(sig_abs[0],fi.channelId,Signal::ABS);
        }
        else
        {
            int midx = mRun->sizeAllMpoints();
            MPoint* mp = mRun->getCreatePre(midx); //fi.signalId-1);

            //qDebug() << "noCa" << midx;

            for(int i=0; i<noChannels; i++)
            {
                sig_raw[i].channelID = i+1;
                sig_abs[i].channelID = i+1;

                mp->setSignal(sig_raw[i],i+1,Signal::RAW);
                mp->setSignal(sig_abs[i],i+1,Signal::ABS);
            }
            mp = mRun->getPost(fi.signalId-1);

            for(int i=0; i<noChannels; i++)
            {
                mp->setSignal(sig_raw[i],i+1,Signal::RAW);
                mp->setSignal(sig_abs[i],i+1,Signal::ABS);
            }
        }
        //qDebug() << (fi.shortFilename() +" - ID("+QString::number(fi.signalId) + "): Header: removed " + QString::number(count_header) + " lines");
        return 1; //number of imported signals
    }
    return 0;
}


/**
 * @brief IOcustom::getFnamePatternRegExp translates file name pattern to regular expression
 * @param var filename pattern (from ImportDialog)
 * @return
 */
QString IOcustom::getFnamePatternRegExp(QString var)
{
    QString str;
    if(var == "<CHANNEL>")       str = "\\d";  //one digit number
    else if(var == "<SIGNAL>")   str = "\\d+"; //any number
    else if(var == "<RUN>")      str = ".*";   //any characters
    else if(var == "<PMT_GAIN>") str = "";
    else if(var == "<FLUENCE>")  str = "";
    else if(var == "<INTEGER>")  str = "\\d+"; //any number
    else if(var == "<STRING>")   str = ".*";   //any characters
    else str = "";

    return str;
}
