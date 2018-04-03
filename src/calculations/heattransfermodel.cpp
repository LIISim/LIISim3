#include "heattransfermodel.h"

HeatTransferModel::HeatTransferModel()
{
    identifier  = "none";
    name        = "not set";
    description = "not set";
    version     = "0.0";

    sysfunc = dT_ddp;

    useConduction   = true;
    useEvaporation  = true;
    useRadiation    = true;
}


void HeatTransferModel::cloneVars(HeatTransferModel const &other)
{
    identifier  = other.identifier;
    name        = other.name;
    description = other.description;
    version     = other.version;

    sysfunc     = other.sysfunc;

    variables   = other.variables;

    material        = other.material;
    material_spec   = other.material_spec;
    gasmixture      = other.gasmixture;

    process.p_g     = other.process.p_g;
    process.T_g     = other.process.T_g;

    useConduction   = other.useConduction;
    useEvaporation  = other.useEvaporation;
    useRadiation    = other.useRadiation;

    //QString htm_addr;
    //htm_addr.sprintf("%p %i\n", this, *this); // QString("%0 %1").arg(ptr).arg(*ptr);
    //qDebug() << htm_addr << name;
}


void HeatTransferModel::setProcessConditions(double p_g, double  T_g)
{
    process.p_g = p_g;
    process.T_g = T_g;
}


void HeatTransferModel::setParameter(double parameter_list[], int index)
{
    switch(index)
    {
        case 0: /* d_p - particle diameter -> do nothing */  break;
        case 1: process.T_g = parameter_list[index];   break;
        case 2: process.p_g = parameter_list[index];   break;
    }
}


double HeatTransferModel::getParameter(int index)
{
    switch(index)
    {
        case 0: /* d_p - particle diameter -> do nothing */  break;
        case 1: return process.T_g;   break;
        case 2: return process.p_g;   break;
    }
}


bool HeatTransferModel::checkAvailability()
{
    // get variables from subclass
    initModelVariables();

    if(variables.size() == 0)
    {
        QString msg_empty;

        msg_empty = QString("<b>%0: variable list empty! </b><br>"
                            "Please revise constructor of heat transfer model class if \"variables\" is defined").arg(name);

        MSG_ONCE_WINDOW("HTM_checkAvailability", 1,
                        "Heat Transfer Model: Database variables availability check",
                        msg_empty, ERR);
        return false;
    }

    // mandatory variables used in basic equations
    variables << &material.rho_p;

    bool check = true;
    QString missing_varlist;

    // check all variables and return list of missing variables
    QSetIterator<Property*> var(variables);
    while(var.hasNext())
    {
        Property* prop = var.next();

        if(!prop->available)
        {
            check = false;
            missing_varlist.append(prop->name);
            missing_varlist.append("<br>");
        }
    }

    if(check == false)
    {
        QString msg = QString("<b>%0</b>: <br> One or more variables are not defined in selected database files: <br><br>"
                              "Material: <b>%2</b> <br>"
                              "GasMixture: <b>%3</b> and included GasProperties <br><br>"
                              "<i>Missing variables:</i> <br> %1<br>"
                              "Please check all selected database files and add the missing "
                              "variables before starting another simulation/fit.")
                .arg(name)
                .arg(missing_varlist)
                .arg(material.name)
                .arg(gasmixture.name);

        MSG_ONCE_WINDOW("HTM_checkAvailability", 2,
                        "Heat Transfer Model: Database variables availability check",
                        msg, ERR);
    }

    return check;
}


/*********************************
 *      HELPER FUNCTIONS
 *********************************/


double HeatTransferModel::calculateMassFromDiameter(double T, double dp)
{
    return Constants::pi / 6.0 * pow(dp, 3) * material.rho_p(T);
}


double HeatTransferModel::calculateDiameterFromMass(double T, double mp)
{
    return cbrt(6.0 * mp / Constants::pi / material.rho_p(T));
}



/*************************************************************
 *      Differential equations and numeric functions
 *************************************************************/

/**
 * @brief HeatTransferModel::derivativeT
 *
 *      dT/dt = - (Qcond + Qevap + Qrad) / ( mass * c_p_kg)
 *
 * @param T
 * @param dp particle diameter [m]
 * @return dT/dt
 */
double HeatTransferModel::derivativeT(double T, double dp)
{
//    qDebug()<<  T << " - " << dp << " : Cond: "
//            <<  this->calculateConduction(T, dp) << " - Evap: "
//              << this->calculateEvaporation(T, dp) << " - Rad: "
//              << this->calculateRadiation(T, dp) << " - ML: "
//              << this->calculateMassLossEvap(T, dp) * material.c_p_kg(T) * T;

    return -1.0 *(calculateConduction(T, dp)
              + calculateEvaporation(T, dp)
              + calculateRadiation(T, dp)
              )
            / calculateMassFromDiameter(T, dp)
            / material.c_p_kg(T);
}


