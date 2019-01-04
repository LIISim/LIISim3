#include "ioxml.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDateTime>
#include <QDir>

#include "../core.h"
#include "iocsv.h"
#include "iocustom.h"
#include "../signal/mrungroup.h"
#include "../signal/processing/processingchain.h"
#include "../signal/processing/pluginfactory.h"
#include "../signal/processing/processingpluginconnector.h"
#include "../signal/processing/processingplugin.h"
#include "../signal/processing/plugins/temperaturecalculator.h"
#include "../signal/processing/plugins/dummyplugin.h"
#include "../calculations/fit/fitrun.h"
#include "../settings/mrunsettings.h"

/**
 * @brief IOxml::IOxml Constructor
 * @param parent QObject
 */
IOxml::IOxml(QObject *parent) :   IOBase(parent)
{
    // for each run we need one importStep call here
    p_mode = PM_PERMRUN;

    connect(this, SIGNAL(m_MrunLoadingStepFinished(MRun*)),
            this, SLOT(m_onMrunLoadingStepFinished(MRun*)),
            Qt::QueuedConnection);

    connect(this, SIGNAL(m_importSetupFinished()),
            this, SLOT(m_onImportSetupFinished()),
            Qt::QueuedConnection);

    mProgressCounter = 0;
    mRelativePaths = false;
    mXMLfname = "";

}


/**
 * @brief IOxml::loadInitSession Static helper method, loads
 * the initial session (initSession.xml). This is done at program startup
 * (see main.cpp) and allows to reconstruct the program state on next startup.
 */
void IOxml::loadInitSession()
{
    if(!Core::instance()->settingsFound())
    {
        MESSAGE("Since the application settings could not be found, session import is skipped. This is normal if you run LIISim for the first time and can be ignored.", INFO);
        return;
    }
    // check if data should be loaded at startup
    if(Core::instance()->generalSettings->loadDataAtStartup())
    {
        // get filename of default session from general settings
        QString fname = Core::instance()->generalSettings->initDataStateFilePath();
        MESSAGE(QString("XML-Session Import: loading default session file '%0' ...").arg(fname),INFO);

        // create an IO-Request
        SignalIORequest rq;
        rq.userData.insert(0,fname);
        rq.itype = XML;

        // pass IO-Request to SignalManager
        Core::instance()->getSignalManager()->importSignalsManager(rq);
    }
}


/**
 * @brief IOxml::saveInitSession Satic helper method, saves
 * the current program state including all runs, groups and processing
 * plugins to the initSession.xml file. This is done when the program
 * execution is finished (see main.cpp) and allows to reconstruct
 * the program state on next startup.
 */
void IOxml::saveInitSession()
{
    // get filename of default session from general settings
    QString fname = Core::instance()->generalSettings->initDataStateFilePath();
    MESSAGE(QString("XML-Session Import: save default session file '%0' ...").arg(fname),INFO);

    // create an IO-Request for session export
    SignalIORequest rq;
    rq.itype = XML;
    rq.userData.insert(0,Core::instance()->generalSettings->initDataStateFilePath());

    // add all runs to session
    QList<MRun*> allRuns = Core::instance()->dataModel()->mrunList();
    QList<QVariant> exportRunIDs;
    for(int i = 0; i < allRuns.size(); i++)
        exportRunIDs << allRuns[i]->id();

    rq.userData.insert(2,true);
    rq.userData.insert(8,exportRunIDs);
    rq.userData.insert(9,false);  // include modeling settings within session
    rq.userData.insert(10,false); // include gui settings within session
    rq.userData.insert(11,false); // include general settings within session
    rq.userData.insert(12,true);  // include data within session

    // pass IO-Request to Signal Manager
    Core::instance()->getSignalManager()->exportSignalsManager(rq);
}


/**
 * @brief IOxml::setupImport Implements IOBase::setupImport(), contains
 * concurrent code
 */
void IOxml::setupImport()
{
    // no concurrent operations needed!
    // notify helper method, which runs is executed in App-Thread
    emit m_importSetupFinished();
}

