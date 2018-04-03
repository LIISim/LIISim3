#include "core.h"
#include <QVector>
#include <QSettings>
#include <QFile>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>

#include "general/LIISimException.h"
#include "settings/settingsbase.h"
#include "gui/signalEditor/sessionsavedialog.h"
#include "gui/signalEditor/sessionloaddialog.h"
#include "io/ioxml.h"
#include "calculations/numeric.h"


// heat transfer model classes
#include "calculations/models/htm_melton.h"
#include "calculations/models/htm_kock_soot.h"
#include "calculations/models/htm_liu.h"
#include "calculations/models/htm_menser.h"
#include "calculations/models/htm_mansmann.h"


Core* Core::c_instance = 0;
bool  Core::m_underConstruction = false;

const QString Core::LIISIM_VERSION  = "3.0.6";
const bool Core::LIISIM_VERSION_PRE = false;

// ----------------------
// PROGRAM ROOT DIRECTORY
// ----------------------
const QString Core::rootDir = "../src/";   // useful for developement (build directory is in LIISim3 directory)
//const QString Core::rootDir = "";            // use same folder as .exe

Core::Core() : QObject(0)
{
    MSG_DETAIL_1("init Core class");

    // Expiration management for releases
    // STABLE: set 'false' to disable expiration
    // TEST/EVALUATION: set 'true' to enable expiration at given date
    m_expirationDataSet = false;
    expirationDate.setDate( QDate(2018, 6, 15)); // YYYY,MM,DD
    expirationDate.setTime( QTime(23,50,0,0));

    m_settingsFound = false;

    generalSettings     = new GeneralSettings;
    modelingSettings    = new ModelingSettings;
    guiSettings         = new GuiSettings;
    ioSettings          = new IOSettings;

#ifdef LIISIM_PICOSCOPE
    psSettings    = new PicoScopeSettings;
#endif

    dbManager     = new DatabaseManager();
    m_dataModel   = new DataModel();
    notifiManager = new NotificationManager();

    // setup signal/slot connections for setting/ModellingSettings
    modelingSettings->setupConnections(this);

    // assign core to SignalManager
    sigManager = new SignalManager(m_dataModel);    
    sigManager->setCore(this);

    // programm settings/session auto-save
    autoSaveTimer = new QTimer;
    autoSaveTimer->setTimerType(Qt::CoarseTimer);

#ifdef LIISIM_NIDAQMX

    // detect if computer is about to enter a suspended state (needed for DeviceManager)
    nativeEventFilter = new NativeEventFilter();
    QApplication::instance()->installNativeEventFilter(nativeEventFilter);

    devManager = new DeviceManager();
    laserEnergyPosition = new LaserEnergyPosition(this);

#endif

    // setup heat transfer model list
    HTM_Melton          *h1 = new HTM_Melton;
    HTM_KockSoot        *h2 = new HTM_KockSoot;
    HTM_Liu             *h3 = new HTM_Liu;

#ifdef LIISIM_FULL
    HTM_Menser          *h4 = new HTM_Menser;
    HTM_Mansmann        *h5 = new HTM_Mansmann;
#endif

    heatTransferModels.push_back( h1 );
    heatTransferModels.push_back( h2 );
    heatTransferModels.push_back( h3 );

#ifdef LIISIM_FULL
    heatTransferModels.push_back( h4 );
    heatTransferModels.push_back( h5 );
#endif

    // set standard heat transfer model (to avoid NULL pointer -> crash)
    modelingSettings->m_heatTransferModel = heatTransferModels.at(0);

    // progressbar
    progressBarSteps    = 1;
    progressBarCounter  = 1;


    // actions for HomeScreen can be reused anywhere
    actionLoadXMLSession = new QAction("Load Session (xml)",this);
    actionLoadXMLSession->setIcon(QIcon(Core::rootDir + "resources/icons/open_folder.png"));
    connect(actionLoadXMLSession,SIGNAL(triggered()),
            SLOT(onActionLoadXMLSession()));

    actionSaveXMLSession = new QAction("Save Session (xml)",this);
    actionSaveXMLSession->setIcon(QIcon(Core::rootDir + "resources/icons/report_save.png"));
    connect(actionSaveXMLSession,SIGNAL(triggered()),
            SLOT(onActionSaveXMLSession()));

    actionLoadProgramSettings = new QAction("Load Program Settings",this);
    actionLoadProgramSettings->setIcon(QIcon(Core::rootDir + "resources/icons/cog_go.png"));
    connect(actionLoadProgramSettings,SIGNAL(triggered()),
            SLOT(onActionLoadProgramSettings()));

    actionSaveProgramSettings = new QAction("Save Program Settings",this);
    actionSaveProgramSettings->setIcon(QIcon(Core::rootDir + "resources/icons/cog_edit.png"));
    connect(actionSaveProgramSettings,SIGNAL(triggered()),
            SLOT(onActionSaveProgramSettings()));

    connect(this,SIGNAL(asyncMsg(QString,LIISimMessageType)),
            SLOT(handleAsyncMsg(QString,LIISimMessageType)),
            Qt::QueuedConnection);

    connect(autoSaveTimer, SIGNAL(timeout()), SLOT(onAutoSaveTimerTimeout()));

    MSG_DETAIL_1("Core init done.");
}


