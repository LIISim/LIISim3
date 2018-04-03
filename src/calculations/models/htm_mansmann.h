#ifndef HTM_MANSMANN_H
#define HTM_MANSMANN_H

#include "../heattransfermodel.h"

class HTM_Mansmann : public HeatTransferModel
{
public:

    HTM_Mansmann();
    HTM_Mansmann(HTM_Mansmann const &other);

    virtual HTM_Mansmann* clone();

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

#endif // MANSMANN_H
