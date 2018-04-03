#ifndef FT_MODELINGSETTINGSTABLE_H
#define FT_MODELINGSETTINGSTABLE_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>

#include "../utils/numberlineedit.h"

/**
 * @brief The FT_ModelingSettingsTable class enables editing
 * the (global) modeling settings for the FitTools (FitCreator) GUI
 */
class FT_ModelingSettingsTable : public QWidget//public QTreeWidget//public QTableWidget
{
    Q_OBJECT
public:
    explicit FT_ModelingSettingsTable(QWidget *parent = 0);
    ~FT_ModelingSettingsTable();

    double getPressure();

protected:
    //virtual void resizeEvent(QResizeEvent * event);

private:
    const double unitConversion_pressure;

    QComboBox* cbHtm;
    QComboBox* cbMaterial;
    QComboBox* cbGasmix;
    NumberLineEdit* lePressure;

    QCheckBox *checkboxConduction;
    QCheckBox *checkboxEvaporation;
    QCheckBox *checkboxRadiation;

private slots:
    //handlers for program state changes

    void onDBCchanged(int dbcid = -1);
    void onModelingSettingsChanged();

    // handlers for user interaction

    void onCbHtmEdited(int idx);
    void onCbMaterialEdited(int idx);
    void onCbGasmixEdited(int idx);    
    void onCheckboxStateChanged();

};

#endif // FT_MODELINGSETTINGSTABLE_H
