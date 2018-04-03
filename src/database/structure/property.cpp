#include "property.h"
#include <math.h>

#include <QStringList>

// List of available equation type (for EquationList class)
QStringList Property::eqTypeList = QStringList()
           << "const"
           << "case"
           << "poly"
           << "poly2"
           << "polycase"
           << "exp"
           << "exppoly"
           << "powx"           
           << "optics_case"
           << "optics_exp"
           << "optics_lambda"
           << "optics_temp"
           << "notSet"
           << "error"; // if type is not found

/**
 * @brief Property::Property
 *  Use constructor for manual setting of member vars
 */
Property::Property()
{
   name     = "notSet";
   type     = "notSet";
   unit     = "notSet";
   source   = "notSet";
   description = "no description";

   // inFile will be set to true, if property exists in database file
   inFile = false;
   // available will be set to true, if property exists and type != "notSet"
   available = false;

   // defines if variable is optional or mandatory
   optional = false;

   for(int i = 0; i< 9;i++)
       parameter[i] = 0.0;
}


/**
 * @brief Property::operator ()
 * Use this for non-temperature-dependent variables
 * Example:
 *      molar_mass()
 * @return parameter[0]
 */
double Property::operator()()
{
    //!!! throw exception if this->type != const
    return parameter[0];
}


/**
 * @brief Property::operator ()
 * When called, property gives value at temperature
 * Example:
 *      rho_p(300) gives density at 300 K
 * @param T temperature
 * @return value at temperature T (double)
 */
double Property::operator()(double T)
{
    if(type == "const")
    {
        return parameter[0];
    }
    else if (type == "case")
    {
        if (T <= parameter[0])  return parameter[1];
        else                    return parameter[2];
    }
    else if (type == "poly")
    {
        // SUM (Ai*T^i)
        return parameter[0]
                + parameter[1]*T
                + parameter[2]*T*T
                + parameter[3]* pow(T,3)
                + parameter[4]* pow(T,4)
                + parameter[5]* pow(T,5)
                + parameter[6]* pow(T,6)
                + parameter[7]* pow(T,7)
                + parameter[8]* pow(T,8);
    }
    else if (type == "poly2")
    {
        return parameter[0] + parameter[1]*T + parameter[2]*T*T
                + parameter[3]*T*T*T + parameter[4]/T + parameter[5]/T/T;
    }
    else if (type == "polycase")
    {
        if (T <= parameter[0])  return parameter[1] + parameter[2]*T + parameter[3]*T*T + parameter[4]*T*T*T;
        else                    return parameter[5] + parameter[6]*T + parameter[7]*T*T + parameter[8]*T*T*T;
    }    
    else if (type == "exp")
    {
         // a + b * exp(c + d/T + e*T)
        return parameter[0] + parameter[1] * exp(parameter[2] + parameter[3]/T + parameter[4]*T);
    }
    else if (type == "exppoly")
    {
         // a + b * exp(c + d*T + e*T^2 + f*T^3 + g*T^4 + h*T^5)
        return parameter[0] + parameter[1] * exp(parameter[2] + parameter[3]*T + parameter[4]*T*T + parameter[5]*T*T*T + parameter[6]*T*T*T*T + parameter[7]*T*T*T*T*T);
    }
    else if (type == "powx")
    {
        // a + b *c^(d + e/T + f*T)
        return parameter[0] + parameter[1] * pow(parameter[2], (parameter[3] + parameter[4]/T + parameter[5]*T));
    }   
    // OPTICS (parameter[0] is used for wavelength recognition
    else if (type == "optics_temp")
    {
        return parameter[1] + parameter[2]*T + parameter[3]*T*T + parameter[4]*T*T*T;
    }
    else if (type == "optics_case")
    {
        if (T <= parameter[1])  return parameter[2];
        else                    return parameter[3];
    }
    else
    {
        // for example if type == "notSet"
        //!!! throw exception
        return 0.0;
    }
}


