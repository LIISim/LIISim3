#ifndef CORE_H
#define CORE_H

#include <QSettings>
#include <QAction>
#include <QDateTime>

#include "../../calculations/heattransfermodel.h"
#include "database/databasemanager.h"
#include "models/datamodel.h"
#include "signal/signalmanager.h"

#ifdef LIISIM_NIDAQMX
    #include "io/devicemanager.h"
    #include "io/laserenergyposition.h"
#endif

// Load shortcuts for static method calls
#include "logging/msghandlerbase.h"

#include "settings/generalsettings.h"
#include "settings/modelingsettings.h"
#include "settings/guisettings.h"
#include "settings/iosettings.h"
#ifdef LIISIM_PICOSCOPE
    #include "settings/picoscopesettings.h"
#endif
#include "general/nativeeventfilter.h"
#include "gui/utils/notificationmanager.h"

class Numeric;


/**
 * @brief The Core class ...
 */
class Core : public QObject
{
    Q_OBJECT

private:
    Core();
    // reference to data model
    DataModel* m_dataModel;

    DatabaseManager* dbManager;
    SignalManager* sigManager;
    NotificationManager* notifiManager;

    static Core* c_instance;
    static bool m_underConstruction;

    int progressBarSteps;
    int progressBarCounter;

    bool m_expirationDataSet;
    QDateTime expirationDate;
    QDateTime m_settingsLastModified;

    bool m_settingsFound;

public:

    ~Core();

    static const QString LIISIM_VERSION;
    static const bool LIISIM_VERSION_PRE;
    static const bool LIISIM_LARGE_FONTS;

    static Core* instance()
    {
        if(m_underConstruction)
            return 0;

        if(!c_instance)
        {
            m_underConstruction = true;
            c_instance = new Core();
            m_underConstruction = false;
        }
        return c_instance;
    }

    /**
     * @brief rootDir rootdirectory for default program settings,
     * initial session, etc ...
     */
    static const QString rootDir;

    DataModel* dataModel();


    DatabaseManager* getDatabaseManager();
    SignalManager* getSignalManager();
    NotificationManager* getNotificationManager();

    GeneralSettings* generalSettings;
    ModelingSettings* modelingSettings;
    GuiSettings* guiSettings;
    IOSettings* ioSettings;

    NativeEventFilter *nativeEventFilter;
    QTimer *autoSaveTimer;

#ifdef LIISIM_NIDAQMX
    DeviceManager *devManager;
    LaserEnergyPosition *laserEnergyPosition;
#endif

#ifdef LIISIM_PICOSCOPE
    PicoScopeSettings *psSettings;
#endif

    // list of all available heattransfermodels
    QList<HeatTransferModel*> heatTransferModels;

    QAction* actionLoadXMLSession;
    QAction* actionSaveXMLSession;
    QAction* actionLoadProgramSettings;
    QAction* actionSaveProgramSettings;

    // check when settings file was last modified for expiration management
    inline QDateTime settingsLastModified(){ return m_settingsLastModified;}

    // expiration management
    inline bool useExpirationDate(){ return m_expirationDataSet;}
    bool checkIfExpired(const QDateTime& refdate);
    bool showExpirationDialog(const QDateTime &date,
                              bool showOnlyIfExpired,
                              bool shutDownIfExpired);

    bool settingsFound() { return m_settingsFound; }

public slots:

    void loadProgramSettings(QString fname);
    void saveProgramSettings(QString fname);

    /**
     * @brief updateProgressBar Updates the progress bar in the status bar.
     * If 'value' is set to 100, the progress bar will be made invisible.     
     * @param value Percentage value, ranging from 0 to 100
     **/
    void updateProgressBar(int value);
    /**
     * @brief initProgressBar sets maximal number of steps, incProgressBar() can then be used to track progress
     * @param steps maximal number of steps
     */
    void initProgressBar(int steps);
    /**
     * @brief incProgressBar used in combination with initProgressBar
     */

    void updateMaxProgressBarSteps(int steps);
    void incProgressBar();
    void finishProgressBar();

private slots:

    void onActionLoadXMLSession();
    void onActionSaveXMLSession();
    void onActionLoadProgramSettings();
    void onActionSaveProgramSettings();

    void handleAsyncMsg(QString msg,LIISimMessageType type = NORMAL);

    void onAutoSaveTimerTimeout();

signals:

    void programSettingsLoaded();
    void asyncMsg(QString msg,LIISimMessageType type = NORMAL);

    void sig_updateProgressBar(int value);

};

#endif // CORE_H
