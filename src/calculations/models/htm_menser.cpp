#include "htm_menser.h"

HTM_Menser::HTM_Menser()
{    
    identifier  = "menser";
    name        = "Menser - Appl.Phys.B (2016) 122:277";
    description = "Menser - Silicon heat transfer model\n"
                  "Laser‑induced incandescence from laser‑heated silicon nanoparticles"
                  "Appl. Phys. B (2016) 122:277"
                  "DOI 10.1007/s00340-016-6551-4";

    version     = "-";

    sysfunc = SysFunc::dT_dM;
}


// clone functions
HTM_Menser::HTM_Menser(HTM_Menser const &other) { cloneVars(other); }
HTM_Menser* HTM_Menser::clone() { return new HTM_Menser(*this); }


void HTM_Menser::initModelVariables()
{
    variables.clear();

    // general (used by default derivative functions)
    variables << &material.rho_p
              << &material.C_p_mol
              << &material.molar_mass;

    // evaporation
    variables << &material.H_v
              << &material.molar_mass_v
              << &material.p_v;

    // gas properties
    QList<GasProperties*> gases = gasmixture.getAllGases();
    for(int i = 0; i < gases.size(); i++)
    {
        GasProperties* gas = gases.at(i);

        variables << &gas->molar_mass
                  << &gas->alpha_T
                  << &gas->zeta;
    }
}


/**
 * @brief HTM_Menser::calculateEvaporation Sublimation
 * @param T     particle temperature [K]
 * @param dp    particle diameter [m]
 * @return
 */
double HTM_Menser::calculateEvaporation(double T, double dp)
{
    if(!useEvaporation) return 0.0;

        return -1.0 * material.H_v(T)
            / material.molar_mass_v(T)
            * calculateMassLossEvap(T, dp);
    }


    /**
     * @brief HTM_Menser::calculateMassLossEvap Conduction
     * @param T     particle temperature [K]
     * @param dp    particle diameter [m]
     * @return
     */
    double HTM_Menser::calculateMassLossEvap(double T, double dp)
    {
        if(!useEvaporation) return 0.0;


        // Kelvin equation

        // F. Millot, V. Sarou-Kanian, J. C. Rifflet, and B. Vinet,
        // "The surface tension of liquid silicon at high temperature,"
        // Materials Science and Engineering: A 495, 8-13 (2008).

        // "Fig. 6 recapitulates the evolution of σ in the temperature range
        // (1550–2400 K) in Ar and Ar/2.5% H2 and for different droplet
        // masses. The surface tension can be expressed by the following
        // linear regression:
        // σ = (732 ± 8) − (0.086 ± 0.004)(T − 1685) (6)
        // with σ in mN/m, and T in K"

        double gamma = (732.0 - 0.086*(T - 1685.0))*1E-3; // [N/m]

        // Specific gas constant [J/kg/K]
        double R_s = Constants::R / material.molar_mass_v(T);

        double p_v_kelvin = material.p_v(T)
                                * exp( 4.0 * gamma
                                        / (dp * material.rho_p(T)
                                            * R_s
                                            * T)
                                       );

        return  -1.0 * Constants::pi * dp * dp * 0.25
                        * p_v_kelvin
                        / Constants::k_B
                        / T
                        * material.c_tv(T)
                        / Constants::N_A * material.molar_mass_v(T);
    }


    /**
     * @brief HTM_Menser::calculateConduction Conduction
     * @param T     particle temperature [K]
     * @param dp    particle diameter [m]
     * @return
     */
    double HTM_Menser::calculateConduction(double T, double dp)
    {
        if(!useConduction) return 0.0;


        // calculate sum for all gases in mixture
        double sum = 0.0;

        QList<GasProperties*> gases = gasmixture.getAllGases();

        for(int i = 0; i < gases.size(); i++)
        {
            GasProperties* gas = gases.at(i);

            sum = sum + (gasmixture.getX(i)
                        * gas->c_tg(process.T_g)
                        * gas->alpha_T()
                        * (2.0 + gas->zeta() / 2.0));

            //qDebug() << "HTM_Menser: cond: i: "   << i << gasmixture.getX(i) << gases.at(i)->name << " sum:" << sum;
        }

        return Constants::pi * dp *dp * 0.25
                    * process.p_g / Constants::k_B / process.T_g
                    * sum
                    * Constants::k_B * (T - process.T_g);
    }


    /**
     * @brief HTM_Menser::calculateRadiation
     * @param T     particle temperature [K]
     * @param dp    particle diameter [m]
     * @return
     */
    double HTM_Menser::calculateRadiation(double T, double dp)
    {
        if(!useRadiation) return 0.0;

        // not used in this model
        return 0.0;
    }


    /*******************************
     *  NOT USED IN THIS MODEL
     *******************************/
    double HTM_Menser::calculateMassLossOx(double T, double dp) {   return 0.0; }

    double HTM_Menser::calculateOxidation(double T, double dp) {   return 0.0; }
    double HTM_Menser::calculateAnnealing(double T, double dp) {   return 0.0; }
    double HTM_Menser::calculateThermionicEmission(double T, double dp) {   return 0.0; }
