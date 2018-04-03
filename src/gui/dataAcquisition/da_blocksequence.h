#ifndef DA_BLOCKSEQUENCE_H
#define DA_BLOCKSEQUENCE_H

#include <QThread>
#include <QSemaphore>

#include <core.h>


class DataAcquisitionWindow;

class DA_BlockSequenceWorker : public QThread
{
    Q_OBJECT
public:
    DA_BlockSequenceWorker(QObject *parent);
    ~DA_BlockSequenceWorker();

    void run() Q_DECL_OVERRIDE;
    void stop();

    bool isStopping();

private:
    bool threadShouldStop;
    bool voltageBoundReached;

    QSemaphore blockRunLock;

    void updateTimeLeft(DataAcquisitionWindow *daw, int nsize, double waitTime, int noCaptures);
    void resetTimeLeft(DataAcquisitionWindow *daw);

signals:
    void workerStopped();
    void voltageLimitReached();

    void status(QString status);

    void startBlockRun();

private slots:
    void onFinished();

public slots:
    void blockCaptured();

};

#endif // DA_BLOCKSEQUENCE_H
