#include "iocsv.h"

#include <QDir>
#include <QDirIterator>
#include <QStack>
#include <QStringList>
#include <QThreadPool>
#include "../core.h"
#include <iostream>
#include "../signal/processing/processingchain.h"
#include "../settings/mrunsettings.h"

IOcsv::IOcsv(QObject *parent) :  IOBase(parent)
{
    // set parallelization mode to one file per worker thread
    p_mode = PM_PERFILE;
}


// ------------------------
// IMPLEMENTATION OF IOBASE
// ------------------------


/**
 * @brief IOcsv::setupImport Called by IOBase::importSignals() concurrently to GUI-thread.
 * Performs directory scan for .csv data files (see IOcsv::scanDirectory())
 */
void IOcsv::setupImport()
{
    try
    {
        if(m_initialRequest.itype == CSV_SCAN)
            scanDirectory();
        else if(m_initialRequest.itype == CSV)
            m_generatedRequests.append(m_initialRequest);
        else
            throw LIISimException("IOcsv: cannot handle SignalIORequest: wrong IOtype!");

        if(m_generatedRequests.size() > QThreadPool::globalInstance()->maxThreadCount())
            p_mode = PM_PERMRUN;
    }
    catch(LIISimException e)
    {
        logMessage(e.what(),e.type());
    }


    // emit the setup-finished signal
    // This call is required for IOBase::onImportSetupFinished() to execute
    emit importSetupFinished();
}


/**
 * @brief IOcsv::importStep concurrently called by IOBase::onImportSetupFinished().
 * loades a list of .csv files to a MRun
 * @param mrun measurement run object
 * @param fileInfos list of file informations
 */
void IOcsv::importStep(MRun* mrun, SignalFileInfoList fileInfos)
{
    int noSignals = 0;
    for(int i = 0; i < fileInfos.size(); i++)
    {
        // check if abort flag is set
        if(abort_flag)
        {
            importStepFinished(0);
            return;
        }

        try
        {
            SignalFileInfo fi = fileInfos.at(i);
            noSignals = loadFile(mrun,fi);
        }
        catch(LIISimException e)
        {
            logMessage(e.what(),e.type());
        }
    }
    emit importSuccess(mrun->importRequest(), fileInfos);
    emit importStepFinished(noSignals);
}


void IOcsv::checkFiles()
{
    scanDirectory();

    //return m_generatedRequests;
    emit checkFilesResult(m_generatedRequests);
}


/**
 * @brief IOcsv::exportSignals !!! TODO PARALLELIZE !!!
 * @param rq
 */
void IOcsv::exportImplementation(const SignalIORequest &rq)
{
    if(rq.itype != CSV)
    {
        logMessage("CSV-Export: IO-Request is not of type CSV! Export canceled.",INFO);
        return;
    }
    m_timer.start();

    try
    {
        //TODO:
        // - replace this by checkbox in ExportDialog: "export all"
        // - rename Mrun is useless with this code: disable if export all is activated

        // get selected mruns from SignalIORequest userdata
        QList<MRun*> mrun_list;
        QList<QVariant> runids = rq.userData.value(8).toList();
        for(int i = 0; i < runids.size(); i++)
        {
            MRun* mrun = Core::instance()->dataModel()->mrun(runids[i].toInt());
            if(mrun)
                mrun_list << mrun;
        }

        // export all mruns in list
        // TODO: parallelize loop
        for(int j=0;j< mrun_list.size();j++)
        {
            // check if operation has been aborted
            if(IOBase::abort_flag)
            {
                emit exportImplementationFinished(m_timer.elapsed()/1000.0);
                return;
            }

            MRun* mrun = mrun_list[j];
            QString fullFname;

            // redefine new filename only for single export
            fullFname = rq.datadir + mrun->getName();

            int noCh = mrun->getNoChannels(Signal::RAW);

            MRunSettings ms(mrun);
            ms.save(rq.datadir);

            // create raw,absolute signals for each channel !!!
            for(int i=0; i < noCh; i++)
            {
                int chID =i+1;
                QString fRaw = fullFname+".Signal.Volts";        // !!!
                QString fAbs = fullFname+".Absolute.Intensity";  // !!!
                QString ch;
                ch.sprintf(".Ch%d.csv",chID);
                fRaw += ch;
                fAbs += ch;

                QString filenameStdevRaw = fullFname;
                filenameStdevRaw.append(QString(".StandardDeviation.Raw.Ch%0.csv").arg(chID));

                QString filenameStdevAbs = fullFname;
                filenameStdevAbs.append(QString(".StandardDeviation.Abs.Ch%0.csv").arg(chID));

                if(e_flag_raw)
                    writeToCSV(fRaw,mrun,Signal::RAW,chID);

                if(e_flag_abs)
                    writeToCSV(fAbs,mrun,Signal::ABS,chID);

                if(e_flag_raw_stdev)
                    writeStdevToCSV(filenameStdevRaw, mrun, Signal::RAW, chID);

                if(e_flag_abs_stdev)
                    writeStdevToCSV(filenameStdevAbs, mrun, Signal::ABS, chID);
            }

            emit progressUpdate( (float)j/(float) mrun_list.size());
         }
    }
    catch(LIISimException e)
    {
        emit logMessage(e.what(),e.type());
    }
    emit exportImplementationFinished(m_timer.elapsed()/1000.0);
}


