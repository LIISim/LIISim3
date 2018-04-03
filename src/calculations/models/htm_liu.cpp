#include "htm_liu.h"

#include "../../calculations/numeric.h"

HTM_Liu::HTM_Liu()
{
    identifier  = "liu";
    name        = "Liu - Appl.Phys.B - Review(2007)";
    description = "Liu - Soot heat transfer model\n"
                  "Independent model from:"
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
HTM_Liu::HTM_Liu(HTM_Liu const &other) { cloneVars(other); }
HTM_Liu* HTM_Liu::clone() { return new HTM_Liu(*this); }


void HTM_Liu::initModelVariables()
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
              << &material.p_v;

    // conduction
    variables << &material.alpha_T_eff
              << &gasmixture.gamma_eqn;
            // gas->molar_mass

    // radiation
    variables << &material.eps;

    // gas properties
    QList<GasProperties*> gases = gasmixture.getAllGases();
    for(int i = 0; i < gases.size(); i++)
    {
        GasProperties* gas = gases.at(i);

        variables << &gas->molar_mass;
    }
}


/**
 * @brief HTM_Liu::calculateEvaporation Sublimation (according Equation (22))
 * Source: Michelsen et.al - Appl. Phys. B 87, 503-521 (2007).
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Liu::calculateEvaporation(double T, double dp)
{
    if(!useEvaporation) return 0.0;

    return -1.0 * material.H_v(T)
            / material.molar_mass_v(T)
            * calculateMassLossEvap(T, dp);
}


/**
 * @brief HTM_Liu::calculateMassLossEvap Conduction (according Equation (xxx))
 * Source: Michelsen et.al - Appl. Phys. B 87, 503-521 (2007).
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Liu::calculateMassLossEvap(double T, double dp)
{
    if(!useEvaporation) return 0.0;

    double K = 0.5;

    // Uses T-dependent molar mass of vapor species
    double molar_mass_v = material.molar_mass_v(T);

    return  -1.0 * Constants::pi * dp * dp
            * molar_mass_v
            * material.theta_e(T)
            * material.vapor_pressure(T)
            / Constants::R / T
            * pow(
                (0.5 * Constants::R * T
                   / molar_mass_v
                   / Constants::pi), K);
}


/**
 * @brief HTM_Liu::calculateConduction Conduction (according Equation (33) and (38))
 * Source: Michelsen et.al - Appl. Phys. B 87, 503-521 (2007).
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Liu::calculateConduction(double T, double dp)
{
    if(!useConduction) return 0.0;

    double T_g = process.T_g;

    /**
     *  heat-capacity ratio (defined in GasMixture class)
     *
     *      -   this model uses a fit to tabulated values of
     *          the heat capacity ratio (flame or air)
     *
     *      -   gamma function needs to be manually set in GasMixture database file
     *
     *      -   if gamma is not defined in GasMixure, gamma will be calculated according:
     *              gasmix.gamma(T) = c_p_mol(T) / (c_p_mol(T) - Constants::R);
     *
     *      -    Please see GasMixture::gamma(T) for more information
     **/


    // Equation (38):
    // in this model gamma is replaces by gamma_mean
    // (values of gamma rely on particle temperature):
    // integration used 10 data points between Tg and Tp

    auto fp = std::bind(&GasMixture::gamma, gasmixture, std::placeholders::_1);
    double gamma_mean = Numeric::integrate_trapezoid_func(fp, T_g, T, 10)
                        / (T - T_g);

//    double gamma_mean = gasmix.gamma(T_g);

//    qDebug() << "HTM_Liu: Tg - " << T_g << "- gamma_mean: " << gamma_mean
//                << "gamma(T_g): " << gasmixture.gamma(T_g)
//                << " gamma(T): "<< gasmixture.gamma(T);

    // Equation (33)
    return Constants::pi
              * dp * dp
              * material.alpha_T_eff(T)
              * process.p_g
              / 2.0 / T_g
              * sqrt(
                    Constants::R
                    * T_g
                    / 2.0 / Constants::pi
                    / gasmixture.molar_mass)
              * (gamma_mean + 1.0)
              / (gamma_mean - 1.0)
              * (T - T_g);
}


/**
 * @brief HTM_Liu::calculateRadiation Equation (18)
 * Source: Michelsen et.al - Appl. Phys. B 87, 503-521 (2007).
 * material.eps is used for calculation
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Liu::calculateRadiation(double T, double dp)
{
    if(!useRadiation) return 0.0;

    return 199.0 * pow(Constants::pi, 3)
           * pow(dp, 3)
           * pow((Constants::k_B * T), 5)
           * material.eps(T)
           / pow(Constants::h, 4)
           / pow(Constants::c_0, 3);
}


/*******************************
 *  NOT USED IN THIS MODEL
 *******************************/
double HTM_Liu::calculateMassLossOx(double T, double dp) {   return 0.0; }

double HTM_Liu::calculateOxidation(double T, double dp) {   return 0.0; }
double HTM_Liu::calculateAnnealing(double T, double dp) {   return 0.0; }
double HTM_Liu::calculateThermionicEmission(double T, double dp) {   return 0.0; }
