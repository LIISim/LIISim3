#ifndef LABELEDLINEEDIT_H
#define LABELEDLINEEDIT_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>


#include "numberlineedit.h"
#include "plugininputfield.h"


/**
 * @brief The LabeledLineEdit class is widget which contains
 * a Textfield for Number inputs and a description label.
 * @details This class implements the functionality to serve as an
 * input field for signal processing plugins
 * @ingroup GUI-Utilities
 */
class LabeledLineEdit : public PluginInputField
{
    Q_OBJECT

    QHBoxLayout* layMainH;
    QLabel* description;
    NumberLineEdit* field;


    void init(NumberLineEdit::NumberType ntype);


public:

    explicit LabeledLineEdit(QWidget *parent = 0);
    LabeledLineEdit(QString descr, QWidget *parent = 0);
    LabeledLineEdit(QString descr,NumberLineEdit::NumberType ntype, QWidget *parent = 0);

    void setValue(double d);
    void setEmpty();
    double getValue();
    void setMaxFieldWidth(int width);


    // implement the PluginInputField interface
    void init();
    void setPluginParamValue(QVariant value);
    void setPluginParamMinValue(QVariant minValue);
    void setPluginParamMaxValue(QVariant maxValue);
    void setPluginLabelText(QString text);
    void setPluginLabelList(QList<QString> labels);
    void setPluginValueList(QList<QVariant> values);
    QVariant getPluginParamValue() ;

    QVariant userData;

signals:

    void valueChanged();

public slots:

private slots:

    void onValueChanged();
    void onEditingFinished();

};

#endif // LABELEDLINEEDIT_H
