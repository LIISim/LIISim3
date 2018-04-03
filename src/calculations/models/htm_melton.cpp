#include "htm_melton.h"

HTM_Melton::HTM_Melton()
{
    identifier  = "melton";
    name        = "Melton";
    description = "Melton - Soot heat transfer model\n"
                  "Constraint model from:"
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
HTM_Melton::HTM_Melton(HTM_Melton const &other) { cloneVars(other); }
HTM_Melton* HTM_Melton::clone() { return new HTM_Melton(*this); }


void HTM_Melton::initModelVariables()
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
    variables << &material.alpha_T_eff
              << &gasmixture.therm_cond
              << &gasmixture.L;

    // gas properties
    QList<GasProperties*> gases = gasmixture.getAllGases();
    for(int i = 0; i < gases.size(); i++)
    {
        GasProperties* gas = gases.at(i);

        variables << &gas->C_p_mol; // used by gasmixture.gamma()
    }
}


/**
 * @brief HTM_Melton::calculateEvaporation Sublimation (according Equation (4))
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Melton::calculateEvaporation(double T, double dp)
{
    if(!useEvaporation) return 0.0;

    // This model uses molar mass of solid species

    double test = -1.0 * material.H_v(T) / material.molar_mass(T) * calculateMassLossEvap(T, dp);

    //qDebug() << "cEvap: " << T << dp << test;

    return test;
}


/**
 * @brief HTM_Melton::calculateMassLoss Conduction (according Equation (5))
 * Source: H. A. Michelsen, F. Liu, B. F. Kock, H. Bladh, A. Boiarciuc, M. Charwath, T. Dreier, R. Hadef, M. Hofmann, J. Reimann, S. Will, P. E. Bengtsson, H. Bockhorn, F. Foucher, K. P. Geigle, C. Mounaïm-Rousselle, C. Schulz, R. Stirn, B. Tribalet, and R. Suntz, "Modeling laser-induced incandescence of soot: a summary and comparison of LII models," Appl. Phys. B 87, 503-521 (2007).
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Melton::calculateMassLossEvap(double T, double dp)
{
    if(!useEvaporation) return 0.0;

   // calculate vapor pressure by Clausius-Clapeyron equation
   // Table 3: Equation c
   // Molar mass of sublimed clusters (C3)

    return  -1.0 * Constants::pi * dp * dp
            * material.molar_mass_v(T)
            * material.theta_e(T)
            * material.vapor_pressure(T)
            / Constants::R / T
            * sqrt(0.5 * Constants::R * T
                   / material.molar_mass_v(T));
}


/**
 * @brief HTM_Melton::calculateConduction Conduction (according Equation (6),(7) and (8))
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Melton::calculateConduction(double T, double dp)
{
    if(!useConduction) return 0.0;

    // heat-capacity ratio (defined in GasMixture class)
    // gasmix.gamma(T) = c_p_mol(T) / (c_p_mol(T) - Constants::R);

    double T_g = process.T_g;

    // Eucken correction f to the thermal conductivity
    double f = 0.25 * (9.0 * gasmixture.gamma(T_g) - 5.0);

    // geometry-dependent heat-transfer factor G
    double G = 8.0 * f / (material.alpha_T_eff(T) * (gasmixture.gamma(T_g) + 1.0));

    return 2.0 * gasmixture.therm_cond(T_g)
            * Constants::pi * dp * dp
            / (dp + G * gasmixture.L(T_g))
            * (T - T_g);
}


/**
 * @brief HTM_Melton::calculateRadiation radiative cooling neglected in this model
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Melton::calculateRadiation(double T, double dp)
{
    if(!useRadiation) return 0.0;

    return 0.0;
}


/*******************************
 *  NOT USED IN THIS MODEL
 *******************************/
double HTM_Melton::calculateMassLossOx(double T, double dp) {   return 0.0; }

double HTM_Melton::calculateOxidation(double T, double dp) {   return 0.0; }
double HTM_Melton::calculateAnnealing(double T, double dp) {   return 0.0; }
double HTM_Melton::calculateThermionicEmission(double T, double dp) {   return 0.0; }

