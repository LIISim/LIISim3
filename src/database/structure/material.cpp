#include "material.h"
#include "../../calculations/constants.h"
#include <math.h>

Material::Material()
{
    type = "Material";

    /*
     *  molar_mass      [kg/mol]    - Molar mass
     *  molar_mass_v    [kg/mol]    - Molar mass of vapor species
     *  rho_p           [kg/m^3]    - Particle density
     *  H_v             [J/mol]     - Evaporation enthalpy
     *
     *  alpha_T_eff     [-]         - Effective thermal accommodation coefficient for heat conduction
     *  theta_e         [-]         - Thermal accommodation coefficient for evaporation
     *
     *  C_mol_p         [J/mol/K]   - Molar heat capacity
     *  p_v             [Pa]        - Vapor pressure
     *
     *  eps             [-]         - Total emissivity of a single particle (for Stefan-Boltzmann law)
     *  Em              [-]         - E(m) - Absorption function at fixed wavelength lambda
     *  Em_func         [-]         - E(m) - Absorption function as function of wavelength lambda
     *  omega_p         [rad/s]     - plasma frequency (Drude)
     *  tau             [s]         - relaxation time (Drude)
     */

    molar_mass.name         = "molar_mass";
    molar_mass.unit         = "[kg/mol]";
    molar_mass.description  = "Molar mass";

    molar_mass_v.name         = "molar_mass_v";
    molar_mass_v.unit         = "[kg/mol]";
    molar_mass_v.description  = "Molar mass of (material) vapor species";

    rho_p.name          = "rho_p";
    rho_p.unit          = "[kg/m^3]";
    rho_p.description   = "Particle density";

    H_v.name            = "H_v";
    H_v.unit            = "[J/mol]";
    H_v.description     = "Evaporation enthalpy";

    alpha_T_eff.name        = "alpha_T_eff";
    alpha_T_eff.unit        = "[-]";
    alpha_T_eff.description = "Effective thermal accommodation coefficient for heat conduction";

    theta_e.name        = "theta_e";
    theta_e.unit        = "[-]";
    theta_e.description = "Thermal accommodation coefficient for evaporation";

    C_p_mol.name            = "C_p_mol";
    C_p_mol.unit            = "[J/mol/K]";
    C_p_mol.description     = "Molar heat capacity";

    p_v.name                = "p_v";
    p_v.unit                = "[Pa]";
    p_v.description         = "Vapor pressure";

    p_v_ref.name           = "p_v_ref";
    p_v_ref.unit           = "[Pa]";
    p_v_ref.description    = "Clausius-Clapeyron - Reference pressure";

    T_v_ref.name           = "T_v_ref";
    T_v_ref.unit           = "[K]";
    T_v_ref.description    = "Clausius-Clapeyron - Reference temperature";

    eps.name            = "eps";
    eps.unit            = "[-]";
    eps.description     = "Total emissivity of the particle cloud (for Stefan-Boltzmann law)";

    Em.name             = "Em";
    Em.unit             = "[-]";
    Em.description      = "E(m) - Absorption function at fixed wavelength lambda (for two-color pyrometry)";

    Em_func.name        = "Em_func";
    Em_func.unit        = "[-]";
    Em_func.description = "E(m) - Absorption function as function of wavelength lambda (for spectral fit)";

    omega_p.name        = "omega_p";
    omega_p.unit        = "[rad/s]";
    omega_p.description = "Plasma frequency (Drude)";

    tau.name            = "tau";
    tau.unit            = "[s]";
    tau.description     = "Relaxation time (Drude)";
}


