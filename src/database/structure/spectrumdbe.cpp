#include "spectrumdbe.h"

SpectrumDBE::SpectrumDBE() : DatabaseContent()
{
    type = "Spectrum";
}


SpectrumDBE::SpectrumDBE(QString name)
{
    type = "Spectrum";
    this->name = name;
}


SpectrumDBE::~SpectrumDBE()
{
}


varList SpectrumDBE::getVarList()
{
    varList v;
    return v;
}


void SpectrumDBE::initVars(varList list)
{
    for(varList::iterator it = list.begin(); it != list.end(); ++it)
    {
        if((*it).second.type == "group")
        {
            QString groupstring = (*it).second.name.trimmed();
            if(groupstring.compare("calibration", Qt::CaseInsensitive) == 0)
                group = Group::CALIBRATION;
            else if(groupstring.compare("sensitivity", Qt::CaseInsensitive) == 0)
                group = Group::SENSITIVITY;
            else if(groupstring.compare("error", Qt::CaseInsensitive) == 0)
                group = Group::ERROR;
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
