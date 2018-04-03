#ifndef GASPROPERTIES_H
#define GASPROPERTIES_H

#include "../../calculations/constants.h"
#include "../databasecontent.h"

class GasProperties : public DatabaseContent
{
public:

    GasProperties();
    ~GasProperties(){}
    /** @var Property GasProperties::C_p_mol.
     *  @brief [J/mol/K] - specific heat capacity (isobaric)
     */
    Property C_p_mol;

    /** @var Property GasProperties::molar_mass.
     *  @brief [kg/mol] - molar mass
     */
    Property molar_mass;

    /**
     * @var Property GasProperties::alpha_T
     * @brief alpha_T [-] - accomodation coefficient
     */
    Property alpha_T;

    /**
     * @var Property GasProperties::zeta
     * @brief zeta [-] degrees of freedom of gas molecules
     */
    Property zeta;


    /**
     * @brief counter for the number of GasMixture references for this gas
     * (larger than zero -> warning on deletion)
     */
    int no_mixture_references;

    varList getVarList();
    void initVars(varList vl);

    // individual equations for gas
    double c_tg(double temperature);
    double c_p_kg(double temperature);
};

#endif // GASPROPERTIES_H
