#ifndef MATERIAL_H
#define MATERIAL_H


#include "../databasecontent.h"
#include "opticalproperty.h"

class Material :  public DatabaseContent
{
public:

    Material();
    ~Material(){}

    Property H_v;
    Property molar_mass;
    Property molar_mass_v;
    Property rho_p;

    Property alpha_T_eff;
    Property theta_e;

    Property C_p_mol;
    Property p_v;

    Property eps;
    OpticalProperty Em;
    Property Em_func;

    // optional
    Property omega_p;
    Property tau;

    Property p_v_ref;
    Property T_v_ref;


    varList getVarList();
    void initVars(varList vl);

    double c_p_kg(double temperature);
    double c_tv(double temperature);

    double vapor_pressure(double temperature);

    double p_v_clausius_clapeyron(double temperature);

};

#endif // MATERIAL_H
