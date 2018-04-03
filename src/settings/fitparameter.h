#ifndef FITPARAMETER_H
#define FITPARAMETER_H



#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

/**
 * @brief The FitParameter class
 */
class FitParameter
{
public:

    explicit FitParameter(int ident,
                          QString name,
                          QString unit,
                          double val,
                          double minVal,
                          double maxVal,
                          double maxDelta,
                          bool enabled = true);

    explicit FitParameter(int ident,
                          QString name,
                          QString unit,
                          double val,
                          double minVal,
                          double maxVal,
                          double lowerBound,
                          double upperBound,
                          double maxDelta,
                          bool enabled = true);

    explicit FitParameter(int ident,
                          const QMap<QString, QVariant> &settings);

    virtual ~FitParameter();


    inline int ident() const {return m_ident;}

    /**
     * @brief name of parameter
     * @return
     */
    inline QString name() const {return m_name;}
    inline void setName(QString name){m_name = name;}


    /**
     * @brief unit string representation of parameter unit
     * @return
     */
    inline QString unit() const {return m_unit;}
    inline void setUnit(QString unit){m_unit = unit;}

    inline double value() const {return m_current_value;}
    bool setValue(double value);

    /**
     * @brief lowerBound lower fit boundary
     * @return
     */
    inline double lowerBound() const {return m_bound_lower;}
    bool setLowerBoundary(double val);

    /**
     * @brief upperBound upper fit boundary
     * @return
     */
    inline double upperBound() const {return m_bound_upper;}
    bool setUpperBoundary(double val);

    /**
     * @brief maxDelta maximum value for search direction
     * to minimize overshooting in non-physical parameter ranges
     * @return
     */
    inline double maxDelta() const {return m_max_delta;}
    bool setMaxDelta(double val);

    /**
     * @brief minAllowedValue minium permissible value of parameter
     * (used for validation of user input)
     * @return
     */
    inline double minAllowedValue(){return m_min_allowed_value;}


    /**
     * @brief maxAllowedValue maximum permissible value of parameter
     * (used for validation of user input)
     * @return
     */
    inline double maxAllowedValue(){return m_max_allowed_value;}

    /**
     * @brief enabled false: parameter will still be used but is kept constant
     * @return
     */
    inline bool enabled() const {return m_enabled;}
    inline void setEnabled(bool state){m_enabled = state;}

    QString toString();

    void writeTo(QMap<QString, QVariant> &settings);
    void readFrom(const QMap<QString, QVariant> &settings);

private:

    /**
     * @brief m_ident used for idenfication
     */
    int m_ident;

    /**
     * @brief m_title
     */
    QString m_name;

    /**
     * @brief m_unit
     */
    QString m_unit;

    /**
     * @brief m_bound_lower
     */
    double m_bound_lower;

    /**
     * @brief m_bound_upper
     */
    double m_bound_upper;

    /**
     * @brief m_max_delta
     */
    double m_max_delta;


    /**
     * @brief m_min_allowed_value
     */
    double m_min_allowed_value;

    /**
     * @brief m_max_allowed_value
     */
    double m_max_allowed_value;

    /**
     * @brief m_enabled
     */
    bool m_enabled;

    /**
     * @brief m_current_value
     */
    double m_current_value;

};

#endif // FITPARAMETER_H
