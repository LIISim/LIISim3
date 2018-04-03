#ifndef HTM_LIU_H
#define HTM_LIU_H

#include "../heattransfermodel.h"

class HTM_Liu : public HeatTransferModel
{
public:
    HTM_Liu();
    HTM_Liu(HTM_Liu const &other);

    virtual HTM_Liu* clone();

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

#endif // HTM_LIU_H
