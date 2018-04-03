#ifndef SPEKTRUMDBE_H
#define SPEKTRUMDBE_H

#include <QString>

#include "../databasecontent.h"
#include "./signal/spectrum.h"

class SpectrumDBE : public DatabaseContent
{
public:

    enum Group
    {
        CALIBRATION,
        SENSITIVITY,
        ERROR,
        OTHER
    };

    SpectrumDBE();
    SpectrumDBE(QString name);
    ~SpectrumDBE();

    varList getVarList();
    void initVars(varList);

    Group group;

    Spectrum spectrum;

};

#endif // SPEKTRUMDBE_H
