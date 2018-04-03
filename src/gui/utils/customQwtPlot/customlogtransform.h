#ifndef CUSTOMLOGTRANSFORM_H
#define CUSTOMLOGTRANSFORM_H

#include <qwt_transform.h>

/**
 * @brief The CustomLogTransform class implements custom logatrithmic transformation.
 * Difference to default QwtLogTransform: Values < 0 are set to a minimum value before
 * log transformation (=> no NaN/inf values in plots!)
 * @ingroup GUI-Utilities
 */
class CustomLogTransform: public QwtTransform
{
public:
    CustomLogTransform();
    virtual ~CustomLogTransform();

    virtual double transform( double value ) const;
    virtual double invTransform( double value ) const;
    virtual double bounded( double value ) const;

    virtual QwtTransform *copy() const;

    static const double LogMin;
    static const double LogMax;
};

#endif // CUSTOMLOGTRANSFORM_H
