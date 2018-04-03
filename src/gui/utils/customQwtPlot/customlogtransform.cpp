#include "customlogtransform.h"



//! Smallest allowed value for logarithmic scales: 1.0e-150
const double CustomLogTransform::LogMin = 1.0e-150;

//! Largest allowed value for logarithmic scales: 1.0e150
const double CustomLogTransform::LogMax = 1.0e150;


/**
 * @brief CustomLogTransform::CustomLogTransform Constructor
 */
CustomLogTransform::CustomLogTransform(): QwtTransform()
{

}

/**
 * @brief CustomLogTransform::~CustomLogTransform Destructor
 */
CustomLogTransform::~CustomLogTransform()
{

}


/**
 * @brief CustomLogTransform::transform
 * @param value Value to be transformed
 * @return log( value )
 */
double CustomLogTransform::transform( double value ) const
{
    if(value <= 0.0)
        value = 1e-15;

    return ::log( value );
}

/**
 * @brief CustomLogTransform::invTransform
 * @param value Value to be transformed
 * @return exp( value )
 */
double CustomLogTransform::invTransform( double value ) const
{
    return exp( value );
}


/**
 * @brief CustomLogTransform::bounded
 * @param value Value to be bounded
 * @return qBound( LogMin, value, LogMax )
 */
double CustomLogTransform::bounded( double value ) const
{
    return qBound( LogMin, value, LogMax );
}


/**
 * @brief CustomLogTransform::copy
 * @return Clone of the transformation
 */
QwtTransform *CustomLogTransform::copy() const
{
    return new CustomLogTransform();
}

