#ifndef PROCESSINGTASK2_H
#define PROCESSINGTASK2_H

#include <QObject>
#include <QRunnable>
#include <QList>
#include <QMutex>
#include "../../general/LIISimException.h"
#include "../mrun.h"

/**
 * @brief The ProcessingTask class is responsible for calculating all ProcessingChains
 * of a MRun Object. It is started in the global Threadpool by the SignalManager.
 * @ingroup Signal-Processing
 */
class ProcessingTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ProcessingTask(MRun* mrun, Signal::SType start_signal_type, QObject *parent = 0);
    explicit ProcessingTask(MRun* mrun, QList<Signal::SType> typeList, QObject *parent = 0);

    static void resetIdCounter();
private:

    /// @brief MRun Object
    MRun* mrun;

    /// @brief the signal type of the first processing chain
    Signal::SType startStype;

    QList<Signal::SType> processTypes;

    /// @brief number of measurements (of mrun)
    int noMpoints;

    void run();

    void processChain(Signal::SType stype);

    /// @brief counter used for ID generation
    static unsigned long id_count;

    /// @brief id of processing task
    unsigned long id;

    bool threadShouldStop;

signals:

    /**
     * @brief finished emmitted if the processing job is done.
     * @param MRun Object
     */
    void finished(MRun* mrun);


    /**
     * @brief signal_msg used for sending text/error messages via signal/slot system
     * @param msg   text message
     * @param msg_type  message type
     */
    void signal_msg(const QString & msg, const LIISimMessageType & msg_type = NORMAL );

public slots:

    void stopTask();

};

#endif // PROCESSINGTASK2_H