void Material::initVars(varList var_list)
{
    type = "Material";

    alpha_T_eff = Property::assignCheck(alpha_T_eff, var_list);
    C_p_mol     = Property::assignCheck(C_p_mol, var_list);

    H_v          = Property::assignCheck(H_v, var_list);
    molar_mass   = Property::assignCheck(molar_mass, var_list);
    molar_mass_v = Property::assignCheck(molar_mass_v, var_list);
    rho_p        = Property::assignCheck(rho_p, var_list);

    theta_e      = Property::assignCheck(theta_e, var_list);
    p_v          = Property::assignCheck(p_v, var_list);

    Em           = OpticalProperty::assignCheck(Em, var_list);
    Em_func      = Property::assignCheck(Em_func, var_list);

    // optional properties (required by specific spectroscopic models)
    omega_p     = Property::assignCheckOptional(omega_p, var_list);
    tau         = Property::assignCheckOptional(tau, var_list);

    // optional properties (required by specific heat transfer models)
    eps        = Property::assignCheckOptional(eps, var_list);
    p_v_ref    = Property::assignCheckOptional(p_v_ref, var_list);
    T_v_ref    = Property::assignCheckOptional(T_v_ref, var_list);
}

varList Material::getVarList()
{
    varList var_list;
    var_list.insert(varPair(alpha_T_eff.name, alpha_T_eff));
    var_list.insert(varPair(C_p_mol.name, C_p_mol));
    var_list.insert(varPair(H_v.name, H_v));
    var_list.insert(varPair(molar_mass.name, molar_mass));
    var_list.insert(varPair(molar_mass_v.name, molar_mass_v));
    var_list.insert(varPair(rho_p.name, rho_p));

    var_list.insert(varPair(p_v.name, p_v));
    var_list.insert(varPair(p_v_ref.name, p_v_ref));
    var_list.insert(varPair(T_v_ref.name, T_v_ref));
    var_list.insert(varPair(theta_e.name, theta_e));

    for(varListConstIterator it = Em.values.begin(); it != Em.values.end(); it++) {
        var_list.insert(varPair(it->second.name, it->second));
    }
    var_list.insert(varPair(Em_func.name, Em_func));
    var_list.insert(varPair(eps.name, eps));

    // Drude
    var_list.insert(varPair(omega_p.name, omega_p));
    var_list.insert(varPair(tau.name, tau));

    return var_list;
}



/**
 * @brief Material::c_p_kg
 * @param temperature - Temperature of the particle [K]
 * @return specific heat capacity at temperature [J/kg/K]
 */
double Material::c_p_kg(double temperature)
{
    return this->C_p_mol(temperature) / this->molar_mass();
}


/**
 * @brief Material::c_tv
 * @param temperature - Temperature of the vapor (= particle surface temperature) [K]
 * @return thermal velocity of vapor molecules [m/s]
 */
double Material::c_tv(double temperature)
{
    /* NOTE:
     * with R = k_B * N_A this is equivalent to:
     *  sqrt(8.0 * Constants::R * temperature / Constants::pi / this->molar_mass)
     * Simplification:
     *  material.c_tv(T) / 4.0 * sqrt(Constants::pi)
     * is equivalent to:
     *  sqrt(Constants::R * temperature / material.molar_mass / 2.0)
     */

    return sqrt(8.0 * Constants::k_B
                * temperature
                / Constants::pi
                / this->molar_mass_v(temperature)
                * Constants::N_A);
}

/**
 * @brief Material::vapor_pressure
 * @param temperature - Temperature of the vapor (= particle surface temperature) [K]
 * @return vapor pressure
 */
double Material::vapor_pressure(double temperature)
{
    // !!!!!!!!
    // if any changes are made to this function,
    // please check and update also MaterialEditor::updateCurrentView()
    // !!!!!!!!

    // check if vapor pressure formula was given or reference values for Clausius-Clapeyron
    if(p_v.available)
    {
        return p_v(temperature);
    }
    else if(p_v_ref.available && T_v_ref.available && H_v.available)
    {
        return p_v_clausius_clapeyron(temperature);
    }
    else
    {
        //!!! TODO: throw exception
        return 0.0;
    }
}


/**
 * @brief Material::p_v_clausius_clapeyron
 * @param temperature
 * @return vapor pressure [Pa] from Clausius-Clapeyron equation using reference point
 * at pressure "p_v_ref" [Pa] and temperature "T_v_ref" [K]
 */
double Material::p_v_clausius_clapeyron(double temperature)
{    
    return p_v_ref()
            * exp( -1.0 * H_v(temperature)
                   / Constants::R
                   * (1.0 / temperature - 1.0 / T_v_ref())
                   );
}