/**
 * @brief IOxml::m_onImportSetupFinished helpermethod, called right after "setupImort" has finished.
 * Used for Object creation in app-thread!
 */
void IOxml::m_onImportSetupFinished()
{
    // read file in app thread to ease object creation !!!
    try
    {
        mXMLfname = m_initialRequest.userData.value(0,"").toString();

        QFile xfile(mXMLfname);
        if(!xfile.open(QIODevice::ReadOnly) )
        {
            throw LIISimException( QString("IOxml: cannot read " )
                 .append(mXMLfname),ERR_IO);
        }

        initGlobalPPCs.clear();

        int dataMode = m_initialRequest.userData.value(19,0).toInt();
        // delete current runs and proc steps
        if(dataMode == 0)
        {
            deleteAllRuns();
        }
        // add new runs, use existing proc steps
        else if(dataMode == 1)
        {
            // find existing ProcessingPluginConnectors
            initGlobalPPCs = findGlobalPPCs();
        }
        // add new runs, load existing proc steps from file
        else if(dataMode == 2)
        {
            deleteAllProcessingSteps();
        }



        // open xml stream, iterate over all xml tokens
        QXmlStreamReader r(&xfile);
        while(!r.atEnd() && !r.hasError())
        {
            QXmlStreamReader::TokenType token = r.readNext();

            if(r.tokenType() == QXmlStreamReader::Invalid)
            {
                //qDebug() << "INVALID";
                break;
            }

            if(token == QXmlStreamReader::StartDocument)
                continue;

            if(token == QXmlStreamReader::StartElement)
            {

                if(r.name() == "LIISim")
                {
                    mRelativePaths = r.attributes().value("relativePaths").toInt();
                    QString material = r.attributes().value("materialSpec").toString();
                    if(!material.isEmpty())
                        Core::instance()->modelingSettings->setMaterialSpec(material);
                }

                // read groups (inluding MRuns, Processing Chains etc
                if(r.name() == "Group")
                {
                    readGroup(r);
                }

                // !!! redundant, keep currently for backwards compatibility !!!
                if(r.name() == "MRun")
                {

                    while( !(r.name() == "MRun" &&
                             r.tokenType() == QXmlStreamReader::EndElement))
                    {

                        if(r.tokenType() == QXmlStreamReader::StartElement)
                        {
                            if(r.name() == "SignalIORequest")
                            {
                                SignalIORequest irq = SignalIORequest::fromXML(r,mXMLfname);
                                m_generatedRequests.append(irq);
                            }
                        }
                        r.readNext();
                    }
                }

                // Read Fit runs
                if(r.name() == "FitRun")
                {
                    FitRun* fr = new FitRun();
                    fr->readFromXml(r);
                }
            }
        }
    }
    catch(LIISimException e)
    {
        logMessage(e.what(),e.type());
    }

    // !!! redundant, keep currently for backwards compatibility !!!
    // assign groups
    for(int i =0; i < m_generatedRequests.size(); i++)
    {
        // if group id is negative no matching group existed at
        // the moment we parsed the file
        if(m_generatedRequests[i].group_id == -1)
        {
            QString gname =m_generatedRequests[i].userData.value(5,"").toString();

            if(gname.isEmpty()) continue;

            // check if group has been created in meantime
            QList<MRunGroup*> groups = Core::instance()->dataModel()->groupList();
            for(int j = 0; j < groups.size(); j++)
                if(groups[j]->title() == gname)
                {
                    m_generatedRequests[i].group_id = groups[j]->id();
                    qDebug()<< "IOxml m_onImportSetupFinished group already found" << gname;
                    break;
                }

            // if id is still negative ... create a new group
            if(m_generatedRequests[i].group_id == -1)
            {
                MRunGroup* g = new MRunGroup(gname);
                m_generatedRequests[i].group_id = g->id();
            }
        }
    }

    // active plugin connections
    QList<ProcessingPluginConnector*> ppcs = readPlugConnectorsMap.values();
    for(int i = 0; i < ppcs.size(); i++)
        ppcs[i]->setActive(true);

    readPlugConnectorsMap.clear();
    emit importSetupFinished();
}


