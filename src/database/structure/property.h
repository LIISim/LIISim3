#ifndef PROPERTY_H
#define PROPERTY_H

#include <QString>
#include <QDebug>

class Property
{
public:

    Property();
//    Property(QString name,
//             QString type,
//             QString description);

    // contains available equations types for use in EquationList class
    static QStringList eqTypeList;

    QString name;
    QString type;
    QString unit;    
    QString description;
    QString source;

    bool available;
    bool inFile;

    bool optional;

    double parameter[9];

    QString identifier; // used for GasMixture databasecontent (filename)

    //!!! add warning if non-static variable is called with zero temperature
    //!!! rho_p(0) - poly
    //!!! GasMixture::GasMixture() uses molar_mass()
    //!!! Material::c_tv() uses molar_mass()
    //!!! Material::p_s_clausius_clapeyron() uses molar_mass()

    double operator()();
    double operator()(double T);
    double operator()(double T, double wavelength);

    QString toString() const;

    QString valueAsString() const;

    static Property assignCheck(Property prop, // this property should contain preset unit and description
                                std::multimap<QString, Property> var_list);

    static Property assignCheckOptional(Property prop, // this property should contain preset unit and description
                                std::multimap<QString, Property> var_list);

    static double assignCheckValueDouble(std::multimap<QString, Property> var_list,
                                QString var_name);
};

#endif // PROPERTY_H
