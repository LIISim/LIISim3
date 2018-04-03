#include "htm_kock_soot.h"

#include "../../calculations/numeric.h"

HTM_KockSoot::HTM_KockSoot() : HeatTransferModel()
{
    identifier  = "kock";
    name        = "Kock - PhD thesis (2006)";
    description = "Boris Kock - Soot heat transfer model (evaporation via Clausius-Clapeyron) \n"
                  "\"Zeitaufgelöste Laserinduzierte Inkandeszenz (TR-LII): \n"
                  "Partikelgrößenmessung in einem Dieselmotor und einem Gasphasenreaktor\" (Cuvillier, 2006)\n\n"
                  "This model is identical to the model published in:\n"
                  "H. A. Michelsen, F. Liu, B. F. Kock, H. Bladh, A. Boiarciuc, M. Charwath, \n"
                  "T. Dreier, R. Hadef, M. Hofmann, J. Reimann, S. Will, P. E. Bengtsson, \n"
                  "H. Bockhorn, F. Foucher, K. P. Geigle, C. Mounaïm-Rousselle, \n"
                  "C. Schulz, R. Stirn, B. Tribalet, and R. Suntz, "
                  "\"Modeling laser-induced incandescence of soot: a summary and \n"
                  "comparison of LII models,\" Appl. Phys. B 87, 503-521 (2007).";

    version     = "-";

    sysfunc = SysFunc::dT_ddp;
}

// clone functions
HTM_KockSoot::HTM_KockSoot(HTM_KockSoot const &other) { cloneVars(other); }
HTM_KockSoot* HTM_KockSoot::clone() { return new HTM_KockSoot(*this); }


void HTM_KockSoot::initModelVariables()
{
    variables.clear();

    // general (used by default derivative functions)
    variables << &material.rho_p
              << &material.C_p_mol
              << &material.molar_mass;

    // evaporation
    variables << &material.H_v
              << &material.molar_mass_v
              << &material.theta_e
              << &material.p_v_ref
              << &material.T_v_ref;

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


/**
 * @brief HTM_KockSoot::derivativeT customized dT/dt for this heat-transfer model, due to
 * the individual internal energy change term
 * see also: H. A. Michelsen, M. A. Linne, B. F. Kock, M. Hofmann, B. Tribalet, and C. Schulz,
 * "Modeling laser-induced incandescence of soot: enthalpy changes during sublimation,
 * conduction, and oxidation," Appl. Phys. B 93, 645-656 (2008).
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_KockSoot::derivativeT(double T, double dp)
{
    // calculation of the integral of material.C_p_mol using trapezoid Newton-Cotes formula
    // The provided polynom is not valid for temperatures lower than 281.8 K what would cause
    // odd results. The right term of the denumerator is therefore not included in the model.
    /*
    double int_cp = Numeric::integrate_trapezoid(material.C_p_mol,
                                               0.0001, // 0 K
                                               T);

    // The following debug output can visualize the internal energy change term and the term
    // for evaporation, which are highly correlated.
    // int_cp * calculateMassLoss(T,dp) is much smaller than this->calculateEvaporation(T, dp);
    qDebug() <<  int_cp * calculateMassLoss(T,dp) <<  this->calculateEvaporation(T, dp);
    */

    return -1.0 *(this->calculateConduction(T, dp)
              + this->calculateEvaporation(T, dp)
              + this->calculateRadiation(T, dp)
              //+ int_cp * (-1.0) * calculateMassLoss(T,dp)
                 )
            / calculateMassFromDiameter(T, dp)
            / material.c_p_kg(T);
}


double HTM_KockSoot::calculateEvaporation(double T, double dp)
{
    if(!useEvaporation) return 0.0;

    //  // PhD Thesis Kock (page 11):    
    return -1.0 * material.H_v(T) / material.molar_mass_v(T) * calculateMassLossEvap(T, dp);
}


double HTM_KockSoot::calculateMassLossEvap(double T, double dp)
{
    if(!useEvaporation) return 0.0;

   // calculate vapor pressure by Clausius-Clapeyron equation
   // (PhD Thesis Kock (page 13)):
   // Molar mass of vapor (C3) see chapter 2.1.2

   //if(material.p_v_ref.isset)

   double vapor_density = material.vapor_pressure(T)
           * material.molar_mass_v(T)
           / Constants::R
           / T;

   return -0.25 * Constants::pi
           * material.theta_e(T)
           * dp * dp
           * material.c_tv(T)
           * vapor_density;
}

/**
 * @brief HTM_KockSoot::calculateConduction Conduction (according Equation 2.10)
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_KockSoot::calculateConduction(double T, double dp)
{
    if(!useConduction) return 0.0;

    double T_g = process.T_g;

    return material.alpha_T_eff(T)
            * Constants::pi
            * dp * dp
            * process.p_g / 8.0
            * gasmixture.c_tg(T_g)
            * (gasmixture.gamma(T_g) + 1.0)
            / (gasmixture.gamma(T_g) - 1.0)
            * (T / T_g - 1.0);
}

double HTM_KockSoot::calculateRadiation(double T, double dp)
{
   if(!useRadiation) return 0.0;

   return HeatTransferModel::calculateBaseRadiation(T, dp);
}

/*******************************
 *  NOT USED IN THIS MODEL
 *******************************/
double HTM_KockSoot::calculateMassLossOx(double T, double dp) {   return 0.0; }

double HTM_KockSoot::calculateOxidation(double T, double dp) {   return 0.0; }
double HTM_KockSoot::calculateAnnealing(double T, double dp) {   return 0.0; }
double HTM_KockSoot::calculateThermionicEmission(double T, double dp) {   return 0.0; }
