#ifndef CALCULATIONTOOLBOX_H
#define CALCULATIONTOOLBOX_H

#include "ribbontoolbox.h"

#include <QAction>
#include <QToolButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QList>

#include "materialcombobox.h"
#include "labeledcombobox.h"
#include "../../signal/signal.h"

class CalculationToolbox : public RibbonToolBox
{
    Q_OBJECT
public:
    CalculationToolbox(QWidget *parent = 0);

private:
    QToolButton *buttonRecalc;
    QToolButton *buttonReset;
    QToolButton *buttonCancel;

    QRadioButton *rbCalcModeSingle;
    QRadioButton *rbCalcModeGroup;
    QRadioButton *rbCalcModeAll;

    QButtonGroup *bgCalcModes;

    QCheckBox *checkboxCalcRaw;
    QCheckBox *checkboxCalcAbs;
    QCheckBox *checkboxCalcTemp;

    MaterialComboBox* materialComboBox;
    QCheckBox *checkboxRescanDBAtRecalc;

    QString identifier_settings_group;
    QString identifier_calcMode;
    QString identifier_calcRaw;
    QString identifier_calcAbs;
    QString identifier_calcTemp;
    QString identifier_dbscan;

signals:
    void recalc(QList<Signal::SType> typeList);

private slots:
    void onRecalcClicked();
    void onResetClicked();
    void onCancelClicked();

    void onCalcModeToggled(QAbstractButton *button, bool state);
    void onCheckboxCalcTypeChanged();
    void onCheckboxRescanDBChanged();

    void onProcessingStateChanged(bool state);

public slots:
    void onGuiSettingsChanged(QString subgroup, QString key, QVariant value);
    void onGuiSettingsChanged();
};

#endif // CALCULATIONTOOLBOX_H
