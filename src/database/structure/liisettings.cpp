#include "liisettings.h"

const QString LIISettings::defaultFilterName = QString("no Filter");

LIISettings::LIISettings()
{
    type = "LIISettings";
    laser_wavelength    = 0.0;

    defaultFilter();
}


void LIISettings::initVars(varList var_list)
{
    laser_wavelength = Property::assignCheckValueDouble(var_list, "laser_wavelength");
}


/**
 * @brief LIISettings::filter returns a filter from
 * filter list with given identifier
 * @param filter identifier
 * @return Filter from filterlist
 */
Filter LIISettings::filter(const QString &identifier)
{
    // ignore filter request when filter is not set!
    if(identifier == Filter::filterNotSetIdentifier)
        return Filter(Filter::filterNotSetIdentifier);

    // search filter in list
    for(int i = 0; i < filters.size(); i++)
        if(filters[i].identifier == identifier)
            return filters[i];

    // if filter is not present within list -> undefined filter!
    return Filter(identifier);

    // return default filter if identifier is not found
    // return defaultFilter();
}


/**
 * @brief LIISettings::isFilterDefined checks if a filter with the given identifier
 * is defined within LIISettings
 * @param identifier requested filter identifier
 * @return true if filter is defined, false if not defined
 */
bool LIISettings::isFilterDefined(const QString &identifier)
{
    for(int i = 0; i < filters.size(); i++)
        if(filters[i].identifier == identifier)
            return true;
    return false;
}

/**
 * @brief LIISettings::defaultFilter returns default filter
 * (100% transmission)
 * @return
 */
Filter LIISettings::defaultFilter()
{
    for(int i = 0; i < filters.size(); i++)
        if(filters[i].identifier == defaultFilterName)
            return filters[i];

    // add a default 100% Filter if 100% filter cannot be found
    Filter defaultFilter;
    defaultFilter.identifier = defaultFilterName;
    filters << defaultFilter;
    return defaultFilter;
}


varList LIISettings::getVarList()
{
    varList vars;
    Property p;

    p.name = "laser_wavelength";
    p.parameter[0] = laser_wavelength;
    p.description = " ";
    vars.insert(varPair(p.name, p));

    return vars;
}
