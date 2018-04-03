#ifndef CHECKBOXGROUPINPUT_H
#define CHECKBOXGROUPINPUT_H

#include "plugininputfield.h"

#include <QGridLayout>

#include <QLabel>
#include <QCheckBox>
#include <QListWidget>



class CheckboxGroupInput : public PluginInputField
{
    Q_OBJECT

    QGridLayout *layGrid;

    QLabel *labelText; // general label (top left)

    QList<QString> labels; // labels for checkboxes
    QList<bool> values; // checkbox values


    QList<QCheckBox *> checkBoxes;

    public:
        explicit CheckboxGroupInput(QWidget *parent = 0);

        // call this function after setting checkbox data
        void init();

        // implement the PluginInputField interface
        void setPluginParamValue(QVariant value);
        void setPluginParamMinValue(QVariant minValue);
        void setPluginParamMaxValue(QVariant maxValue);
        void setPluginLabelText(QString text);
        void setPluginLabelList(QList<QString> labelList);
        void setPluginValueList(QList<QVariant> valueList);
        QVariant getPluginParamValue();

    signals:

    public slots:

    private slots:
        void onValueChanged();
};

#endif // CHECKBOXGROUPINPUT_H