/**
 * @brief HeatTransferModel::derivativeDp
 *
 * *   m        = pi/6*rho*dp^3        (particle mass)
 * *   dm/d(dp) = pi/2*rho*dp^2        (correlation of mass and diameter)
 * *   dm/dt    = mdot                 (mass loss per time unit)
 * *   d(rho_p)dT is much smaller compared to evaporation and thus assumed as zero
 *
 *  d(dp)/dt = dm/dt * d(dp)/dm     (particle diameter change per time unit)
 *
 *  => d(dp)/dt = mdot * 2/(pi*rho*dp^2)
 *
 * @param T particle temperature [K]
 * @param dp particle diameter [m]
 * @return d(dp)/dt
 */
double HeatTransferModel::derivativeDp(double T, double dp)
{
    return (calculateMassLossEvap(T, dp)
            + calculateMassLossOx(T, dp))
            * 2.0
            / (Constants::pi * material.rho_p(T) * dp*dp);

//    // derivative of linear function rho_p = a0 + a1 * T
//    double drho_dT = material.rho_p.parameter[1];

//    double dpdT = 2.0 / (Constants::pi * material.rho_p(T) * dp*dp)
//            * (-1.0 * this->calculateMassLoss(T, dp)
//               - Constants::pi / 6 * dp*dp*dp * drho_dT);

//    qDebug() << "dpdT: " << dpdT << -1.0 * this->calculateMassLoss(T, dp) << Constants::pi / 6 * dp*dp*dp * drho_dT;
//    return dpdT;
}


double HeatTransferModel::derivativeMp(double T, double dp)
{
    return calculateMassLossEvap(T, dp)
            + calculateMassLossOx(T, dp);
}


/**
 * @brief HeatTransferModel::ode_sys
 *       system function for ODE solver
 *       http://www.boost.org/doc/libs/1_53_0/libs/numeric/odeint/doc/html/boost_numeric_odeint/concepts/system.html
 *       rk4.do_step( sys , x , t , dt );
 * @param x     [0] temperature T
 *              [1] particle diameter d_p or particle mass m_p
 * @param dxdt  derivative of:
 *              [0] temperature: dx/dt = f(x,t) == dT/dt
 *              [1] particle diameter: d(d_p)/dt or particle mass: d(m_p)/dt
 * @param t     time
 */
void HeatTransferModel::ode_sys(const state_type &x,
                               state_type &dxdt,
                               double t)
{        
    // x[0]:  temperature
    // x[1]:  particle mass
    if(sysfunc == dT_dM)
    {
        double dp = calculateDiameterFromMass(x[0], x[1]);

        dxdt[0] = derivativeT(x[0], dp);  // dT/dt
        dxdt[1] = derivativeMp(x[0], dp); // d(m_p)/dt
    }
    // x[0]:  temperature
    // x[1]:  particle diameter
    else // dT_ddp
    {
        dxdt[0] = derivativeT(x[0], x[1]);  // dT/dt
        dxdt[1] = derivativeDp(x[0], x[1]); // d(d_p)/dt
    }

    //qDebug() << "ODE_Sys: " << t << x[0] << dxdt[0];
    //qDebug() << "ODE_Sys: " << t << x[0] << x[1] << dxdt[0] << dxdt[1];
}


/******************************************************************
 *      BASE HEAT TRANSFER EQUATIONS
 *      (for implementation in individual heat transfer models
 *
 *      Usage:
 *          - HeatTransferModel::calculateBaseEvaporation(T, dp);
 *          - HeatTransferModel::calculateBaseMassLoss(T, dp);
 *          - HeatTransferModel::calculateBaseConduction(T, dp);
 *          - HeatTransferModel::calculateBaseRadiation(T, dp);
 ******************************************************************/

/**
 * @brief HeatTransferModel::calculateBaseEvaporation
 * @param T particle temperature [K]
 * @param dp particle diameter [m]
 * @return heat transfer q [J/s]
 */
double HeatTransferModel::calculateBaseEvaporation(double T, double dp)
{
    // enthalpy of formation - material.H_v(T) - [J/mol]

    return -1.0 * material.H_v(T)
            / material.molar_mass_v(T)
            * calculateMassLossEvap(T, dp);
}


/**
 * @brief HeatTransferModel::calculateBaseMassLossEvap
 * Source: H. A. Michelsen et al. - Appl. Phys. B 87, 503-521 (2007).
 * @param T particle temperature [K]
 * @param dp particle diameter [m]
 * @return mdot absolute value [kg/s]
 */
double HeatTransferModel::calculateBaseMassLossEvap(double T, double dp)
{
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
 * @brief HeatTransferModel::calculateBaseConduction
 * Calculates heat transfer by conduction (free molecular regime)
 * Source: H. A. Michelsen et al. - Appl. Phys. B 87, 503-521 (2007).
 * @param T particle temperature [K]
 * @param dp particle diameter [m]
 * @return heat transfer q [J/s]
 */
double HeatTransferModel::calculateBaseConduction(double T, double dp)
{
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
 * @brief HeatTransferModel::calculateBaseRadiation (Stefan-Boltzmann law)
 * @param T particle temperature [K]
 * @param dp particle diameter [m]
 * @return heat transfer q [J/s]
 */
double HeatTransferModel::calculateBaseRadiation(double T, double dp)
{
    return Constants::pi
            * dp * dp
            * material.eps(T)
            * Constants::sigma
            * (pow(T,4) - pow(process.T_g,4));
}
