#include "nativeeventfilter.h"

#include "Windows.h"

NativeEventFilter::NativeEventFilter(QObject *parent) : QObject(parent), QAbstractNativeEventFilter()
{

}


bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if(eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        MSG *msg = (MSG*)(message);

        if(msg->message == WM_POWERBROADCAST) //0x218
        {
            if(msg->wParam == PBT_APMSUSPEND)
            {
                emit systemAboutToSuspend();
                return true;
            }
        }
    }
    return false;
}
