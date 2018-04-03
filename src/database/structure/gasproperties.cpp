#include "gasproperties.h"

GasProperties::GasProperties()
{
    type = "Gas";

    C_p_mol.name        = "C_p_mol";
    C_p_mol.unit        = "[J/mol/K]";
    C_p_mol.description = "Molar heat capacity";

    molar_mass.name         = "molar_mass";
    molar_mass.unit         = "[kg/mol]";
    molar_mass.description  = "Molar mass";

    alpha_T.name        = "alpha_T";
    alpha_T.unit        = "[-]";
    alpha_T.description = "Thermal accomodation coefficient - needs to be defined depending on pair Material-Gas";

    zeta.name        = "zeta";
    zeta.unit        = "[-]";
    zeta.description = "Number of active internal degrees of freedom of the gas molecule";

    // counts how many times the gas is used in a mixture
    // Used in GasEditor::onRemoveCurrentSelectionFromList()
    // if number is larger than zero, warning window is shown before deletion
    no_mixture_references = 0;
}


void GasProperties::initVars(varList vl)
{
    C_p_mol     = Property::assignCheck(C_p_mol, vl);
    molar_mass  = Property::assignCheck(molar_mass, vl);
    alpha_T     = Property::assignCheck(alpha_T, vl);
    zeta        = Property::assignCheck(zeta, vl);
}


varList GasProperties::getVarList()
{
    varList vars;
    vars.insert(varPair(C_p_mol.name, C_p_mol));
    vars.insert(varPair(molar_mass.name, molar_mass));
    vars.insert(varPair(alpha_T.name, alpha_T));
    vars.insert(varPair(zeta.name, zeta));

    return vars;
}


/** @brief GasProperties::c_tg
* @param temperature - Temperature of the surrounding gas [K]
* @return thermal velocity of gas molecules [m/s]
*/
double GasProperties::c_tg(double temperature)
{
   return sqrt(8.0
               * Constants::k_B
               * Constants::N_A
               * temperature
               / Constants::pi
               / this->molar_mass());
}


/**
 * @brief GasProperties::c_p_kg
 * @param temperature - Temperature of the surrounding gas [K]
 * @return specific heat capacity - [J/kg/K]
 */
double GasProperties::c_p_kg(double temperature)
{
    return this->C_p_mol(temperature) / this->molar_mass();
}