// ---------------
// PRIVATE HELPERS
// ---------------


/**
 * @brief IOcsv::scanDirectory helper method, scans directory of initial IO-Request for
 * CSV files. Filename composition is important here!
 */
void IOcsv::scanDirectory()
{
    QString dirname = m_initialRequest.datadir;
    int noChannels  = m_initialRequest.noChannels;
    bool loadRawData = m_initialRequest.userData.value(6,true).toBool();
    bool loadAbsData = m_initialRequest.userData.value(7,true).toBool();

    logMessage( "IOcsv: scan " + dirname,DEBUG);

    // TODO update default data directory
    // Core::instance()->generalSettings->setDataDirectory(dirname);

        QStringList nameFilters;
        nameFilters << "*.csv" << "*_settings.txt";
        QDir datdir(dirname);

        if(!datdir.exists())
        {
            throw LIISimException("Data directory does not exist ("+dirname+")",ERR_IO);
        }

        QDirIterator::IteratorFlag it_flag;

        // set filters on directory
        if(m_initialRequest.subdir == true)
        {
            logMessage( "Scan subdirectories",DEBUG);
            datdir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDot | QDir::NoDotDot);
            it_flag = QDirIterator::Subdirectories;
        }
        else
        {
            datdir.setFilter(QDir::Files | QDir::NoDot | QDir::NoDotDot);
            it_flag = QDirIterator::NoIteratorFlags;
        }

        datdir.setNameFilters(nameFilters);

        QStack<QString> filesStack;

        // iterate through dir and push all ".csv" files on stack
        QDirIterator it(datdir, it_flag);

        while(it.hasNext()) {
           filesStack.push(it.next().remove(dirname));
        }

        // generate a list of import requests from filenames
        for(int i=0; i < filesStack.size(); i++)
        {
            QString fname = filesStack.at(i);

            bool isSettingsFile = false;
            QString frunname;
            QStringList sl = fname.split(".");

            if(fname.endsWith("_settings.txt"))
            {
                frunname = fname.remove("_settings.txt");
                isSettingsFile = true;
                emit logMessage("CSV-Import: MRunDetails found: " + frunname);
            }
            else
                frunname = sl[0];

            if(sl.size() != 5 && !isSettingsFile) // !!!
            {
                QString msg = "CSV-Import: skipped file, invalid filename composition:"+filesStack.at(i);
                logMessage(msg,WARNING);
                continue;
            }

            // check if run with given name is already loaded!
//            if(Core::instance()->dataModel()->containsMRun(frunname))
//            {
//                QString msg = "CSV-Import: run "+frunname+" already loaded!";
//                logMessage(msg,WARNING);
//                continue;
//            }

            // check if an Import request for the given MRun has already been created
            int idx = -1;
            for(int k=0; k< m_generatedRequests.size(); k++)
            {
                if(m_generatedRequests.at(k).runname == frunname)
                {
                    idx = k;
                    break;
                }
            }

            if(idx == -1) // add new import request if there is no existing import request for the file's runname
            {
                SignalIORequest nirq;
                nirq.runname = frunname;
                nirq.datadir = "";
                nirq.itype   = CSV;
                nirq.group_id = m_initialRequest.group_id;
                nirq.delimiter   = m_initialRequest.delimiter; // standard value
                nirq.decimal     = m_initialRequest.decimal; // standard value
                nirq.userData.insert(3,m_initialRequest.userData.value(3));
                nirq.userData.insert(18,m_initialRequest.userData.value(18));
                nirq.noChannels = noChannels;
                nirq.noSignals = 0; // workaround for SignalManager::loadImportRequest()
                m_generatedRequests.push_back(nirq);
                idx = m_generatedRequests.size()-1;
            }


            if(isSettingsFile)
            {
                // if the current file is a settings file, save the
                // settings directory information to irq
                m_generatedRequests[idx].runsettings_dirpath = dirname;

                logMessage("CSV-Import: MRunDetails loaded: " + filesStack[i]);

                MRunSettings ms(dirname+filesStack[i]);

                // update some fields of import request based on mrun settings!

                // update liisettings filename
                if(!ms.liiSettingsFname().isEmpty())
                {
                    m_generatedRequests[idx].userData.insert(3,ms.liiSettingsFname());
                    LIISettings mls = IOBase::getLIISettings(m_generatedRequests[idx]);

                    // update number of channels
                    int new_noCh = mls.channels.size();
                    if(new_noCh > 0)
                    {
                        m_generatedRequests[idx].noChannels = new_noCh;
                        noChannels = new_noCh;
                    }
                }

                m_generatedRequests[idx].userData.insert(30, true); //Flag for 'MRunSettings-file found'
                m_generatedRequests[idx].userData.insert(31, ms.runName());
            }
            else
            {
                SignalFileInfo finfo;
                finfo.filename  = dirname+filesStack.at(i);
                finfo.itype     = CSV;
                finfo.delimiter = m_initialRequest.delimiter; // assign general delimiter to each file
                finfo.decimal   = m_initialRequest.decimal;   // assign general decimal to each file

                QFile file(finfo.filename);
                if(file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QString line = file.readLine();
                    while(!file.atEnd())
                    {
                        line = file.readLine();
                        finfo.fileLineCount++;
                    }
                    file.close();
                }

                // determine if signal is raw or absolute (use 2nd entry from filenames stringlist!)
                if(sl.at(1)== "Absolute")
                    finfo.signalType = Signal::ABS;
                else if(sl.at(1) == "Signal")
                    finfo.signalType = Signal::RAW;
                else if(sl.at(1) == "StandardDeviation")
                {
                    finfo.stdevFile = true;
                    if(sl.at(1) == "Raw")
                        finfo.signalType = Signal::RAW;
                    else if(sl.at(1) == "Abs")
                        finfo.signalType = Signal::ABS;
                }

                // use third item for channel info
                finfo.channelId = sl[3].remove("Ch").toInt();

                // add file to stack if the requested signal type should be imported
                if( (finfo.signalType == Signal::RAW && loadRawData) ||
                    (finfo.signalType == Signal::ABS && loadAbsData) )
                {
                    // add the file import information on the import requests file list
                    m_generatedRequests[idx].flist.push_back(finfo);
                }
            }
        }
        if(m_generatedRequests.isEmpty())
           logMessage("Skipping import (no datafiles!)", WARNING);
}


