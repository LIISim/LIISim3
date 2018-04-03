#ifndef TRANSMISSIONDBE_H
#define TRANSMISSIONDBE_H

#include <QString>

#include "../databasecontent.h"
#include "./signal/spectrum.h"

class TransmissionDBE : public DatabaseContent
{
public:

    enum Group
    {
        BANDPASS,
        DICHROIC,
        ND,
        OTHER
    };

    TransmissionDBE();
    TransmissionDBE(QString name);
    ~TransmissionDBE();

    varList getVarList();
    void initVars(varList list);

    Group group;

    Spectrum spectrum;
};

#endif // TRANSMISSIONDBE_H