/**
 * @brief Property::operator ()
 * When called, property gives value at temperature and wavelength
 * Example:
 *      Em_func(2000, 500) gives absorption function at 2000 K and 500 nm
 * @param T
 * @param wavelength [m]
 * @return
 */
double Property::operator()(double T, double wavelength)
{
    if(type == "const")
    {
        //y = A
        return parameter[0];
    }
    else if(type == "optics_lambda")
    {         
        //y = A + B*x + C*x^2 + D*x^3 + E*x^4 + F*x^5 + G*x^6 + H*x^7 + I*x^8
        return parameter[0]
                + parameter[1] * wavelength
                + parameter[2] * pow(wavelength,2)
                + parameter[3] * pow(wavelength,3)
                + parameter[4] * pow(wavelength,4)
                + parameter[5] * pow(wavelength,5)
                + parameter[6] * pow(wavelength,6)
                + parameter[7] * pow(wavelength,7)
                + parameter[8] * pow(wavelength,8);
    }    
    else if(type == "optics_exp")
    {
        //y = x^(1 - A) * B
        return parameter[0] * pow(wavelength, (1 - parameter[1]));
    }
    else
    {
         // for example if type == "notSet"
        //!!! throw exception
        return 0.0;
    }
}


/**
 * @brief Property::assignCheck
 * Checks if var_name is available in var_list and returns Property object.
 * Is used in Material and GasProperties classes.
 * @param var_list
 * @return Property object
 */
Property Property::assignCheck(Property prop, std::multimap<QString, Property> var_list)
{
    if(var_list.count(prop.name) == 0)
    {        
        // if element does not exist in file, create new element,
        // set available = false and pass default values
        Property element;
        element.name        = prop.name;
        element.unit        = prop.unit;
        element.description = prop.description;
        element.optional    = prop.optional;

        element.type        = "notSet";
        element.available   = false;
        element.inFile      = false;

        return element;
    }
    else
    {                
        Property element = var_list.find(prop.name)->second;

        // overwrite with default values
        element.unit        = prop.unit;
        element.description = prop.description;
        element.optional    = prop.optional;
        element.inFile      = true;

        // if file was auto-generated, property exists, but is empty
        // avoid usage of this property during calculations by setting available = false
        if(element.type == "notSet")
        {
            element.available   = false;
        }
        else
        {
            element.available   = true;
        }

        return element;
    }
}


Property Property::assignCheckOptional(Property prop, std::multimap<QString, Property> var_list)
{
    prop.optional = true;

    // do the same as assignCheck()
    return Property::assignCheck(prop, var_list);
}


/**
 * @brief Property::assignCheckValueDouble
 * Checks if var_name is available in var_list and returns first parameter of Property object (parameter[0]).
 * Is used in LIISettings class
 * @param var_list
 * @param var_name
 * @return element.parameter[0]
 */
double Property::assignCheckValueDouble(std::multimap<QString, Property> var_list, QString var_name)
{

    //!!! exception needs to be included

    if(var_list.count(var_name) == 0)
    {
        return 0.0;
    }
    else
    {
        Property element = var_list.find(var_name)->second;        
        return element.parameter[0];
    }
}


QString  Property::toString() const
{
    QString s = "name: "+name+"\ttype: "+type+"\tparams:";
    QString params;
    if(type == "const")
    {
        params.sprintf("%f",parameter[0]);
    }
    else
    {
        params.sprintf("\t%f %f %f %f %f %f %f %f %f",parameter[0],parameter[1],parameter[2],parameter[3],parameter[4],parameter[5],parameter[6],parameter[7],parameter[8]);
    }

    s = s + params.toLatin1().data()+"\n";
    return s;
}


QString Property::valueAsString() const
{
    QString res;
    if(type == "const")
    {
        res.sprintf("%f",parameter[0]);
    }
    else
    {
        res.sprintf("\t%f %f %f %f %f %f %f %f %f",parameter[0],parameter[1],parameter[2],parameter[3],parameter[4],parameter[5],parameter[6],parameter[7],parameter[8]);
    }
    return res;
}

