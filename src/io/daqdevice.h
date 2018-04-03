#ifndef DAQDEVICE_H
#define DAQDEVICE_H

#include <cstdint>

#include <QString>
#include <QMap>

class DAQDevice
{
public:
    DAQDevice(QString identifier);
    //base information
    QString identifier;
    QString type;
    uint32_t serialNumber;
    bool isSimulated;
    //io
    QMap<unsigned int, QString> analogIn;
    QMap<unsigned int, QString> analogOut;
    QMap<unsigned int, QString> digitalIn;
    QMap<unsigned int, QString> digitalOut;

};

#endif // DAQDEVICE_H
