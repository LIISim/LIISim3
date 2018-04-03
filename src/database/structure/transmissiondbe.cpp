#include "transmissiondbe.h"

TransmissionDBE::TransmissionDBE()
{
    type = "Transmission";
}


TransmissionDBE::TransmissionDBE(QString name)
{
    type = "Transmission";
    this->name = name;
}


TransmissionDBE::~TransmissionDBE()
{
}


varList TransmissionDBE::getVarList()
{
    varList v;
    return v;
}


void TransmissionDBE::initVars(varList list)
{
    for(auto it = list.begin(); it != list.end(); ++it)
    {
        if((*it).second.type == "group")
        {
            QString groupstring = (*it).second.name.trimmed();
            if(groupstring.compare("bandpass", Qt::CaseInsensitive) == 0)
                group = Group::BANDPASS;
            else if(groupstring.compare("dichroic", Qt::CaseInsensitive) == 0)
                group = Group::DICHROIC;
            else if(groupstring.compare("nd", Qt::CaseInsensitive) == 0)
                group = Group::ND;
            else
                group = Group::OTHER;
        }
        else
        {
            spectrum.xData.append((*it).second.parameter[0]);
            spectrum.yData.append((*it).second.parameter[1]);
        }
    }
}
