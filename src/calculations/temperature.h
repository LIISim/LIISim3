#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "../database/structure/material.h"

#include "../signal/signal.h"
#include "../calculations/constants.h"

#include <QDebug> //!!!

class LIISettings;
class MPoint;


class Temperature
{

    /** @brief minimum temperature limit [K] */
    static double limitMin;
    /** @brief maximum temperature limit [K] */
    static double limitMax;

    /** @brief minimum C limit [-] */
    static double limitCMin;
    /** @brief maximum C limit [-] */
    static double limitCMax;

    /** @brief sourceEmOptions options for calculation of E(m) values    */
    static QStringList sourceEmOptions;

public:    

    static bool checkEmSource(Material& material, QString sourceEm, QMap<int, Channel> channels);

    static double getEmBySource(double lambda_m, Material& material, QString sourceEm);

    static double calcDrudeEm(int lambda, Material& material);

    static Signal calcTemperatureFromTwoColor( LIISettings& liiSettings,
                                               Material& material,
                                               QString sourceEm,
                                               MPoint*,
                                               int chId1,
                                               int chId2,
                                               Signal::SType inputSigType);

    static Signal calcTemperatureFromSpectrum( LIISettings& liiSettings,
                                               Material& material,
                                               QString sourceEm,
                                               MPoint*,
                                               Signal::SType inputSigType,
                                               QList<bool>* activeChannels,
                                               bool bpIntegration,
                                               bool weighting,
                                               bool autoStartC,
                                               int iterations,
                                               double startTemperature,
                                               double startC
                                               );

    static Signal calcTemperatureFromSpectrumTest( LIISettings& liiSettings,
                                               Material& material,
                                               QString sourceEm,
                                               MPoint*,
                                               Signal::SType inputSigType,
                                               QList<bool>* activeChannels,
                                               int iterations,
                                               double startTemperature,
                                               double startC
                                               );

    static double calcTwoColor(double v1, double v2,
                                      double lambda1, double lambda2,
                                      Material& material,
                                      QString sourceEm);


    static double calcPlanckIntensity(double lambda_m,
                                      double T,
                                      double C,
                                      Material& material,
                                      QString sourceEm);

    static double calcPlanckIntensityBandpass(int lambda,
                                              int bandwidth,
                                              double T,
                                              double C,
                                              Material& material,
                                              QString sourceEm);


};

#endif // TEMPERATURE_H
