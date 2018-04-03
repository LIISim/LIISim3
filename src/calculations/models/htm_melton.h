#ifndef HTM_MELTON_H
#define HTM_MELTON_H

#include "../heattransfermodel.h"

class HTM_Melton : public HeatTransferModel
{
public:
    HTM_Melton();
    HTM_Melton(HTM_Melton const &other);

    virtual HTM_Melton* clone();

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

#endif // HTM_MELTON_H
