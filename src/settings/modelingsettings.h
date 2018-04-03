#ifndef MODELINGSETTINGS_H
#define MODELINGSETTINGS_H

#include "settingsbase.h"

#include "../database/structure/material.h"
#include "../database/structure/liisettings.h"
#include "../database/structure/gasmixture.h"
#include "../calculations/heattransfermodel.h"


class Core;

class ModelingSettings : public SettingsBase
{
    Q_OBJECT

    friend class Core;

public:
    explicit ModelingSettings(QObject *parent = 0);

    ~ModelingSettings();

    inline Material material(){ return m_material; }
    inline Material materialSpec(){ return m_material_spec; }
    inline GasMixture gasMixture(){ return m_gasMixture; }
    inline LIISettings defaultLiiSettings(){ return m_defaultLIISettings; }
    inline HeatTransferModel* heatTransferModel(){ return m_heatTransferModel; }

    /// @brief returns process pressure [Pascal]
    inline double processPressure(){ return m_processPressure; }

    bool setMaterial(const QString & fname);
    bool setMaterialSpec(const QString & fname);
    bool setGasMixture(const QString & fname);
    bool setLIISettings(const QString & fname);
    bool setHeatTransferModel(int index);
    bool setProcessPressure(double pressure);

    void copyFrom(ModelingSettings* other);

private:

    void init();

    QString key_gasMixture;

    QString key_heatTransferModel;
    QString key_liiSettings;
    QString key_material;
    QString key_material_spec; // spectroscopic material (TemperatureCalculator)
    QString key_processPressure;    

    Material m_material;
    Material m_material_spec;
    GasMixture m_gasMixture;
    LIISettings m_defaultLIISettings;
    HeatTransferModel *m_heatTransferModel;

    /// @brief holds process pressure [Pascal]
    double m_processPressure;

    // default values
    double default_processPressure;

    QString default_gasMixture;
    QString default_heatTransferModel;
    QString default_liiSettings;
    QString default_material;
    QString default_material_spec;

signals:

    /**
     * @brief materialChanged This signal is emitted when the material
     * selection changed
     */
    void materialChanged();
    void materialSpecChanged();

public slots:
    void setupConnections(Core *c);

private slots:
    void onDBContentChanged(int db_ident);

};

#endif // MODELINGSETTINGS_H
