#include "opticalproperty.h"


OpticalProperty::OpticalProperty()
{
   name         = "notSet";
   unit         = "notSet";
   description  = "no description";
   source       = "notSet";

   // inFile will be set to true, if property exists in database file
   inFile = false;
   // available will be set to true, if property exists and type != "notSet"
   available = false;

}


/**
 *  When called, optical property gives value for
 *      temperature T (double)
 *      wavelength (int)
 *
 *  Example:
 *      Em(300, 442) gives E(m) at 300 K and 442 nm
 */
double OpticalProperty::operator()(double T, int wavelength)
{
    return values[wavelength](T);
}


/**
 * @brief OpticalProperty::exists
 * @param wavelength
 * @return true if wavelength exists
 */
bool OpticalProperty::exists(int wavelength)
{   
    return values.find(wavelength) != values.end();
}


OpticalProperty OpticalProperty::assignCheck(OpticalProperty iprop, varList var_list)
{    
    Property property;
    int wavelength;

    // load default values
    OpticalProperty optProp;
    optProp.name        = iprop.name;
    optProp.unit        = iprop.unit;
    optProp.description = iprop.description;

    if(var_list.count(iprop.name) == 0)
    {
        // if element does not exist in file:
        // set available and inFile = false and pass default values
        optProp.inFile    = false;
        optProp.available = false;

        return optProp;
    }
    else
    {
        // at least one entry is available
        optProp.inFile    = true;
        optProp.available = true;

        // gives iterator for range of same var_name
        std::pair<varList::iterator,varList::iterator> pit;
        pit = var_list.equal_range(iprop.name);

        // save wavelength dependency in optProp.values
        for (varList::iterator it = pit.first;
               it != pit.second;
               ++it)
        {

            wavelength = int(it->second.parameter[0]);
            property   = it->second;

            // assign unit/desc also for Property child objects
            property.unit        = iprop.unit;
            property.description = iprop.description;

            property.inFile     = true;
            property.available  = true;

            optProp.values.insert(std::pair<int, Property>(wavelength, property));
        }

        return optProp;
    }
}


QString  OpticalProperty::toString() const
{
    QString s = "name: " + name + "\t number of values: ";

    QString novals;
    novals.sprintf(" %d ", values.size());

    s = s + novals.toLatin1().data()+"\n";
    return s;
}


QString OpticalProperty::valueAsString() const
{
    QString res;
    res.sprintf("%d values", values.size());
    return res;
}

