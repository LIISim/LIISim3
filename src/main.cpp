
#include <QApplication>

/** @mainpage
 *
 * This documentations contains detailed information about the LIISim source code.
 *
 * .:. TODO .:.
 *
 * Changelog:
 * - v ...
 *
 *
 * External Libraries:
 * - Qt Framework 5.5.0
 *   http://qt-project.org/
 * - Boost library 1.55.0 (November 11th, 2013 19:50 GMT)
 *   http://www.boost.org/users/download/
 * - Qwt plot library 6.1.0 (compiled for Qt 5.2.1)
 *   http://qwt.sourceforge.net/
 * - MatIO 1.5.2: Matlib MAT File I/O Library
 *   http://matio.sourceforge.net/
 *
 * @defgroup External-Libraries
 * @brief External C++ libraries
 *
 * @defgroup Qt-Framework
 * @ingroup External-Libraries
 * @brief Qt Framework 5.2.1, see http://qt-project.org/
 *
 * @defgroup Boost
 * @ingroup External-Libraries
 * @brief Boost library 1.55.0, see http://www.boost.org/users/download/
 *
 * @defgroup Qwt
 * @ingroup External-Libraries
 * @brief Qwt plot library 6.1.0, see http://qwt.sourceforge.net/
 *
 *
 * @defgroup GUI
 * @brief Graphical User Interface
 * @details Widget based Desktop GUI using the Qt-Framework
 *
 * @defgroup GUI-Utilities
 * @ingroup GUI
 * @brief Frequently used custom widgets and helper-classes.
 *
 * @defgroup IO
 * @brief Framework defining signal data and program state Import/Export-Operations
 *
 * @defgroup Hierachical-Data-Model
 * @brief Classes which store signal data within a hierachical tree
 * @details TODO: details here ...
 *
 * @defgroup Signal-Processing
 * @brief Classes regarding the processing of signal data
 * @details TODO: details here ...
 *
 * @defgroup ProcessingPlugin-Implementations
 * @ingroup Signal-Processing
 * @brief Implementations of the abstract ProcessingPlugin class.
 */


#include <QMessageBox>
#include <QDebug>
#include <QTextBlock>
#include <QTextCursor>

#include "core.h"
#include "logging/msghandlerbase.h"
#include "logging/consoleouthandler.h"
#include "logging/logfilehandler.h"

#include "gui/masterwindow.h"

#include "signal/processing/pluginfactory.h"

#include "io/ioxml.h"

#include "general/singleinstanceguard.h"

int main(int argc, char *argv[])
{    
    SingleInstanceGuard guard("random_key");
    if(!guard.tryToRun())
    {
        QApplication app(argc, argv);
        QMessageBox msgbox;
        msgbox.setText("Error:");
        msgbox.setInformativeText(QString("Another instance is already running. Please close the instance before opening another instance."));
        msgbox.exec();
        return 0;
    }


    // register meta types (for signal/slot mechanism for multiple threads)
    qRegisterMetaType<LIISimMessageType>("LIISimMessageType");
    qRegisterMetaType<SignalFileInfoList>("SignalFileInfoList");
    qRegisterMetaType<SignalFileInfo>("SignalFileInfo");
    qRegisterMetaType<SignalIORequest>("SignalIORequest");
    qRegisterMetaType<Signal::SType>("Signal::SType");
    qRegisterMetaType<QTextBlock>("QTextBlock");
    qRegisterMetaType<QTextCursor>("QTextCursor");
    qRegisterMetaType<QTextCharFormat>("QTextCharFormat");
    qRegisterMetaType<QList<SignalIORequest>>("QList<SignalIORequest>");
    qRegisterMetaType<LogMessage>("LogMessage");

    qInstallMessageHandler(&MsgHandlerBase::handleQtMessage);

#if QT_VERSION >= 0x050600
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    ConsoleOutHandler* outputHandler = new ConsoleOutHandler();
    LogFileHandler* logfileHandler = new LogFileHandler();

    MasterWindow* master = new MasterWindow;
    master->show();


    // switch expiration mode in Core constructor (m_expirationDataSet)
    if(Core::instance()->useExpirationDate())
    {
        bool expired = Core::instance()->showExpirationDialog(QDateTime::currentDateTime(),false,false);
        if(!expired)
            expired = Core::instance()->showExpirationDialog(
                      Core::instance()->settingsLastModified(),true,false);

        // close program if software is exprired
        if(expired) return 0;
    }

    // load general-, modeling-, gui-, io- and picoscopesettings and textfile database
    Core::instance()->loadProgramSettings(Core::rootDir + "defaultSettings.ini");

    // load initial program data session (groups, runs, processing steps)
    IOxml::loadInitSession();

    // load recent processing chains
    PluginFactory::instance()->handleApplicationStartup();

#ifdef LIISIM_NIDAQMX
    Core::instance()->devManager->handleStartup();
#endif

    int res = app.exec();

    // do this when program is closed:
    try
    {
        // save general-, modeling-, gui-, io- and picoscopesettings and textfile database
        Core::instance()->saveProgramSettings(Core::rootDir + "defaultSettings.ini");

        // save initial program data session (groups, runs, processing steps)
        IOxml::saveInitSession();

        // wait until the parallel export is done.
        while(Core::instance()->getSignalManager()->isExporting())
        {
            // manually process pending events (signals) in Eventloop
            // why? -> main eventloop is finished after 'app.exec()'!
            QApplication::processEvents();
        }        
    }
    catch(LIISimException e)
    {
        MSG_ERR(e.what());
    }

    delete outputHandler;
    delete Core::instance();
    // close logfile at the end
    delete logfileHandler;

    return res;
}
