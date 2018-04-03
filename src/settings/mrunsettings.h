#ifndef MRUNSETTINGS_H
#define MRUNSETTINGS_H

#include "settingsbase.h"
#include "../general/picoscopecommon.h"

class MRun;

/**
 * @brief The MRunSettings class stores metadata of a measurement
 * run object. It defines what information should be saved and
 * provides methods for saving/loading run settings
 */
class MRunSettings : public SettingsBase
{
    Q_OBJECT

public:

    explicit MRunSettings(MRun* run = 0, QObject *parent = 0);
    explicit MRunSettings(const QString & fname);
    ~MRunSettings();

    bool save(const QString & dirpath);
    bool load(const QString & dirpath);

    QString runName();
    QString liiSettingsFname();
    QString description();
    QString filterName();
    double laserFluence();
    double pmtChannelGainVoltage(int channelID);

    //PicoScope parameter
    PSRange ps_range();
    PSCoupling ps_coupling();
    double ps_offset();
    double ps_collectionTime();
    double ps_sampleIntervall();
    double ps_presample();

    double laser_setpoint();
    double laser_position();

    QString acquisition_mode();

    QMap<QString, QVariant> userDefinedParameters;


protected:

    virtual void init() {}

private:

    MRun* run;

signals:

public slots:
};

#endif // MRUNSETTINGS_H
