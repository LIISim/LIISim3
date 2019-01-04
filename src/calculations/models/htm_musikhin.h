#ifndef HTM_MUSIKHIN_H
#define HTM_MUSIKHIN_H

#include "../heattransfermodel.h"

class HTM_Musikhin : public HeatTransferModel
{
public:

    HTM_Musikhin();    
    HTM_Musikhin(HTM_Musikhin const &other);

    virtual HTM_Musikhin* clone();

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

#endif // HTM_MUSIKHIN_H
