#ifndef IOBASE_H
#define IOBASE_H

#include <QObject>
#include <QTime>
#include <QDebug>
#include "../signal/signalmanager.h"


/**
 * @brief The IOBase class is the base class of
 * all classes which read/write signal data from/to
 * the internal MRun data structure. Those derived classes
 * will be referred as IO-Objects. Each IO-Object provides
 * read/write support for a certain file format/structure.
 * IO-Objects will be created by the SignalManager when an
 * IO request has been emited by the GUI.
 * IO-Objects should be destroyed after one import/export operation
 * (see also IOBase::setAutoDelete()).
 * A call to IOBase::importSignals() will initiate the whole import process.
 * When the import is done, the IOBase::importFinished() signal will be handled
 * by the SignalManager (same for export).
 */
class IOBase : public QObject
{
    Q_OBJECT

    public:
        explicit IOBase( QObject *parent = 0);
        /*virtual ~IOBase(){
            qDebug() << "~IOBase";
        }*/

        void importSignals(const SignalIORequest &irq);

        void importSignals(const QList<SignalIORequest> &irq);

        void exportSignals(const SignalIORequest & irq);

        void setAutoDelete(bool state);

        void setEnabledConcurrency(bool state);

        inline void setMRunToProcess(MRun* mrun){this->m_mrun = mrun;}

        inline void setMuteLogging(bool state){this->m_muteLog = state;}

        /**
         * @brief The ParallelizationMode enum
         */
        enum ParallelizationMode{
            PM_PERMRUN,     // one worker thread per mrun
            PM_PERFILE,     // one worker thread per file
            PM_PERCHANNEL   // one worker thread per mrun channel
        };

        /**
         * @brief static abort flag, all Subclass
         * instances should check if this flag is set during import/export.
         */
        static bool abort_flag;

        void checkImportRequest(const SignalIORequest &irq);

        virtual void checkFiles() = 0;

    protected:

        /**
         * @brief setupImport This abstract method must be implemented by derived classes.
         * This method is executed once at the beginning of the import-process. Here
         * we can make file-format-specific preparations for the import (eg. directory scan...).
         * This method is executed concurrently to the GUI thread and HAS TO
         * emit the IOBase::importSetupFinished() signal => IOBase::onImportSetupFinished()
         * will be executed
         */
        virtual void setupImport() = 0;

        /**
         * @brief importStep This method loads a list of data files to the given
         * mrun object. This Method is called multiple times, concurrently (by IOBase::onImportSetupFinished())! Programmer must
         * ensure thread safety of code! This method must be implemented by derived classes
         * and emit the IOBase::importStepFinished() signal!
         * @param mrun measurement run object where the data should be loaded to
         * @param fileInfo list of datafile informations
         */
        virtual void importStep(MRun* mrun,SignalFileInfoList  fileInfos) = 0;

        /**
         * @brief exportImplementation This abstract method must be implemented by derived classes.
         * This function should write datafiles for a certain format using informations of
         * the passed SignalIORequest
         * @param irq
         */
        virtual void exportImplementation(const SignalIORequest & irq) = 0;


        /// @brief holds the signal io request which has been passed from gui
        SignalIORequest m_initialRequest;


        /// @brief holds a list of signal-IO-requests generated during setupImport()
        QList<SignalIORequest> m_generatedRequests;


        /// @brief holds parallelization mode for io operations
        ParallelizationMode p_mode;

        /// @brief timer to store import/export times
         QTime m_timer;

        MRun* m_mrun;

        /**
         * @brief logMessage This signal sends a text message to the program log
         * via a qeued connection
         * @param msg message
         * @param msg_type message type
         */
        void logMessage(const QString& msg, LIISimMessageType msg_type = NORMAL);

        LIISettings getLIISettings(SignalIORequest &irq);


        /// @brief export flag: save raw signal data
        bool e_flag_raw;

        /// @brief export flag: save absolute signal data
        bool e_flag_abs;

        /// @brief expoart flag: save temperature signal data
        bool e_flag_tmp;

        /// @brief export flag: save raw data postprocessed
        bool e_flag_raw_postproc;

        /// @brief export flag: save absolute data postprocessed
        bool e_flag_abs_postproc;

        /// @brief export flag: save temperature data postprocessed
        bool e_flag_temp_postproc;

        /// @brief export flag: save raw standard deviation data
        bool e_flag_raw_stdev;

        /// @brief export flag: save abs standard deviation data
        bool e_flag_abs_stdev;

        /// @brief export flag: save temperature standard deviation data
        bool e_flag_temp_stdev;

        ///@brief export flag: enable matlab file compression
        bool e_flag_matlab_compression;

    private:

        /// @brief number of parallel io-operations
        int m_task_count;

        /// @brief number of finished parallelized io-operations
        int m_finished_tasks;

        /// @brief holds the number of successfully loaded/saved signals
        int m_numberOfSignals;

        /// @brief holds auto delete flag
        bool m_autoDelete;

        bool m_kill_ok1;
        bool m_kill_ok2;

        bool m_concurrency;

        /// @brief mutes logging
        bool m_muteLog;

    signals:

        /**
         * @brief importSetupFinished This signal must be emitted if
         * at the end of the setupImport() method to proceed with importing
         */
        void importSetupFinished();

        /**
         * @brief importStepFinished This signal must be emitted at the
         * end of the importFiles() method to keep track of finished import steps.
         * Derived classes must emit this signal at the end of the importStep()
         * method to notify IOBase that a concurrent IO-operation is done.
         * @param noSignals number of successfully loaded signals
         */
        void importStepFinished(int noSignals);


        /**
         * @brief importFinished This signal is emitted if the import of all files
         * has finished. This signal will be emitted by IOBase and is caught by
         * the SignalManager.
         * @param noSignals number of read signals
         * @param loading time in seconds
         */
        void importFinished(int noSignals, double time);


        /**
         * @brief exportFinished This signal is emitted if export of a SignalIORequest
         * is finished.
         * @param time export time in seconds
         */
        void exportFinished(double time);

        /**
         * @brief exportImplementationFinished This signal is emitted if export of a SignalIORequest
         * is finished. This signal must be emitted by derived classes and is caught by
         * the SignalManager.
         * @param time export time in seconds
         */
        void exportImplementationFinished(double time);

        /**
         * @brief progressUpdate This signal is emitted after an importStep is finished and provides
         * information about current progress
         * @param value progress in range 0..1
         */
        void progressUpdate(float value);

        /**
         * @brief sig_logMessage This signal sends a text message to the program log
         * via a qeued connection
         * @param msg message
         * @param msg_type message type
         */
        void sig_logMessage(const QString& msg, LIISimMessageType msg_type = NORMAL);

        void checkFilesResult(QList<SignalIORequest> result);

        void importError(SignalIORequest source, SignalFileInfo file, QString error);

        void importSuccess(SignalIORequest source, SignalFileInfoList fileList);

    protected slots:

        void onImportSetupFinished();
        void onImportStepFinished(int noSignals);

    private slots:

        void handleQueuedMessage(const QString& msg, LIISimMessageType msg_type = NORMAL);
        void onExportImplementationFinished(double time);

    public slots:

};

#endif // IOBASE_H