/**
 * @brief IOcsv::loadFile helper method, loads a CSV file
 * @param mRun measurement run object, where the signal data should be loaded to.
 * @param fi signal file informations
 * @return number of read signals
 */
int IOcsv::loadFile(MRun *mRun, const SignalFileInfo &fi)
{
    if(mRun == NULL)
        throw LIISimException("CSV-Import: no MRun", ERR_NULL);

    QString filename = fi.filename;
    int chID = fi.channelId;
    Signal::SType stype = fi.signalType;
    bool copyRawToAbs = m_initialRequest.userData.value(18,false).toBool();

    if(filename == "not set")
        return 0;

    if(!filename.endsWith(".csv"))
    {
        emit importError(mRun->importRequest(), fi, "Not a *.csv file");
        logMessage("CSV-Import: file is no .csv file ("+filename+")", ERR_IO);
        return 0;
    }


    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // first line: time data
        QString line = file.readLine();

        // only replace comma by dot if specified by user
        if(fi.decimal == ",")
            line.replace(',',tr("."));     // !!! not that nice, slow

        // check file ...
        if(line.isEmpty())
        {
            emit importError(mRun->importRequest(), fi, "Empty file");
            logMessage("CSV-Import: empty file ("+filename+")", ERR_IO);
            return 0;
        }

        QStringList strings = line.split(fi.delimiter);  // !!! not that nice


        if(strings.size()<2 )
        {
            emit importError(mRun->importRequest(), fi, "Not enough data");
            logMessage("CSV-Import: not enough data ("+filename+")", ERR_IO);
            return 0;
        }

        // determine delta time
        double t1 = strings.at(0).toDouble();
        double t2 = strings.at(1).toDouble();
        double dt = t2-t1;

        strings.clear();

        int linecount = 0;
        while (!file.atEnd())
        {
            line = file.readLine();

            // only replace comma by dot if specified by user
            if(fi.decimal == ",")
                line.replace(',',tr("."));   // !!! not that nice

            strings = line.split(fi.delimiter);   // !!! not that nice
          //  qDebug() << strings;
            MPoint* mp = mRun->getCreatePre(linecount);
            //we use the default signal, since we don't know if the stdev signal
            //will be loaded before or after the normal signal data
            Signal sig = mp->getSignal(chID, stype);

            if(!fi.stdevFile)
            {
                //normal signal data, so we can set some base info
                sig.channelID = chID;
                sig.type = stype;
                sig.start_time = t1;
                sig.dt = dt;
            }

            // for raw signals: remove first entry scince it contains no signal data !!!!
            // TODO: add this option to import settings !
            if(stype == Signal::RAW && !strings.isEmpty())
            {
                strings.removeAt(0);
            }

            QVector<double> data;
            for(int i=0;i<strings.size()-1;i++)
            {
                data.push_back(strings.at(i).toDouble());
            }

            if(fi.stdevFile)
                sig.stdev = data;
            else
                sig.data = data;

            mp->setSignal(sig,chID,stype);

            if(copyRawToAbs && stype == Signal::RAW)
            {
                Signal cpy = sig;
                cpy.type = Signal::ABS;
                mp->setSignal(cpy,chID,Signal::ABS);
            }

            mp = mRun->getPost(linecount);
            mp->setSignal(sig,chID,stype);

            if(copyRawToAbs && stype == Signal::RAW)
            {
                Signal cpy = sig;
                cpy.type = Signal::ABS;
                mp->setSignal(cpy,chID,Signal::ABS);
            }

            //Core::instance()->asyncMsg(QString("%0 %1 %2 %3 %4").arg(fi.filename).arg(fi.stdevFile).arg(sig.data.size()).arg(sig.stdev.size()).arg(copyRawToAbs));

            linecount++;
            strings.clear();
        }

        file.close();
        return linecount;
    }
    else
    {
        emit importError(mRun->importRequest(), fi, "Cannot open file");
        logMessage("CSV-Import: cannot open file ("+filename+")", ERR_IO);
        return 0;
    }
}


