#ifndef MRUN_H
#define MRUN_H

#include "../models/dataitem.h"
#include <QList>
#include <QVector>
#include <QString>
#include <QMutex>
#include "mpoint.h"
#include <QDebug>


#include "../io/signaliorequest.h"
#include "../database/structure/liisettings.h"
#include "../general/picoscopecommon.h"
#include "mruncalculationstatus.h"
#include "tempcalcmetadata.h"

class ProcessingPluginConnector;
class ProcessingChain;
class MRunGroup;
class TemperatureProcessingChain;


/**
 * @brief Measurement Run
 * @ingroup Hierachical-Data-Model
 * @details A Measurement Run is a storage class for multiple signals.
 * For each Measurement Point the original data and the preprocessed data
 * is stored.
 *
 * data vector usage (used by the DataItem dataChanged-signal/slot mechanism)
 *
 * emit dataChanged(idx)
 *
 * 0: run name
 * 1: run color
 * 2: -
 * 3: description
 * 4: LIISettings name
 * 5: laser fluence [mJ/mm^2]
 * 6: pmt channel gain voltages [?]
 * 7: filter name
 * 8: import informations changed
 */
class MRun : public DataItem
{
    Q_OBJECT
private:

    /** @brief laser fluence [mJ/mm^2] */
    double m_laserFluence;

    /** @brief name of current filter (filters are defined in LIISettings)*/
    QString m_filterSetting;

    /** @brief PMT channel gain voltage for each channel */
    QVector<double> m_pmtGainVoltage;

    /** @brief Measured PMT channel gain voltage for each channel */
    QVector<double> m_pmtReferenceGainVoltage;

    /** @brief list of signal data before preprocessing */
    QList<MPoint*> pre;  // all signals of run (unprocessed)

    /** @brief list of signal data after proprocessing */
    QList<MPoint*> post; // all signals after preprocessing

    /** @brief list of indices of valid signal data */
    QList<int> validPointsIdx; // indices of valid MPoints

    /** @brief used to lock all list operations */
    QMutex mutexMPointLists;

    QMutex settingsMutex;

    /** @brief number of channels */
    int channelCount_raw_abs;

    /** @brief processing chain for raw data signals */
    ProcessingChain * pchainRaw;

    /** @brief processing chain for absolute data signals */
    ProcessingChain * pchainAbs;

    /** @brief processing chain for temperature signal */
    TemperatureProcessingChain * pchainTemp;

    /** @brief stores current calculation status and warn/error status messages*/
    MRunCalculationStatus* m_calcState;

    bool busy;

    /** @brief stores the import request which has been used to create this mrun */
    SignalIORequest m_importRequest;

    LIISettings m_liiSettings;

    /** @brief list of temperature channel ids */
    QList<int> temperatureChannelIDs;


    //PicoScope parameter

    /** @brief selected picoscope range at capturing time in volts (+-)*/
    //double ps_range;

    PSRange ps_range;

    QVector<double> m_psRange;

    /** @brief selected picoscope coupling at capturing time
     *  Available combinations: dc = false              : AC
     *                          dc = true + 50r = false : DC 1 MOhm
     *                          dc = true + 50r = true  : DC 50 Ohm
     */
    //bool ps_coupling_dc;
    //bool ps_coupling_50r;

    PSCoupling ps_coupling;

    /** @brief picoscope offset in volts (+-) */
    double ps_offset;
    QVector<double> m_psOffset;

    /** @brief collection time in seconds */
    double ps_collectionTime;

    /** @brief sample interval in seconds */
    double ps_sampleInterval;

    /** @brief presample in percent */
    double ps_presample;

    QString acquisitionMode;

    double laserPosition;

    double laserSetpoint;

public:
    MRun(QString name, int channelCount_raw_abs, MRunGroup* group = 0);
    MRun(QString name, QString filename, int channelCount_raw_abs, MRunGroup *group = 0);
    virtual ~MRun();


    /** @brief name of Measurement Run */
    QString name;

    QString filename;

    QString getName();
    void setName(const QString & name);
    int getNoChannels(Signal::SType stype);

    MRunGroup* group();

    void setDescription(const QString & descr);
    QString description();

    void setLaserFluence(double laserFluence);
    double laserFluence();