/**
 * @brief IOxml::importStep implements IOBase::importStep(), processes all
 * SignalIORequests parsed from the XML-File
 * (IOxml::m_onImportSetupFinished()).
 * Initializes file format specific Import-Operations.
 * @param mrun MRun object generated during XML-File parsing
 * @param fileInfos parameter can be ignored! Uses SignalIORequest from MRun
 * to load signal data instead
 */
void IOxml::importStep(MRun *mrun, SignalFileInfoList fileInfos)
{
    try
    {
        // get SignalIORequest parsed from xml file from run
        SignalIORequest irq = mrun->importRequest();
        if(irq.itype == CSV)
        {
            IOcsv* io = new IOcsv;
            io->setMRunToProcess(mrun);
            io->setAutoDelete(true);
            io->setMuteLogging(true);
            io->setEnabledConcurrency(false);
            io->importSignals(irq);
        }
        else if(irq.itype == CUSTOM)
        {
            IOcustom* io = new IOcustom;
            io->setMRunToProcess(mrun);
            io->setAutoDelete(true);
            io->setMuteLogging(true);
            io->setEnabledConcurrency(false);
            io->importSignals(irq);
        }
    }
    catch(LIISimException e)
    {
        logMessage(e.what(),e.type());
    }
    emit m_MrunLoadingStepFinished(mrun);
}



/**
 * @brief IOxml::m_onMrunLoadingStepFinished is an intermediate step
 * during the xml import. This slot is called right after the "m_MrunLoadingStepFinished"-signal
 * is emitted by the importStep method.
 * @param mrun
 */
void IOxml::m_onMrunLoadingStepFinished(MRun *mrun)
{
    mProgressCounter++;
    emit importStepFinished(mrun->sizeAllMpoints()*mrun->getNoChannels(Signal::RAW));
}


/**
 * @brief IOxml::exportImplementation Implementation of a XML-File Export.
 * Saves the current program session to a XML-File.
 * The resulting XML-File does not contain the MRuns's signal data!
 * Instead it stores SignalIORequests which define how the SignalData can be
 * retrieved.
 * @param rq SignalIORequest of XML-Session
 */
void IOxml::exportImplementation(const SignalIORequest &rq)
{
    m_timer.start();
    try
    {
        mXMLfname = rq.userData.value(0,"").toString();

        mRelativePaths = rq.userData.value(2,false).toBool();

        // get information about what should be save from request
        bool saveModelingSettings = rq.userData.value(9,false).toBool();;
        bool saveGuiSettings =      rq.userData.value(10,false).toBool();;
        bool saveGeneralSettings =  rq.userData.value(11,false).toBool();;
        bool saveData =             rq.userData.value(12,false).toBool();

        // get list of runs which should be exported (checked by user in SessionSaveDialog)
        QList<MRun*> checkedRuns;
        QList<QVariant> runids = rq.userData.value(8).toList();
        for(int i = 0; i < runids.size(); i++)
        {
            MRun* mrun = Core::instance()->dataModel()->mrun(runids[i].toInt());
            if(mrun)
                checkedRuns << mrun;
        }

        QString xmlDir = "";
        if(mRelativePaths)
            xmlDir = mXMLfname;



        // todo: remove this!!
        QString pchainDirname = Core::instance()->generalSettings->procChainDirectory();
        if(!pchainDirname.endsWith("/"))
            pchainDirname.append("/");

        // save those autogenerated processing chaings to "tmp" subdirectory
        // within processing chin directory
        pchainDirname.append("tmp/");


        QFile xfile(mXMLfname);
        if(!xfile.open(QIODevice::WriteOnly) )
        {
            throw LIISimException( QString("IOxml: cannot write " )
                 .append(mXMLfname),ERR_IO);

        }

        QXmlStreamWriter w(&xfile);
        w.setAutoFormatting(true);
        w.writeStartDocument("1.0");
        w.writeComment("LIISim startup data");
        w.writeEndDocument();
        w.writeStartElement("LIISim");
        w.writeAttribute("relativePaths",QString("%0").arg(mRelativePaths));
        w.writeAttribute("materialSpec", Core::instance()->modelingSettings->materialSpec().filename);

        if(saveModelingSettings)
            Core::instance()->modelingSettings->writeToXML(w);
        if(saveGuiSettings)
            Core::instance()->guiSettings->writeToXML(w);
        if(saveGeneralSettings)
            Core::instance()->generalSettings->writeToXML(w);

        if(saveData)
        {
            DataItem* rootItem = Core::instance()->dataModel()->rootItem();
            for(int i = 0; i < rootItem->childCount(); i++)
            {
                MRunGroup* gr = dynamic_cast<MRunGroup*>(rootItem->childAt(i));
                if(gr)
                    writeGroup(w,gr,checkedRuns);
            }
        }

        // TODO: SAVE FITRUNS HERE

        w.writeEndElement(); // LIISim
    }
    catch(LIISimException e)
    {
        logMessage(e.what(),e.type());
    }
    emit exportImplementationFinished((double)m_timer.elapsed()/1000.0);
}


