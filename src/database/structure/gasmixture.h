#ifndef GASMIXTURE_H
#define GASMIXTURE_H

#include "../../calculations/constants.h"
#include "../databasecontent.h"
#include "gasproperties.h"

#include <cmath>
#include <QString>

/**
 * @brief The GasMixture class
 * TODO: do not store GasProperties pointer directly.
 * store ids instead!
 */
class GasMixture : public DatabaseContent
{
public:
    GasMixture();

    ~GasMixture();

    double molar_mass;
    void calculateMolarMass();

    Property therm_cond;    // thermal conductivity [W/m/K];
    Property L;             // mean free path [m]
    Property gamma_eqn;     // heat capacity ratio (individual expression) [-]

    double c_tg(double temperature);
    double C_p_mol(double temperature);
    double c_p_kg(double temperature);
    double gamma(double temperature);


    varList getVarList();
    void initVars(varList vl);

    // getters, setters ...
    void addGas(GasProperties* gas,double x);
    bool removeGas(GasProperties* gas);
    GasProperties* getGas(int index);
    QList<GasProperties*> getAllGases();


    int getNoGases();    
    double getX(int index);
    int contains(GasProperties* gas);

    void clearAllGases();

private:
    std::vector<GasProperties*> gases_;
    std::vector<double> x_;
};

#endif // GASMIXTURE_H
