#include "laserenergy.h"

LaserEnergy::LaserEnergy()
{
    type = "LaserEnergy";
}


LaserEnergy::LaserEnergy(QString name)
{
    type = "LaserEnergy";
    this->name = name;
}


LaserEnergy::~LaserEnergy()
{

}


/*void LaserEnergy::addProperty(LaserEnergyEntry entry)
{
    lookupTable.push_back(entry);
}*/


varList LaserEnergy::getVarList()
{
    varList v;
    /*for(int i = 0; i < lookupTable.size(); i++)
    {
        Property p;
        p.name = "laser_energy_component";
        p.type = "const";
        p.description = this->description;
        p.identifier = this->filename;
        p.parameter[0] = lookupTable.at(i).set;
        p.parameter[1] = lookupTable.at(i).pos;
        p.parameter[2] = lookupTable.at(i).energy;
        v.insert(varPair(p.name, p));
    }*/

    for(QMap<double, QPair<double, double> >::iterator it = table.begin(); it != table.end(); ++it)
    {
        Property p;
        p.name = "laser_energy_component";
        p.type = "const";
        p.description = this->description;
        p.identifier = this->filename;
        p.parameter[0] = it.value().first;  //set
        p.parameter[1] = it.value().second; //pos
        p.parameter[2] = it.key();          //energy
        v.insert(varPair(p.name, p));
    }

    return v;
}


void LaserEnergy::initVars(varList list)
{
    for(varList::iterator it = list.begin(); it != list.end(); ++it)
    {
        /*LaserEnergyEntry lee;

        lee.set = (*it).second.parameter[0];
        lee.pos = (*it).second.parameter[1];
        lee.energy = (*it).second.parameter[2];

        addProperty(lee);*/

        table.insert((*it).second.parameter[2], QPair<double, double>((*it).second.parameter[0], (*it).second.parameter[1]));
    }
}