void IOxml::checkFiles()
{
    QList<SignalIORequest> nothing;
    emit checkFilesResult(nothing);
}


/**
 * @brief IOxml::writeGroup Helper method for XML-Session Export, writes 'Group' token
 * @param w QXmlStreamWriter
 * @param g MRunGroup
 */
void IOxml::writeGroup(QXmlStreamWriter &w, MRunGroup* g,QList<MRun*>& checkedRuns )
{
    // get checked runs within group 'g'
    QList<MRun*> checkedRunsInGroup;

    QList<MRun*> runsInGroup = g->mruns();
    for(int i = 0; i < runsInGroup.size(); i++)
        if(checkedRuns.contains(  runsInGroup[i]))
            checkedRunsInGroup << runsInGroup[i];

    // do not save group if it would be empty anyway
    if(checkedRunsInGroup.isEmpty())
        return;

    w.writeStartElement("Group");
    w.writeAttribute("name",g->title());

    // iterate over all MRuns in group
    for(int i = 0;i < checkedRunsInGroup.size(); i++)
    {
        // save run token
        writeMRun(w,checkedRunsInGroup[i]);
    }

    w.writeEndElement(); // Group
}

/**
 * @brief IOxml::readGroup Helper method for XML-Session Import, reads 'Group' token
 * @param r QXmlStreamReader
 */
void IOxml::readGroup(QXmlStreamReader &r)
{

    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "Group")
    {
        return;
    }

    QXmlStreamAttributes a = r.attributes();
    QString gname = a.value("name").toString();

    MRunGroup* g = 0;
    if(gname == Core::instance()->dataModel()->defaultGroup()->title())
        g = Core::instance()->dataModel()->defaultGroup();
    else
        g = new MRunGroup(gname);


    while( !(r.name() == "Group" &&  r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement && r.name() == "MRun")
            readMRun(r,g);
        r.readNext();
    }
}


/**
 * @brief IOxml::writeMRun Helper method for XML-Session Export, writes 'MRun' token.
 * Only Metata (SignalIORequest) of the run's signal data is stored!
 * @param w QXmlStreamWriter
 * @param m MRun
 */
