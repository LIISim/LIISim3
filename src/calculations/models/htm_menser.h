#ifndef HTM_MENSER_H
#define HTM_MENSER_H

#include "../heattransfermodel.h"

class HTM_Menser : public HeatTransferModel
{
public:
    HTM_Menser();
    HTM_Menser(HTM_Menser const &other);

    virtual HTM_Menser* clone();

    void initModelVariables();

    double calculateMassLossEvap(double T, double dp);
    double calculateMassLossOx(double T, double dp);

    double calculateEvaporation(double T, double dp);
    double calculateConduction(double T, double dp);
    double calculateRadiation(double T, double dp);

    double calculateOxidation(double T, double dp);
    double calculateAnnealing(double T, double dp);
    double calculateThermionicEmission(double T, double dp);
};

#endif // HTM_MENSER_H
