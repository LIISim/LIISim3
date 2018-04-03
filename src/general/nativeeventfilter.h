#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>

class NativeEventFilter: public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    NativeEventFilter(QObject *parent = 0);
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

signals:
    void systemAboutToSuspend();
};

#endif // NATIVEEVENTFILTER_H