void IOxml::writeMRun(QXmlStreamWriter &w, MRun *m)
{
    w.writeStartElement("MRun");

    w.writeAttribute("name",m->getName());

    //TODO: noChannel-attributes for raw,abs and temperature signal type !!!
    w.writeAttribute("noChannels",QString("%0").arg(m->getNoChannels(Signal::RAW)));

    w.writeAttribute("filename", m->filename);

    SignalIORequest rq = m->importRequest();

    QString xdir ="";
    if(mRelativePaths)
        xdir = mXMLfname;

    // save run settings if enabled
    if(Core::instance()->guiSettings->value("rundetails","overwrite","true").toBool())
    {

        // check if a runsettings file exists
        // if not: save to default directory
        if(rq.runsettings_dirpath.isEmpty())
        {
            QDir dir(Core::rootDir + "data/runsettings");
            if (!dir.exists()) {
                dir.mkpath(".");
            }

            rq.runsettings_dirpath = QString("%0/%1")
                    .arg(dir.absolutePath())
                    .arg(m->parentItem()->data(0).toString());

            qDebug() << "XML-Export: settingsfile of run '"
                     << m->getName() << "' did not exist when the run was imported, generating new settings in directory: "
                     << Core::rootDir + "data/runsettings";
        }

        if(!rq.runsettings_dirpath.endsWith("/"))
            rq.runsettings_dirpath.append("/");

        MRunSettings ms(m);
        ms.save(rq.runsettings_dirpath);
    }

    rq.toXML(w, xdir);

    writeProcessingChain(w,m->getProcessingChain(Signal::RAW));
    writeProcessingChain(w,m->getProcessingChain(Signal::ABS));
    writeProcessingChain(w,m->getProcessingChain(Signal::TEMPERATURE));

    w.writeEndElement(); // MRun
}


/**
 * @brief IOxml::readMRun Helper method for XML-Session Import, reads 'MRun' token.
 * @param r QXmlStreamReader
 * @param parentGroup MRunGroup
 */
void IOxml::readMRun(QXmlStreamReader &r, MRunGroup* parentGroup)
{
    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "MRun")
        return;

    QXmlStreamAttributes a = r.attributes();
    QString runname = a.value("name").toString();
    int noCh = a.value("noChannels").toInt();
    QString filename = a.value("filename").toString();
    if(filename.contains("_settings.txt")) //legacy reasons
        filename.remove("_settings.txt");

    MRun* mr = new MRun(runname, filename, noCh);

    // check if processing steps must be loaded from xml, depends on user data
    int dataMode = m_initialRequest.userData.value(19,0).toInt();
    bool readPsteps = (dataMode != 1);

    if(parentGroup)
    {
         parentGroup->insertChild(mr,-1,false);
    }

    Core::instance()->dataModel()->registerMRun(mr);

    for(int i = 0; i < initGlobalPPCs.size(); i++)
        initGlobalPPCs[i]->connectMRun(mr,i);


    SignalIORequest irq;
    while( !(r.name() == "MRun" &&  r.tokenType() == QXmlStreamReader::EndElement))
    {

        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            if(r.name() == "SignalIORequest")
            {
                irq = SignalIORequest::fromXML(r,mXMLfname);

                // overwrite fields read by SignalIORequest, scince those
                // are redundant and should be removed in future!!
                irq.runname = runname;
                irq.noChannels = noCh;
                irq.runsettings_filename = filename;
                irq.run_id =mr->id();
                mr->setLiiSettings(getLIISettings(irq), true);

                m_generatedRequests.append(irq);
            }
            if(r.tokenType() == QXmlStreamReader::StartElement)
            {
                if(r.name() == "ProcChain" && readPsteps)
                    readProcessingChain(r,mr);
            }
        }
        r.readNext();
    }
    // register the mrun to the programs datamodel
}


/**
 * @brief IOxml::writeProcessingChain Helper method for XML-Session Export,
 *  writes 'ProcessingChain' token.
 * @param w QXmlStreamWriter
 * @param pc ProcessingChain
 */
