#ifndef FITSETTINGS_H
#define FITSETTINGS_H

#include "settingsbase.h"

#include <QList>

#include "fitparameter.h"

/**
 * @brief The FitSettings class
 */
class FitSettings : public SettingsBase
{
    Q_OBJECT

public:
    explicit FitSettings(QObject *parent = 0);
    ~FitSettings();

    void write(QSettings& dest);
    virtual void writeToXML(QXmlStreamWriter &w);

    inline QList<FitParameter> fitParameters(){return m_fitParameters;}
    void setFitParameters(QList<FitParameter> & fparams);
    int getNoEnabledFitParameters();

    static QList<FitParameter> availableFitParameters();

    inline bool bandpassIntegration() { return m_bp_integration; }
    void setBandpassIntegrationActive(bool active);

    inline bool weightingActive() { return m_weighting; }
    void setWeightingActive(bool active);

    inline QString sourceEm() { return m_sourceEm; }
    void setSourceEm(QString sourceEm);


    QString toString();

private:
    void init();

    /**
     * @brief m_fitParameters
     * 0: diameter
     * 1: gas temperature
     */
    QList<FitParameter> m_fitParameters;

    bool m_bp_integration;
    bool m_weighting;
    QString m_sourceEm;
};

#endif // FITSETTINGS_H
