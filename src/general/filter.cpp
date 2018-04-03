#include "filter.h"
#include <QDebug>
#include "../../core.h"


const QString Filter::filterNotSetIdentifier = QString("filter not set");

Filter::Filter() {}
Filter::Filter(QString identifier) : identifier(identifier)
{

}


/**
 * @brief Filter::getTransmission
 *  TODO: check if wavelength is valid, return default value (1.0 ?)
 * @param wavelength
 * @return
 */
double Filter::getTransmission(int wavelength)
{   
    // handle default filter ("no Filter") and empty filter lists here!
    // Info: default filter is set in LIISettings database structure
    if(identifier == LIISettings::defaultFilterName || this->list.size() == 0)
        return 1.0;

    // handle "filter not set" state
    // This means that no filter settings have been defined during
    // MRunSettings generation (e.g. using Labview) and that the
    // transmission value is undefined!
    // !!! Do not confuse this withh the "no filter", which is
    // a well defined filter (transmission = 1.0) !!!
    if(identifier == filterNotSetIdentifier)
        return 0.0;

    std::multimap<int,double>::iterator res= list.find(wavelength);

    if(res == list.end())
    {
        MSG_ASYNC(QString("Filter::getTransmission: "
                          "wavelength %0 not defined "
                          "for filter '%1'! Return transmission = 1.0")
                  .arg(wavelength)
                  .arg(identifier),
                  WARNING);
        return 1.0;
    }
    else
        return res->second*0.01;

}

/**
 * @brief Filter::getTransmissions returns a list of all filter transmission values
 * of this filter
 * @return
 */
QList<double> Filter::getTransmissions()
{
    QList<double> res;

    for (std::multimap<int,double>::iterator it= list.begin(); it != list.end(); ++it)
        res << it->second;

    return res;
}