void IOxml::writeProcessingChain(QXmlStreamWriter &w, ProcessingChain *pc)
{
    if(pc->noPlugs() == 0)
        return;

    w.writeStartElement("ProcChain");
    w.writeAttribute( "stype", QString("%0").arg(pc->getSignalType()));

    for(int i = 0; i < pc->noPlugs(); i++)
    {
        ProcessingPlugin* p = pc->getPlug(i);

        if(p->getName() == DummyPlugin::pluginName)
            continue;

        w.writeStartElement("ProcessingPlugin");
        w.writeAttribute("name",p->getName());
        w.writeAttribute("position",QString("%0").arg(i));
        w.writeAttribute("active",QString("%0").arg(p->activated()));
        w.writeAttribute("plotVis",QString("%0").arg(p->plotVisibility()));

        w.writeAttribute("link_id",QString("%0").arg(p->linkID()));
        w.writeAttribute("link_state",QString("%0").arg(p->linkState()));
        w.writeAttribute("link_type",QString("%0").arg(p->linkType()));
        w.writeAttribute("sbFlag",QString("%0").arg(p->stepBufferEnabled()));

        ProcessingPluginInputList l = p->getInputs();
        for(int j = 0; j < l.size(); j++)
        {
            ProcessingPluginInput pi = l.at(j);
            w.writeStartElement("field");
            w.writeAttribute("identifier",pi.identifier);
            w.writeAttribute("type",pi.typeToString());

            QString val;

            // for comboboxes: do only write selection to xml.
            // TODO: pretty ugly!!
            if(pi.type == ProcessingPluginInput::COMBOBOX)
            {
                QStringList strlist = pi.value.toString().split(";");
                if(strlist.size() > 0)
                    val = strlist.at(0);
            }
            else
                val = pi.value.toString();

            w.writeAttribute("value",val);

            w.writeEndElement(); // field
        }
        w.writeEndElement(); // ProcessingPlugin
    }

    w.writeEndElement(); // ProcChain
}


/**
 * @brief IOxml::readProcessingChain  Helper method for XML-Session Import,
 * reads 'ProcessingChain' token.
 * @param r QXmlStreamReader
 * @param parentRun MRun
 */
void IOxml::readProcessingChain(QXmlStreamReader &r, MRun* parentRun)
{
    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "ProcChain")
        return;

    QXmlStreamAttributes a = r.attributes();

    ProcessingChain* pchain = 0;

    int stype = a.value("stype").toInt();

    if(stype == 0) pchain = parentRun->getProcessingChain(Signal::RAW);
    else if(stype == 1) pchain = parentRun->getProcessingChain(Signal::ABS);
    else if(stype == 2) pchain = parentRun->getProcessingChain(Signal::TEMPERATURE);

    while( !(r.name() == "ProcChain" &&  r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            if(r.name() == "ProcessingPlugin")
            {
                readProcessingPlugin(r,pchain);
            }
        }
        r.readNext();
    }
}


/**
 * @brief IOxml::readProcessingPlugin Helper method for XML-Session Export,
 * reads 'ProcessingPlugin' token.
 * @param r QXmlStreamReader
 * @param parentChain ProcessingChain
 */
