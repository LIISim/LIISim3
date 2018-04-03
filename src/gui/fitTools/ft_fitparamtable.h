#ifndef FT_FITPARAMTABLE_H
#define FT_FITPARAMTABLE_H

#include <QWidget>
#include <QGridLayout>

#include "../../settings/fitsettings.h"

/**
 * @brief The FT_FitParamTable class provides a
 * graphical user interface for the modification of
 * fit parameters
 */
class FT_FitParamTable : public QWidget
{
    Q_OBJECT
public:
    explicit FT_FitParamTable(QWidget *parent = 0);
    ~FT_FitParamTable();

    QList<FitParameter> fitParameters();

private:
    QGridLayout *mainLayout;

    QList<FitParameter> fparams;

    QString identifierFitInitParam;

    QString identifierParDiaValue;
    QString identifierParDiaLowBound;
    QString identifierParDiaUppBound;
    QString identifierParDiaMaxDelta;

    QString identifierGasTempValue;
    QString identifierGasTempLowBound;
    QString identifierGasTempUppBound;
    QString identifierGasTempMaxDelta;

    QString identifierPeakTempValue;
    QString identifierPeakTempLowBound;
    QString identifierPeakTempUppBound;
    QString identifierPeakTempMaxDelta;

    QList<QWidget*> constantCheckboxes;
    QList<QWidget*> valueNLE;
    QList<QWidget*> lowBoundNLE;
    QList<QWidget*> uppBoundNLE;
    QList<QWidget*> maxDeltaNLE;

private slots:
    void onValueChanged();
    void onCheckboxToggled(bool state);

    void onGuiSettingsChanged();

};

#endif // FT_FITPARAMTABLE_H