    void setPmtGainVoltage(int channelID, double voltage);
    double pmtGainVoltage(int channelID);

    void setPmtReferenceGainVoltage(int channelID, double voltage);
    double pmtReferenceGainVoltage(int channelID);

    bool setFilter(const QString & transmission);
    QString filterIdentifier();
    Filter filter();

    MPoint* getCreatePre(int idx);
    MPoint* getPre(int idx);
    MPoint* getPost(int idx);
    MPoint* getPostPre(int idx);
    int sizeAllMpoints();

    inline void setBusy(bool state){busy = state;}
    inline bool isBusy(){return busy;}

    ProcessingChain * getProcessingChain(Signal::SType stype);

    MRunCalculationStatus* calculationStatus(){return m_calcState;}
    void updateCalculationStatus();

    MPoint* getValidPre(int idx);
    MPoint* getValidPost(int idx);
    MPoint* getValidPostPre(int idx);
    int getRealIdxFromValidList(int validListIdx);
    int sizeValidMpoints();


    void updateValidList();

    void copyProcessingStepsFrom(MRun* mrun);

    SignalIORequest importRequest();
    void setImportRequest(const SignalIORequest& irq);

    QList<ProcessingPluginConnector*> ppcs(bool globalsOnly = false);

    int addTemperatureChannel(int tchid = -1);
    void removeTemperatureChannel(int ch_id);

    QList<int> channelIDs(Signal::SType stype);
    bool isValidChannelID(int id, Signal::SType stype);

    LIISettings liiSettings();
    void setLiiSettings(const LIISettings& liiSettings);

    //double psRange(){return ps_range;}
    //void setPSRange(double range){ps_range = range;}

    PSRange psRange(){return ps_range;}
    void setPSRange(PSRange range){ps_range = range;}

    double psRange(int channelID);
    void setPSRange(int channelID, double range);

    //bool psCouplingDC(){return ps_coupling_dc;}
    //bool psCoupling50R(){return ps_coupling_50r;}
    //void setPSCouplingDC(bool coupling_dc){ps_coupling_dc = coupling_dc;}
    //void setPSCoupling50R(bool coupling_50r){ps_coupling_50r = coupling_50r;}

    PSCoupling psCoupling(){return ps_coupling;}
    void setPSCoupling(PSCoupling coupling){ps_coupling = coupling;}

    double psOffset(){return ps_offset;}
    void setPSOffset(double offset){ps_offset = offset;}

    double psOffset(int channelID);
    void setPSOffset(int channelID, double offset);

    double psCollectionTime(){return ps_collectionTime;}
    void setPSCollectionTime(double collectionTime){ps_collectionTime = collectionTime;}

    double psSampleInterval(){return ps_sampleInterval;}
    void setPSSampleInterval(double sampleInterval){ps_sampleInterval = sampleInterval;}

    double psPresample(){return ps_presample;}
    void setPSPresample(double presample){ps_presample = presample;}

    QMap<QString, QVariant> userDefinedParameters;

    QString getAcquisitionMode(){return acquisitionMode;}
    void setAcquisitionMode(QString mode){acquisitionMode = mode;}

    double getLaserPosition(){return laserPosition;}
    void setLaserPosition(double value){laserPosition = value;}

    double getLaserSetpoint(){return laserSetpoint;}
    void setLaserSetpoint(double value){laserSetpoint = value;}

    // index: Temperature-Channel ID
    QMap<int, TempCalcMetadata> tempMetadata;

signals:

    /**
     * @brief processingFinished this signal is emitted if the mruns signal data
     * has been processed
     */
    void processingFinished();

    /**
     * @brief channelCountChanged This signal is emitted when additional channels have
     * been added to the MRun.
     * @param stype signal type
     * @param count new channel count for signal type stype
     */
    void channelCountChanged(Signal::SType stype, int count);

    /**
     * @brief LIISettingsChanged This signal is emitted when the
     * LIISettings of this MRun have been modified or updated.
     */
    void LIISettingsChanged();

    /**
     * @brief MRunDetailsChanged This signal is emitted when the
     * MRunDetails like PMT gain, description, filter... of this MRun have been modified or updated.
     */
    void MRunDetailsChanged();

private slots:
    void onDBmodified(int id = -1);

    void onPluginGoneDirty();
};

#endif // MRUN_H
