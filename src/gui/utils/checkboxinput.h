#ifndef CHECKBOXINPUT_H
#define CHECKBOXINPUT_H

#include "plugininputfield.h"
#include <QCheckBox>
#include <QHBoxLayout>

/**
 * @brief The CheckboxInput class
 * @ingroup GUI-Utilities
 */
class CheckboxInput : public PluginInputField
{
    Q_OBJECT

    QHBoxLayout* layBoxH;
    QCheckBox* checkBox;

    public:
        explicit CheckboxInput(QWidget *parent = 0);

        // implement the PluginInputField interface
        void init();
        void setPluginParamValue(QVariant value);
        void setPluginParamMinValue(QVariant minValue);
        void setPluginParamMaxValue(QVariant maxValue);
        void setPluginLabelText(QString labelText);
        void setPluginLabelList(QList<QString> labels);
        void setPluginValueList(QList<QVariant> values);
        QVariant getPluginParamValue();

    signals:

    public slots:

    private slots:
        void onValueChanged();

};

#endif // CHECKBOXINPUT_H
