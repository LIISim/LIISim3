#ifndef NUMBERLINEEDIT_H
#define NUMBERLINEEDIT_H

#include <QLineEdit>

class QDoubleValidator;
class QValidator;

/**
 * @brief The NumberLineEdit class (LineEdit with max/min checks, int and double validation)
 * @ingroup GUI-Utilities
 */
class NumberLineEdit : public QLineEdit
{
    Q_OBJECT

public:



    /**
     * @brief type of validation for linedit content
     */
    enum NumberType {NOVALIDATION,DOUBLE,INTEGER};


    explicit NumberLineEdit(QWidget *parent = 0);
     ~NumberLineEdit();
    NumberLineEdit(NumberType ntype, QWidget *parent=0);
    NumberLineEdit(NumberType ntype,const QString & tooltip, QWidget *parent=0);

    inline double getMinValue(){return minValue;}
    inline double getMaxValue(){return maxValue;}

    void setValue(double d);
    void setMinValue(double minv);
    void setMaxValue(double maxv);
    double getValue();
    double getValueWithinLimits();

private:

    void init(NumberType ntype);

    bool useValidation;
    QValidator* validator;

    double minValue;
    double maxValue;
    bool useMinMaxValidation;

signals:

    void valueChanged();

public slots:

    virtual void setText ( const QString & );
    void onTextChanged(const QString & str);

};

#endif // NUMBERLINEEDIT_H