/**
 * @brief IOcsv::writeToCSV helper method, writes a CSV file
 * @param fname full filename
 * @param mru mruns which should be saved
 * @param stype signal type which should be saved
 * @param ChID channel id which should be saved
 */
void IOcsv::writeToCSV(QString fname, MRun *mru, Signal::SType stype, int ChID)
{
    if(mru == NULL)
        throw LIISimException("CSV-Export: invalid mrun", ERR_NULL);

    QFile file(fname);


    bool postproc = true;
    if(stype == Signal::RAW)
        postproc = e_flag_raw_postproc;
    else if(stype == Signal::ABS)
        postproc = e_flag_abs_postproc;

    // check if Processing Chain contains an active Multi Signal Average Plugin
    bool msa = mru->getProcessingChain(stype)->containsActiveMSA();

    // create subdirectories if necesssary
    QFileInfo fi(file);
    if(!fi.dir().exists())
        QDir().mkpath(fi.dir().absolutePath());


    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        //QLocale locale = QLocale(QLocale::German,QLocale::Germany);
        //out.setLocale(locale);

        // TODO: get decimal mark setting from program settings
        // now: use dot (.)

        // write time
        int noMpoints = mru->sizeAllMpoints();

        // if we are exporting processed signals and
        // if the processing chain contains an active MSA Plugin
        // -> write first signal only!
        if(postproc && msa)
            noMpoints = 1;

        if(noMpoints == 0)
            throw LIISimException("CSV-Export canceled: no signal data, mrun: "+mru->getName(), ERR_IO);

        MPoint *mp;

        if(postproc)
            mp = mru->getPost(0);
        else
            mp = mru->getPre(0);

        Signal s = mp->getSignal(ChID, stype);

        // first row is time axis
        int noDataPointsPerSignal = s.data.size();
        double t0 = s.start_time;
        double dt = s.dt;

        //qDebug() << QString::number(s.start_time);
        emit logMessage("CSV-Export: dt"+QString::number(dt), ERR_IO);

        for(int i=0; i< noDataPointsPerSignal;i++)
        {
            out << t0 + i*dt <<";";
        }
        out << "\n";

        // write signal data for each mpoint (one signal per row)
        for(int i=0; i<noMpoints;i++)
        {
            if(postproc)
                mp = mru->getPost(i);
            else
                mp = mru->getPre(i);

            s = mp->getSignal(ChID,stype);

           //   qDebug() << ChID << stype << s.data.size();

            //if(s.data.size() != noDataPointsPerSignal)
            //    throw LIISimException("CSV-Export canceled: varying signal data size!, mrun: "+mru->getName(), ERR_IO);
            //if(s.start_time != t0)
            //    throw LIISimException("CSV-Export canceled: varying signal start time size!, mrun: "+mru->getName(), ERR_IO);

            //TODO:
            if(s.dt != dt)
                qWarning() << "Error: dt is varying between Signals (idx:" << i << "): " << dt << " for this signal: " <<s.dt;
                //throw LIISimException("CSV-Export canceled: varying signal time step!, mrun: "+mru->getName(), ERR_IO);

            for(int i=0; i< noDataPointsPerSignal;i++)
            {
                out <<s.data.at(i) <<";";
            }
            out << "\n";
        }

        logMessage("CSV-Export: wrote "+fname);
        file.close();
    }
    else
    {
        throw LIISimException("CSV-Export: cannot open file ("+fname+")", ERR_IO);
    }
}


