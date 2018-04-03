#ifndef HTM_KOCK_SOOT_H
#define HTM_KOCK_SOOT_H

#include "../heattransfermodel.h"

class HTM_KockSoot : public HeatTransferModel
{
public:

    HTM_KockSoot();
    HTM_KockSoot(HTM_KockSoot const &other);

    virtual HTM_KockSoot* clone();

    void initModelVariables();

    double derivativeT(double T, double dp);

    double calculateMassLossEvap(double T, double dp);
    double calculateMassLossOx(double T, double dp);

    double calculateEvaporation(double T, double dp);    
    double calculateConduction(double T, double dp);
    double calculateRadiation(double T, double dp);

    double calculateOxidation(double T, double dp);
    double calculateAnnealing(double T, double dp);
    double calculateThermionicEmission(double T, double dp);
};

#endif // KOCK_SOOT_H
