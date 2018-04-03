#ifndef SINGLEINSTANCEGUARD_H
#define SINGLEINSTANCEGUARD_H

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>

class SingleInstanceGuard
{
public:
    SingleInstanceGuard(const QString& key);
    ~SingleInstanceGuard();

    bool anotherInstanceRunning();
    bool tryToRun();
    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    Q_DISABLE_COPY(SingleInstanceGuard)
};

#endif // SINGLEINSTANCEGUARD_H
