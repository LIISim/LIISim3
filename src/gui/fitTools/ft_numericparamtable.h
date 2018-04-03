#ifndef FT_NUMERICPARAMTABLE_H
#define FT_NUMERICPARAMTABLE_H

#include <QWidget>
#include <QComboBox>

class NumberLineEdit;

/**
 * @brief The FT_NumericParamTable class provides a
 * user interface for the numerical parameters (used for fit)
 */
class FT_NumericParamTable : public QWidget
{
    Q_OBJECT
public:
    explicit FT_NumericParamTable(QWidget *parent = 0);
    ~FT_NumericParamTable();

    int iterations();
    int ODE();
    int ODE_stepSizeFactor();

private:
    NumberLineEdit* le_it;

    QComboBox *cbODE;
    QComboBox *cbODE_stepSizeFactor;

    // GUI settings keys
    QString gs_group;
    QString gsk_it;    
    QString gsk_ode;
    QString gsk_ode_stepSizeFactor;

private slots:
    void onGuiSettingsChanged();
    void onCbODEEdited(int idx);
    void onCbODEStepSizeFactorEdited(int idx);

};

#endif // FT_NUMERICPARAMTABLE_H
