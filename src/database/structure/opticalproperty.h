#ifndef OPTICALPROPERTY_H
#define OPTICALPROPERTY_H

#include "../databasecontent.h"

class OpticalProperty
{
public:
    OpticalProperty();

    QString name;
    QString unit;
    QString description;
    QString source;

    bool available;
    bool inFile;

    std::map<int, Property> values;


    double operator()(double T, int wavelength);

    bool exists(int wavelength);

    static OpticalProperty assignCheck(OpticalProperty iprop, varList);

    QString toString() const;
    QString valueAsString() const;

};

#endif // OPTICALPROPERTY_H