Core::~Core()
{
    if(generalSettings != NULL)
        delete generalSettings;

    if(modelingSettings != NULL)
        delete modelingSettings;

    if(guiSettings != NULL)
        delete guiSettings;

    if(ioSettings != NULL)
        delete ioSettings;

#ifdef LIISIM_PICOSCOPE
    if(psSettings != NULL)
        delete psSettings;
#endif

    if(dbManager != NULL)
        delete dbManager;

    if(sigManager != NULL)
        delete sigManager;

    if(m_dataModel != NULL)
        delete m_dataModel;

#ifdef LIISIM_NIDAQMX
    if(devManager != NULL)
        delete devManager;

    if(laserEnergyPosition != NULL)
        delete laserEnergyPosition;

    if(nativeEventFilter != NULL)
        delete nativeEventFilter;
#endif

    if(notifiManager)
        delete notifiManager;
}

/********
 * get functions
 ********/

DataModel* Core::dataModel()
{
    return m_dataModel;
}

DatabaseManager* Core::getDatabaseManager()
{
    return dbManager;
}

SignalManager* Core::getSignalManager()
{
    return sigManager;
}

NotificationManager* Core::getNotificationManager()
{
    return notifiManager;
}


/********
 * public slots
 ********/

/**
 * @brief Core::saveProgramSettings
 * @param fname
 */
void Core::saveProgramSettings(QString fname)
{
    QSettings settings(fname,QSettings::IniFormat);

    if(!settings.isWritable())
    {
        qDebug() << "cannot write application settings to "+fname;
        return;
    }
    generalSettings->write(settings);
    modelingSettings->write(settings);
    guiSettings->write(settings);
    ioSettings->write(settings);

#ifdef LIISIM_PICOSCOPE
    psSettings->write(settings);
#endif

    MSG_NORMAL("Saved program settings to: " + fname);
}


/**
 * @brief Core::loadProgramSettings
 * @param fname
 */
void Core::loadProgramSettings(QString fname)
{

    // check if .ini file exists
    QFile file( fname );
    if(!file.exists())
    {
        MESSAGE("Application settings not found, generating default settings.", INFO);
        m_settingsFound = false;
        //MESSAGE("Cannot load application settings: "+fname+" (file does not exist!)", ERR_IO);
        emit programSettingsLoaded();
        emit generalSettings->settingsChanged();
        emit guiSettings->settingsChanged();
        emit ioSettings->settingsChanged();
#ifdef LIISIM_PICOSCOPE
        emit psSettings->settingsChanged();
#endif
        dbManager->slot_scanDatabase();

        modelingSettings->init();

        MESSAGE("Default application settings generated.", INFO);
        return;
    }

    m_settingsFound = true;

    if(Core::instance()->useExpirationDate())
    {
        QFileInfo fi(file);
        m_settingsLastModified = fi.lastModified();
    }

    file.close();

    QSettings settings(fname,QSettings::IniFormat);

    generalSettings->read(settings);

    // load all database files
    dbManager->slot_scanDatabase();

    modelingSettings->read(settings);
    guiSettings->read(settings);
    ioSettings->read(settings);

#ifdef LIISIM_PICOSCOPE
    psSettings->read(settings);
#endif

    MSG_NORMAL("Core: Program settings loaded");

    emit programSettingsLoaded();
}


void Core::handleAsyncMsg(QString msg, LIISimMessageType type)
{
    MESSAGE(msg,type);
}


void Core::onAutoSaveTimerTimeout()
{
    if(sigManager->isBusy())
    {
        MESSAGE("Processing signals, auto-saving skipped!", WARNING);
        return;
    }

    MESSAGE("Auto-saving settings...", INFO);

    IOxml::saveInitSession();

    QString savePath;
    if(guiSettings->hasEntry("io/programsettings","lastfname"))
    {
        savePath = guiSettings->value("io/programsettings","lastfname").toString();
    }
    else
    {
        savePath = rootDir + "defaultSettings.ini";
    }
    saveProgramSettings(savePath);
}


/**********
 * ProgressBar
 **********/

/**
 * @brief Core::initProgressBar provide maximal number of steps, then increment with incProgressBar()
 * @param steps
 */
void Core::initProgressBar(int steps)
{
    if(steps < 1)
        progressBarSteps = 1;
    else
        progressBarSteps = steps;

    progressBarCounter  = 0;

    // show progress bar
    updateProgressBar(0);
}


