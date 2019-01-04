#include "htm_musikhin.h"

HTM_Musikhin::HTM_Musikhin() : HeatTransferModel()
{
    identifier  = "musikhin";
    name        = "Musikhin";
    description = "Base heat transfer model for testing/development";
    version     = "-";

    sysfunc = SysFunc::dT_ddp;
}


// clone functions
HTM_Musikhin::HTM_Musikhin(HTM_Musikhin const &other) { cloneVars(other); }
HTM_Musikhin* HTM_Musikhin::clone() { return new HTM_Musikhin(*this); }


void HTM_Musikhin::initModelVariables()
{
    variables.clear();

    // general (used by default derivative functions)
    variables << &material.rho_p
              << &material.C_p_mol
              << &material.molar_mass;

    // evaporation
    variables << &material.H_v
              << &material.molar_mass_v;
    // conduction
    variables << &material.alpha_T_eff;
    // gas->molar_mass
    // gas->C_p_mol

    // radiation
    variables << &material.eps;

    // gas properties
    QList<GasProperties*> gases = gasmixture.getAllGases();
    for(int i = 0; i < gases.size(); i++)
    {
        GasProperties* gas = gases.at(i);

        variables << &gas->molar_mass
                  << &gas->C_p_mol;
    }
}


double HTM_Musikhin::calculateEvaporation(double T, double dp)
{
    if(!useEvaporation) return 0.0;

    //return HeatTransferModel::calculateBaseEvaporation(T, dp);
    return material.H_v(T) * material.molar_mass_v(T) * this->calculateMassLossEvap(T, dp);
}


double HTM_Musikhin::calculateConduction(double T, double dp)
{
    if(!useConduction) return 0.0;

    double T_g = process.T_g;

    if(T > 3550.0)
        qDebug() << "Musikhin Test" << material.alpha_T_eff(T);

    //return HeatTransferModel::calculateBaseConduction(T, dp);
    return material.alpha_T_eff(T)
            * Constants::pi
            * dp * dp
            * process.p_g / 8.0
            * gasmixture.c_tg(T_g)
            * (gasmixture.gamma(T_g) + 1.0)
            / (gasmixture.gamma(T_g) - 1.0)
            * (T / T_g - 1.0);
}


double HTM_Musikhin::calculateMassLossEvap(double T, double dp)
{
    if(!useEvaporation) return 0.0;

    //return HeatTransferModel::calculateBaseMassLossEvap(T, dp);
    return 0.0;

//    return 0.25 * Constants::pi
//            * material.theta_e(T)
//            * dp * dp
//            * material.c_tv(T)
//            * material.vapor_density(T);
}


double HTM_Musikhin::calculateRadiation(double T, double dp)
{
    if(!useRadiation) return 0.0;

   //return HeatTransferModel::calculateBaseRadiation(T, dp);
    return Constants::pi
            * dp * dp
            * material.eps(T)
            * Constants::sigma
            * (pow(T,4) - pow(process.T_g,4));
}


/*******************************
 *  NOT USED IN THIS MODEL
 *******************************/
double HTM_Musikhin::calculateMassLossOx(double T, double dp) {   return 0.0; }

double HTM_Musikhin::calculateOxidation(double T, double dp) {   return 0.0; }
double HTM_Musikhin::calculateAnnealing(double T, double dp) {   return 0.0; }
double HTM_Musikhin::calculateThermionicEmission(double T, double dp) {   return 0.0; }

