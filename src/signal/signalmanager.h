#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMutex>
#include <QTime>
#include <QQueue>
#include <QThreadPool>
#include <QMap>
#include <QHash>
#include <QAction>
#include <QFileInfo>

class ProcessingTask;
class ProcessingChain;
class Core;
class MRun;
class MRunGroup;
class DataModel;

#include "../general/LIISimException.h"
#include "mrun.h"
#include "memusagemonitor.h"
#include "../io/signaliorequest.h"

/**
 * @brief The SignalManager class manages Signal Import/Export and concurrent signal processing.
 * @details
 */
class SignalManager : public QObject
{
    Q_OBJECT

    /** @brief store reference to core */
    Core* core;

    DataModel* m_dataModel;

    public:

        explicit SignalManager(DataModel* dataModel, QObject *parent = 0);
        ~SignalManager() { reset(); }

        DataModel* dataModel() { return m_dataModel; }

        /**
         * @brief clears MRun List, frees memory for all MRuns
         */
        void reset() {}

        Core* getCore() {           return core;       }
        void setCore(Core* core){   this->core = core; }

        bool isImporting() { return isLoading; }
        bool isExporting() { return m_isExporting;}
        bool isBusy() {      return m_isExporting|| m_isFitting || isLoading || (pendingProcessingTasks > 0); }

        inline QAction* actionDataImport(){return m_importAction;}
        inline QAction* actionDataExport(){return m_exportAction;}

        QString getFnamePatternRegExp(QString var);

        MemUsageMonitor* memUsageMonitor(){return &mem_monitor;}

    private:

        /** @brief default directory which should be opened for signal import (used for scans)*/
        QString defaultDirectory;


        /** @brief threadpool where all loading and processing tasks will be started in*/
        QThreadPool *threadPool ;


        bool isLoading;
        bool m_isExporting = false;
        bool m_isFitting;

        QMutex procMutex;

        /**
         * @brief m_calcMode calculation mode
         *  0: recalculate single run only
         *  1: recalculate group
         *  2: recalculate all runs
         */
        int m_calcMode;

        /** @brief number of total processing tasks */
        int totalProcessingTasks;

        /** @brief number of pending processing tasks */
        int pendingProcessingTasks;

        /** @brief measures signal processing time */
        QTime processingTimer;

        void initActions();
        QAction* m_importAction;
        QAction* m_exportAction;

        MemUsageMonitor mem_monitor;

        bool checkMemory();

    public slots:

        void importSignalsManager(SignalIORequest rq);
        void exportSignalsManager(SignalIORequest rq);        
        void processAllMRuns(Signal::SType startStype = Signal::RAW);
        void processRunList(QList<MRun*>& runs,Signal::SType startStype = Signal::RAW );

        //void processSignals(MRun* mrun, Signal::SType startStype = Signal::RAW);
        void processSignals(MRun *mrun, QList<Signal::SType> typeList);

        void setCalculationMode(int mode);
        int calculationMode(){return m_calcMode;}

        void onProcessingTaskFinished(MRun* mrun);
        void onMessageReceive(const QString & msg, const LIISimMessageType & msg_type = NORMAL );

        void checkImportRequest(SignalIORequest irq);
        void loadImportRequests(QList<SignalIORequest> irq);

        void cancelProcessingTasks();

    private slots:
        void handleActionImport();
        void handleActionExport();

        void onImportFinished(int noSignals, double time);
        void onProgressUpdate(float value);
        void onExportFinished(double time);

        void onFitrunCountChanged();
        void onFitStarted();
        void onFitFinished();

        void onCheckFilesResult(QList<SignalIORequest> result);
        void onIOImportSuccess(SignalIORequest source, SignalFileInfoList fileList);
        void onIOImportError(SignalIORequest source, SignalFileInfo file, QString error);

        void exportFileDialog(QFileInfo &finfo, int &ret);

    signals:

        void processingStateChanged(bool state);
        void importStateChanged(bool state);
        void exportStateChanged(bool state);
        void fitStateChanged(bool state);

        void processingTasksStop();

        /** @brief importFinished emitted if all requested imports are finished */
        void importFinished();

        /** @brief emitted if the import progress has changed, vaulue within range 0..1*/
        void progressUpdate(int value);

        /** @brief exportFinished emitted if export is done. */
        void exportFinished();

        /** @brief processingFinished emitted if signal processing for given runname is finished */
        void processingFinished(QString mrunname, Signal::SType stype);

        /** @brief allProcessingTasksFinished is emitted if all ProcessingTasks from the threadpool have been finished */
        void allProcessingTasksFinished();

        void checkImportRequestFinished(QList<SignalIORequest> result);

        void ioImportSuccess(SignalIORequest source, SignalFileInfoList fileList);
        void ioImportError(SignalIORequest source, SignalFileInfo file, QString error);
};

#endif // SIGNALMANAGER_H