void Core::updateMaxProgressBarSteps(int steps)
{
    // update only if steps is larger than 1
    if(steps <= 1)
        return;

    progressBarSteps = steps;

    int progress = floor(100.0 * progressBarCounter / progressBarSteps);

    updateProgressBar(progress);
}


void Core::incProgressBar()
{
    // update only if max steps is larger than 1
    if(progressBarSteps <= 1)
        return;

    progressBarCounter++;
    //qDebug() << "Core: inc: " << progressBarCounter << progressBarSteps;

    int progress = floor(100.0 * progressBarCounter / progressBarSteps);

    updateProgressBar(progress);
}


void Core::finishProgressBar()
{
    progressBarSteps    = 1;
    progressBarCounter  = 1;
    updateProgressBar(100);
}


void Core::updateProgressBar(int value)
{   
    emit sig_updateProgressBar(value);
}


/**********
 * ACTIONS
 **********/

/**
 * @brief Core::onActionLoadXMLSession This slot is executed when the "Load Session (XML)"-Action
 * has been triggered. It opens a file dialog and initiates a XML-Import
 */
void Core::onActionLoadXMLSession()
{
    SessionLoadDialog* diag = new SessionLoadDialog();
    diag->exec();
}


/**
 * @brief Core::onActionSaveXMLSession This slot is executed when the "Save Session (XML)"-Action
 * has been triggered. It opens a file dialog and initiates a XML-Export
 */
void Core::onActionSaveXMLSession()
{
    SessionSaveDialog* diag = new SessionSaveDialog;
    diag->exec();
}


/**
 * @brief Core::onActionLoadProgramSettings This slot is executed when the "Load Program Settings"-Action
 * has been triggered. It opens a file dialog and loads the file selected
 */
void Core::onActionLoadProgramSettings()
{
    // get the path of the last file used from gui settings
    QString recent = guiSettings->value("io/programsettings","lastfname").toString();
    if(recent.isEmpty())
        recent = rootDir;

    QString fname = QFileDialog::getOpenFileName(QApplication::focusWidget(),
                                         "Load Program Settings",
                                         recent,".ini (*.ini)");
    if(fname.isEmpty())return;


    // update the path of the last file used
    guiSettings->setValue("io/programsettings","lastfname",fname);

    MSG_NORMAL("loading program settings from "+fname+" ...");
    loadProgramSettings(fname);
}


/**
 * @brief Core::onActionSaveProgramSettings This slot is executed when the "Save Program Settings"-Action
 * has been triggered. It opens a file dialog and saves the current program settings
 */
void Core::onActionSaveProgramSettings()
{
    // get the path of the last file used from gui settings
    QString recent = guiSettings->value("io/programsettings","lastfname").toString();
    if(recent.isEmpty())
        recent = rootDir;

    QString fname;
    fname = QFileDialog::getSaveFileName(QApplication::focusWidget(),
                                         "Save Program Settings",
                                         recent,".ini (*.ini)");
    if(fname.isEmpty())return;

    // update the path of the last file used
    guiSettings->setValue("io/programsettings","lastfname",fname);

    saveProgramSettings(fname);
}


/********************************
 * BETA Expiration Management
  *******************************/

bool Core::checkIfExpired(const QDateTime &refdate)
{
    return (refdate > expirationDate);
}


bool Core::showExpirationDialog(const QDateTime &date,
                                bool showOnlyIfExpired,
                                bool shutDownIfExpired)
{
    bool expired = checkIfExpired(date);

    if(!showOnlyIfExpired || expired)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(
                    QString("LIISim %0 - BETA (Expiring in %1 days)")
                    .arg(LIISIM_VERSION)
                    .arg( date.daysTo(expirationDate) ));

        QString msg;
        msg.append( "This BETA version is <b>for testing/evaluation purposes only</b> and will expire <br>after 6 weeks.<br><br>");
        msg.append( "We highly appreciate any feedback that helps us to improve the software<br>");
        msg.append( "before it will be officially released.<br>");
        msg.append( "<b>Please don't hesitate to write us an email with your comments!</b>");
        msg.append( "<br><br>When this test version is expired please contact raphael.mansmann@uni-due.de for the ");
        msg.append( "most recent BETA version or check the LIISim website for the official public release.");

        msg.append("<br><br><b>");
        if(expired)
           msg.append("<font color='#d73027'>Your copy of LIISim has expired on: ");
        else
           msg.append("<font color='#1a9641'>Your copy of LIISim will expire on: ");

        QLocale locale = QLocale(QLocale::English, QLocale::UnitedStates);

        msg.append(locale.toString(expirationDate, "dd MMMM yyyy")+"</font><\b>");

        msgBox.setText(msg);
        msgBox.exec();
    }

    if(expired && shutDownIfExpired)
    {
        qApp->quit();
    }
    return expired;
}