void IOxml::readProcessingPlugin(QXmlStreamReader &r, ProcessingChain* parentChain)
{

    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "ProcessingPlugin")
        return;


    QXmlStreamAttributes a = r.attributes();

    QString name = a.value("name").toString();
    int position = a.value("position").toInt();
    bool active = a.value("active").toInt();

    bool plotVis = a.value("plotVis").toInt();

    int link_id = a.value("link_id").toInt();
    int link_state = a.value("link_state").toInt();
    int link_type = a.value("link_type").toInt();

    if(!a.hasAttribute("link_id"))
        link_id = -1;

    if(!a.hasAttribute("plotVis"))
        plotVis = true;

    bool sbFlag = true;
    if(a.hasAttribute("sbFlag"))
        sbFlag = a.value("sbFlag").toInt();

    ProcessingPluginInputList l;

    while( !(r.name() == "ProcessingPlugin" &&  r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            if(r.name() == "field")
            {
                a = r.attributes();
                ProcessingPluginInput pi;

                pi.identifier = a.value("identifier").toString();
                pi.setTypeFromInt( a.value("type").toInt() );
                pi.value = a.value("value").toString();
                l.push_back(pi);
            }
        }
        r.readNext();
    }

    QStringList avPlugs = PluginFactory::getAvailablePluginNames(parentChain->getSignalType());
    if(!avPlugs.contains(name) )
    {
        throw LIISimException("IOxml::readProcessingPlugin: no plugin found with name '"+name+"' for signal type of chain");
    }

    //avoid multiple temperature calculators !!!
    ProcessingPlugin* p =0;

    p = PluginFactory::createInstance(name,parentChain);
    parentChain->addPlug(p);

    ProcessingPluginInputList l0 = p->getInputs();

    for(int j=0; j < l.size(); j++)
    {
        l0.setValue(l.at(j).identifier, l.at(j).value);
    }
    p->setParameters(l0);
    p->setPlotVisibility(plotVis);
    p->setActivated(active);
    p->setStepBufferEnabled(sbFlag);


    // create/reuse links
    if( link_id != -1 )
    {
        ProcessingPluginConnector* conn;
        if(readPlugConnectorsMap.contains(link_id))
        {
            conn = readPlugConnectorsMap.value(link_id,0);
            if(!conn )return;
        }
        else
        {
            conn = new ProcessingPluginConnector(name,link_type,parentChain,false);
            conn->setActive(false);
            readPlugConnectorsMap.insert(link_id,conn);
        }
        conn->add(p);
        p->setLinkState(link_state);
    }
    else
    {
        // for single instance TemperatureCalcualtors we need to check the
        // if a TemperatureChannelID has been assigned (to ensure backwards
        // compatibility with older xml serializations)
        if(name == TemperatureCalculator::pluginName)
        {
            TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(p);
            if(!tc)return;
            if(tc->temperatureChannelID() < 0)
               tc->setTemperatureChannelID(TemperatureCalculator::generateTemperatureChannelID());
        }
    }
    return;
}


/**
 * @brief IOxml::findGlobalPPCs Searches for existing ProcessingPluginConnector Instances on all Runs
 * @return set of all ProcessingPluginConnectors
 */
QList<ProcessingPluginConnector*> IOxml::findGlobalPPCs()
{
    QList<ProcessingPluginConnector*> ppcs;

    // iterate over all MRuns loaded
    QList<MRun*> runs = Core::instance()->dataModel()->mrunList();
    for(int i = 0; i < runs.size(); i++)
    {
        // iterate over all connected ProcessingPlugins of Run
        // QList<ProcessingPluginConnector*> run_ppcs = runs[i]->ppcs(true);
        QList<ProcessingPluginConnector*> run_ppcs = runs[i]->ppcs(false);
        for(int j = 0; j < run_ppcs.size(); j++)
        {
            ProcessingPluginConnector* c = run_ppcs.at(j);
            if(!ppcs.contains(c))
                ppcs << c;
        }
    }

    return ppcs;
}


/**
 * @brief IOxml::deleteAllProcessingSteps removes all processing steps of all runs
 * loaded
 */
void IOxml::deleteAllProcessingSteps()
{
    QList<MRun*> runs = Core::instance()->dataModel()->mrunList();
    for(int i = 0; i < runs.size(); i++)
    {
        runs[i]->getProcessingChain(Signal::SType::RAW)->clearAll();
        runs[i]->getProcessingChain(Signal::SType::ABS)->clearAll();
        runs[i]->getProcessingChain(Signal::SType::TEMPERATURE)->clearAll();
    }
}

/**
 * @brief IOxml::deleteAllRuns deletes all runs and groups loaded
 */
void IOxml::deleteAllRuns()
{
    QList<MRun*> allRuns = Core::instance()->dataModel()->mrunList();
    for(int i = 0; i < allRuns.size(); i++)
        delete allRuns[i];
    QList<MRunGroup*> allGroups = Core::instance()->dataModel()->groupList();
    for(int i = 0; i < allGroups.size(); i++)
        if(allGroups[i] != Core::instance()->dataModel()->defaultGroup())
            delete allGroups[i];
}
