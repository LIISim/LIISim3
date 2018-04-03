#ifndef LIISETTINGS_H
#define LIISETTINGS_H

#include <vector>

#include "../databasecontent.h"
#include "../../general/channel.h"



class LIISettings : public DatabaseContent
{
public:
    LIISettings();
    ~LIISettings(){}

    static const QString defaultFilterName;

    double laser_wavelength;

    channelList channels;
    filterList filters;

    Filter filter(const QString& identifier);
    Filter defaultFilter();

    bool isFilterDefined(const QString& identifier);

    varList getVarList();
    void initVars(varList);


};

#endif // LIISETTINGS_H
