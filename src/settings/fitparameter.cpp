#include "fitparameter.h"


/**
 * @brief FitParameter::FitParameter
 * @param ident
 * @param name
 * @param unit
 * @param val
 * @param minVal
 * @param maxVal
 */
FitParameter::FitParameter(int ident,
                           QString name,
                           QString unit,
                           double val,
                           double minVal,
                           double maxVal,
                           double maxDelta,
                           bool enabled /* = true */)
{
    m_name = name;
    m_unit = unit;
    m_ident = ident;
    m_min_allowed_value = minVal;
    m_max_allowed_value = maxVal;
    m_bound_lower = m_min_allowed_value;
    m_bound_upper = m_max_allowed_value;
    m_max_delta = maxDelta;
    m_enabled = enabled;
    setValue(val);
}


/**
 * @brief FitParameter::FitParameter
 * @param ident
 * @param title
 * @param unit
 * @param val
 * @param minVal
 * @param maxVal
 * @param lowerBound
 * @param upperBound
 */
FitParameter::FitParameter( int ident,
                            QString name,
                            QString unit,
                            double val,
                            double minVal,
                            double maxVal,
                            double lowerBound,
                            double upperBound,
                            double maxDelta,
                            bool enabled /* = true */)
    : FitParameter(ident,name,unit,val,minVal,maxVal, maxDelta)
{
    m_bound_lower = lowerBound;
    m_bound_upper = upperBound;
    m_enabled = enabled;
}


FitParameter::FitParameter(int ident, const QMap<QString, QVariant> &settings)
{
    m_ident =ident;
    readFrom(settings);
}


/**
 * @brief FitParameter::~FitParameter
 */
FitParameter::~FitParameter()
{

}


/**
 * @brief FitParameter::setValue
 * @param val
 * @return
 */
bool FitParameter::setValue(double val)
{
    if(val < m_bound_lower)
    {
        m_current_value = m_bound_lower;
        return false;
    }
    else if(val > m_bound_upper)
    {
        m_current_value = m_bound_upper;
        return false;
    }

    m_current_value = val;
    return true;
}


/**
 * @brief FitParameter::setLowerBoundary
 * @param val
 * @return
 */
bool FitParameter::setLowerBoundary(double val)
{
    if(val < m_min_allowed_value)
    {
        m_bound_lower = m_min_allowed_value;
        return false;
    }
    else if(val > m_max_allowed_value)
    {
        m_bound_lower = m_max_allowed_value;
        return false;
    }

    if(val > m_bound_upper)
    {
        m_bound_lower = m_bound_upper;
        return false;
    }
    m_bound_lower = val;
    return true;
}


/**
 * @brief FitParameter::setUpperBoundary
 * @param val
 * @return
 */
bool FitParameter::setUpperBoundary(double val)
{
    if(val < m_min_allowed_value)
    {
        m_bound_upper = m_min_allowed_value;
        return false;
    }
    else if(val > m_max_allowed_value)
    {
        m_bound_upper = m_max_allowed_value;
        return false;
    }
    if(val < m_bound_lower)
    {
        m_bound_upper = m_bound_lower;
        return false;
    }
    m_bound_upper = val;
    return true;
}


/**
 * @brief FitParameter::setMaxDelta
 * @param val
 * @return
 */
bool FitParameter::setMaxDelta(double val)
{
    m_max_delta = val;
    return true;
}


/**
 * @brief FitParameter::toString
 * @return
 */
QString FitParameter::toString()
{
    return QString("FitParameter: %0 [%1] val: %2   min: %3 low: %4 upper: %5 max: %6 enabled: %7")
        .arg(m_name)
        .arg(m_unit)
        .arg(m_current_value)
        .arg(m_min_allowed_value)
        .arg(m_bound_lower)
        .arg(m_bound_upper)
        .arg(m_max_allowed_value)
        .arg(m_enabled);
}

void FitParameter::writeTo(QMap<QString, QVariant> &settings)
{
    settings.insert("FitParameter-"+QString::number(m_ident),m_name);
    settings.insert(QString::number(m_ident)+"-value", m_current_value);
    settings.insert(QString::number(m_ident)+"-unit", m_unit);
    settings.insert(QString::number(m_ident)+"-bound_lower", m_bound_lower);
    settings.insert(QString::number(m_ident)+"-bound_upper", m_bound_upper);
    settings.insert(QString::number(m_ident)+"-min_allowed", m_min_allowed_value);
    settings.insert(QString::number(m_ident)+"-max_allowed", m_max_allowed_value);
    settings.insert(QString::number(m_ident)+"-enabled", m_enabled);
}

void FitParameter::readFrom(const QMap<QString, QVariant> &settings)
{
    QStringList keys = settings.keys();
    QString identstr = QString::number(m_ident);
    for(int i = 0; i < keys.size(); i++)
    {
        QString key =keys[i];

        if(key.startsWith("FitParameter") && key.endsWith(identstr))
            m_name = settings.value(key).toString();
        else if(key.startsWith(identstr))
        {
            if(key.endsWith("-value"))
                m_current_value = settings.value(key,0.0).toDouble();
            else if(key.endsWith("-unit"))
                m_unit = settings.value(key).toString();
            else if(key.endsWith("-bound_lower"))
                m_bound_lower = settings.value(key,0.0).toDouble();
            else if(key.endsWith("-bound_upper"))
                m_bound_upper = settings.value(key,0.0).toDouble();
            else if(key.endsWith("-min_allowed"))
                m_min_allowed_value = settings.value(key,0.0).toDouble();
            else if(key.endsWith("-max_allowed"))
                m_max_allowed_value = settings.value(key,0.0).toDouble();
            else if(key.endsWith("-enabled"))
                m_enabled = settings.value(key,false).toBool();
        }
    }
}


