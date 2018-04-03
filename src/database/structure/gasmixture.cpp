#include "gasmixture.h"

GasMixture::GasMixture()
{
    type = "GasMixture";

    therm_cond.name = "therm_cond";
    therm_cond.unit = "[W/m/K]";
    therm_cond.description = "Thermal conductivity of gas mixture";

    L.name = "L";
    L.unit = "[m]";
    L.description = "Mean free path";

    gamma_eqn.name = "gamma_eqn";
    gamma_eqn.unit = "[-]";
    gamma_eqn.description = "Heat capacity ratio (optional)";

}

GasMixture::~GasMixture()
{
    //clearAllGases();    
}


void GasMixture::initVars(varList var_list)
{
    therm_cond  = Property::assignCheckOptional(therm_cond, var_list);
    L           = Property::assignCheckOptional(L, var_list);
    gamma_eqn   = Property::assignCheckOptional(gamma_eqn, var_list);
}


varList GasMixture::getVarList()
{
    varList v;
    for(int i = 0; i < gases_.size(); i++)
    {
        Property p;
        p.name = "gas_component";
        p.type = "const";
        p.unit = "[-]";
        p.description = gases_.at(i)->name;
        p.identifier = gases_.at(i)->filename;
        p.parameter[0] = x_.at(i);
        v.insert(varPair(p.name,p));
    }

    v.insert(varPair(therm_cond.name, therm_cond));
    v.insert(varPair(L.name, L));
    v.insert(varPair(gamma_eqn.name, gamma_eqn));

    return v;
}


/*******************************
 * Calculation of Properties
 *******************************/

/**
 * @brief GasMixture::calculateMolarMass calculates molar mass of gas mixture on initialization
 */
void GasMixture::calculateMolarMass()
{
    this->molar_mass = 0.0;

    double xsum = 0.0;

    for(int i = 0; i < gases_.size(); i++)
    {
        xsum = xsum + x_[i];
        this->molar_mass = this->molar_mass + x_[i] * gases_[i]->molar_mass();
    }

    // check if sum of mole fractions is equal 1
    if (xsum < 1 || xsum > 1)
    {
        //!!! TODO: throw exception or warning
    }
}


/**
 * @brief GasMixture::c_tg
 * @param temperature - Temperature of the surrounding gas [K]
 * @return thermal velocity of gas molecules [m/s]
 */
double GasMixture::c_tg(double temperature)
{
    return sqrt(8.0 * Constants::k_B
                * temperature
                * Constants::N_A
                / Constants::pi
                / this->molar_mass);
}


/**
 * @brief GasMixture::C_p_mol
 * @param temperature - Temperature of the surrounding gas [K]
 * @return molar heat capacity - [J/mol/K]
 */
double GasMixture::C_p_mol(double temperature)
{
    double out = 0;

    for(unsigned int i=0; i < gases_.size(); i++)
    {
        out = out + x_[i] * gases_[i]->C_p_mol(temperature);
    }
    return out;
}


/**
 * @brief GasMixture::c_p_kg
 * @param temperature - Temperature of the surrounding gas [K]
 * @return specific heat capacity - [J/kg/K]
 */
double GasMixture::c_p_kg(double temperature)
{
    return this->C_p_mol(temperature) / this->molar_mass;
}


/**
 * @brief GasMixture::gamma Heat capacity ratio gamma = Cp / (Cp−R)
 * @param temperature          - Temperature of the surrounding gas [K]
 * @return heat capacity ratio - [-]
 */
double GasMixture::gamma(double temperature)
{
    // check if individual equation is defined in GasMixture file
    if(gamma_eqn.available)
        return gamma_eqn(temperature);
    else
        return this->C_p_mol(temperature) / (this->C_p_mol(temperature) - Constants::R);
}



/*******************************
 * Database helper functions
 *******************************/

/**
 * @brief GasMixture::GasMixture
 * @param gas  - GasProperties objects
 * @param x    - Molar fraction values for the gases
 *                Sum of all elements needs to be 1
 *                Example: {0.3, 0.2, 0.5}
 */
void GasMixture::addGas(GasProperties *gas, double x)
{
    gases_.push_back(gas);
    x_.push_back(x);

    gas->no_mixture_references++;
}


/**
 * @brief removed gas from mixture
 * @param gas
 * @return true if gas was part of mixture and has been removed, false if gas is not part of this mixture
 */
bool GasMixture::removeGas(GasProperties *gas)
{
    int idx = contains(gas);
    if(idx>=0)
    {
        gases_.erase(gases_.begin()+idx);
        x_.erase(x_.begin()+idx);
        gas->no_mixture_references--;
    //    qDebug() << "GasMixture.removeGas(): remove gas at"<<idx;
        return true;
    }
    // else
    //    qDebug() <<  "GasMixture.removeGas(): gas not in list";
    return false;
}


/**
 * @brief GasMixture::clearAllGases remove all gases from gas mixture
 */
void GasMixture::clearAllGases()
{
    //decrease reference count on all gases in mixture
    for(int i = 0; i< (int)gases_.size();i++)
        gases_.at(i)->no_mixture_references--;

    x_.clear();
    gases_.clear();
}


int GasMixture::getNoGases()
{
    return gases_.size();
}


GasProperties* GasMixture::getGas(int index)
{
    if(index >= (int)gases_.size())
    {
     //   qDebug() << "index exceeds boundaries !!!";
        return NULL;
    }
    return gases_.at(index);
}


QList<GasProperties*> GasMixture::getAllGases()
{
    QList<GasProperties*> list;

    // return empty list
    if(gases_.size() == 0)
        return list;


    for(int i = 0; i< (int)gases_.size();i++)
        list.append(gases_.at(i));

    return list;
}


/**
 * @brief GasMixture::getX
 * @param index
 * @return x composition
 */
double GasMixture::getX(int index)
{
    if(index >= (int)x_.size())
    {
    //    qDebug() << "index exceeds boundaries !!!";
        return 0.0;
    }
    return x_.at(index);
}


/**
 * @brief checks if gas is contained in mixture
 * @param gas
 * @return index of gas in list or -1 if gas is not in list
 */
int GasMixture::contains(GasProperties *gas)
{
    int sz = gases_.size();
    for(int i=0; i< sz; i++)
      //  if(gases_.at(i)->filename == gas->filename)
        if(gases_.at(i) == gas)
           return i;
    return -1;
}





