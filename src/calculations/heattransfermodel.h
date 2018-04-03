#ifndef HEATTRANSFERMODEL_H
#define HEATTRANSFERMODEL_H


#include "../database/structure/gasmixture.h"
#include "../database/structure/material.h"
#include "../logging/msghandlerbase.h"

class HeatTransferModel
{

public:
    HeatTransferModel();
    virtual ~HeatTransferModel(){}

    virtual HeatTransferModel* clone() = 0;
    void cloneVars(HeatTransferModel const &other);

    typedef std::vector<double> state_type;

    // default ODE system function type
    // determines which variables are used for calculation of derivatives
    enum SysFunc { dT_dM, dT_ddp };

    QString identifier;
    QString name;
    QString version;
    QString description;

    bool useConduction;
    bool useEvaporation;
    bool useRadiation;

    // initialization
    void setProcessConditions(double p_g, double  T_g);
    void setParameter(double parameter_list[], int index);
    double getParameter(int index);

    void setMaterial(const Material & material){ this->material = material; }
    void setMaterialSpec(const Material & material){ this->material_spec = material; }
    void setGasMix(const GasMixture & gasmix){ this->gasmixture = gasmix; }

    bool checkAvailability();


    // calculations
    double calculateMassFromDiameter(double T, double dp);
    double calculateDiameterFromMass(double T, double mp);

    virtual double derivativeT(double T, double dp);
    virtual double derivativeDp(double T, double dp);
    virtual double derivativeMp(double T, double dp);

    SysFunc sysfunc;
    inline int sysFunc() { return sysfunc; }

    void ode_sys(const state_type &x,
                state_type &dxdt,
                double t);

    double calculateBaseEvaporation(double T, double dp);
    double calculateBaseMassLossEvap(double T, double dp);
    double calculateBaseConduction(double T, double dp);
    double calculateBaseRadiation(double T, double dp);


    // pure virtual functions for child classes (mandatory for implementation)
    virtual void initModelVariables() = 0;

    virtual double calculateMassLossEvap(double T, double dp) = 0;
    virtual double calculateMassLossOx(double T, double dp) = 0;

    virtual double calculateEvaporation(double T, double dp) = 0;
    virtual double calculateConduction(double T, double dp) = 0;
    virtual double calculateRadiation(double T, double dp) = 0;
    virtual double calculateOxidation(double T, double dp) = 0;
    virtual double calculateAnnealing(double T, double dp) = 0;
    virtual double calculateThermionicEmission(double T, double dp) = 0;

protected:
    Material material;
    Material material_spec; // for spectroscpic calculations
    GasMixture gasmixture;

    QSet<Property*> variables; // list of variables needed for this model

    struct pcond {
        double p_g;
        double T_g;
    } process;

};

#endif // HEATTRANSFERMODEL_H