/**
 * @brief IOcsv::writeStdevToCSV Writes a *.csv file containing the standard
 * deviation data
 * @param fname Filename where the data should be written (with full path)
 * @param run Data source
 * @param stype Source signal type
 * @param channelID Source channel ID
 */
void IOcsv::writeStdevToCSV(QString fname, MRun *run, Signal::SType stype, int channelID)
{
    if(run == NULL)
        throw LIISimException("CSV-Export: invalid mrun", ERR_NULL);

    QFile file(fname);

    // create subdirectories if necesssary
    QFileInfo fi(file);
    if(!fi.dir().exists())
        QDir().mkpath(fi.dir().absolutePath());

    Signal signal = run->getPre(0)->getSignal(channelID, Signal::RAW);

    if(signal.stdev.isEmpty())
    {
        QString signaltype;
        if(stype == Signal::RAW)
            signaltype = "raw";
        else if(stype == Signal::ABS)
            signaltype = "absolute";

        logMessage(QString("CSV-Export: No standard deviation data for type %0 available. Standard deviation export skipped for %1.")
                   .arg(signaltype).arg(run->getName()), WARNING);
        return;
    }

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);

        int noMpoints = run->sizeAllMpoints();

        if(noMpoints == 0)
            throw LIISimException("CSV-Export canceled: no signal data, mrun: "+run->getName(), ERR_IO);

        // first row is time axis
        int noDataPointsPerSignal = signal.data.size();
        double t0 = signal.start_time;
        double dt = signal.dt;

        for(int i=0; i< noDataPointsPerSignal;i++)
        {
            out << t0 + i*dt << ";";
        }
        out << "\n";

        for(int i = 0; i < noMpoints; i++)
        {
            MPoint *mp = run->getPre(i);

            signal = mp->getSignal(channelID, stype);

            if(signal.dt != dt)
                qWarning() << "Error: dt is varying between Signals (idx:" << i << "): " << dt << " for this signal: " <<signal.dt;

            for(int i=0; i< noDataPointsPerSignal;i++)
            {
                out << signal.stdev.at(i) << ";";
            }
            out << "\n";
        }

        logMessage("CSV-Export (stdev): wrote "+fname);
        file.close();
    }
    else
    {
        throw LIISimException("CSV-Export (stdev): cannot open file ("+fname+")", ERR_IO);
    }
}
