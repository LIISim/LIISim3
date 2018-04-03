#ifndef LASERENERGY_H
#define LASERENERGY_H

#include <QString>
#include <QMap>
#include <QPair>

#include "../databasecontent.h"
#include "laserenergyproperties.h"

class LaserEnergy : public DatabaseContent
{
public:
    LaserEnergy();
    LaserEnergy(QString name);
    ~LaserEnergy();

    //void addProperty(LaserEnergyEntry entry);

    varList getVarList();
    void initVars(varList);

    std::vector<LaserEnergyEntry> lookupTable;

    //Energy, (set, pos)
    QMap<double, QPair<double, double> > table;
};

#endif // LASERENERGY_H
