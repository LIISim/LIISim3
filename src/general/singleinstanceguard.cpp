#include "singleinstanceguard.h"

#include<QCryptographicHash>

namespace
{
    QString generateKeyHash( const QString& key, const QString& salt )
    {
        QByteArray data;

        data.append( key.toUtf8() );
        data.append( salt.toUtf8() );
        data = QCryptographicHash::hash( data, QCryptographicHash::Sha1 ).toHex();

        return data;
    }
}

SingleInstanceGuard::SingleInstanceGuard(const QString& key) : key(key), memLockKey(generateKeyHash(key, "_memLockKey")),
    sharedmemKey(generateKeyHash(key, "_sharedmemKey")), sharedMem(sharedmemKey), memLock(memLockKey, 1)
{
    memLock.acquire();
    {
        QSharedMemory fix( sharedmemKey );
        fix.attach();
    }
    memLock.release();
}

SingleInstanceGuard::~SingleInstanceGuard()
{
    release();
}

bool SingleInstanceGuard::anotherInstanceRunning()
{
    if(sharedMem.isAttached())
        return false;

    memLock.acquire();
    const bool isRunning = sharedMem.attach();
    if (isRunning)
        sharedMem.detach();
    memLock.release();

    return isRunning;
}

bool SingleInstanceGuard::tryToRun()
{
    if(anotherInstanceRunning())   // Extra check
        return false;

    memLock.acquire();
    const bool result = sharedMem.create(sizeof(quint64));
    memLock.release();
    if(!result)
    {
        release();
        return false;
    }

    return true;
}

void SingleInstanceGuard::release()
{
    memLock.acquire();
    if(sharedMem.isAttached())
        sharedMem.detach();
    memLock.release();
}

