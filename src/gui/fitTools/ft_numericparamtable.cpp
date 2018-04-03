#include "ft_numericparamtable.h"

#include "../../core.h"
#include "../utils/numberlineedit.h"
#include "../../settings/numericsettings.h"
#include "../../calculations/numeric.h"


FT_NumericParamTable::FT_NumericParamTable(QWidget *parent) : QWidget(parent)
{
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setMargin(5);
    mainLayout->setVerticalSpacing(0);
    setLayout(mainLayout);

    // gui settings keys
    gs_group   = "ft_npt";
    gsk_it     = "iter";
    gsk_ode    = "ODE";
    gsk_ode_stepSizeFactor = "ODE_step";

    // init max iterations row
    QLabel *labelIterations = new QLabel("Max iterations", this);
    mainLayout->addWidget(labelIterations, 0, 0);

    le_it = new NumberLineEdit(NumberLineEdit::INTEGER);

    // limits
    le_it->setMinValue(1);
    le_it->setMaxValue(2000);

    QString ttip = QString("Maximum iteration count\nmust be > %0\nmust be <%1")
            .arg(le_it->getMinValue())
            .arg(le_it->getMaxValue());
    le_it->setToolTip(ttip);
    labelIterations->setToolTip(ttip);

    mainLayout->addWidget(le_it, 0, 1);

    // ODE combobox
    QLabel *labelODE = new QLabel("ODE solver", this);
    labelODE->setToolTip("Ordinary differential equation solver used for simulation of heat transfer");
    mainLayout->addWidget(labelODE, 1, 0);

    cbODE = new QComboBox;
    cbODE->addItems(Numeric::getAvailableODENameList());
    cbODE->setCurrentIndex((int)Numeric::defaultODESolver());
    mainLayout->addWidget(cbODE, 1, 1);

    // description of combobox elements
    for(int i = 0; i < cbODE->count(); i++)
            cbODE->setItemData(i, Numeric::getODEDescription(i), Qt::ToolTipRole);

    QString tooltip = QString("Step size factor (fac) used for solving the ODE (Fitting only):\n"
                              " - default step size (dt_data) is given by experimental data\n"
                              " - model is solved with dt = dt_data / fac \n"
                              " - 'same as data': fac = 1 \n"
                              " - '2x more accurate': fac = 2\n"
                              " - ...");

    // step size factor
    QLabel *labelAccuracy = new QLabel("Accuracy", this);
    labelAccuracy->setToolTip(tooltip);
    mainLayout->addWidget(labelAccuracy, 1, 2);

    cbODE_stepSizeFactor = new QComboBox;
    cbODE_stepSizeFactor->setToolTip(tooltip);
    mainLayout->addWidget(cbODE_stepSizeFactor, 1, 3);

    cbODE_stepSizeFactor->addItem(QString("same as data"));

    for(int i = 1; i < 5; i++)
    {
        int acc = pow(2, i);
        cbODE_stepSizeFactor->addItem(QString("%0 times more accurate").arg(acc));
    }

    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(spacer, 0, 4);
    mainLayout->setColumnStretch(4, 2);

    //create connections
    connect(Core::instance()->guiSettings,
            SIGNAL(settingsChanged()),
            SLOT(onGuiSettingsChanged()));

    // init fields
    onGuiSettingsChanged();
}



FT_NumericParamTable::~FT_NumericParamTable()
{
    // save field states to guisettings
    GuiSettings* gs = Core::instance()->guiSettings;
    gs->setValue(gs_group, gsk_it, le_it->getValue());    
    gs->setValue(gs_group, gsk_ode, cbODE->currentIndex());
    gs->setValue(gs_group, gsk_ode_stepSizeFactor, cbODE_stepSizeFactor->currentIndex());
}


/**
 * @brief FT_NumericParamTable::onGuiSettingsChanged This
 * slot is executed when the global GuiSettings changed.
 * Updates input field values and min/max limits
 */
void FT_NumericParamTable::onGuiSettingsChanged()
{
    GuiSettings* gs = Core::instance()->guiSettings;

    // max iterations field
    le_it->setValue(gs->value(gs_group, gsk_it,
                              NumericSettings::defaultFitMaxIterations()).toInt());    
    // ODE solver
    int idx = (int)Numeric::defaultODESolver();
    cbODE->setCurrentIndex(gs->value(gs_group, gsk_ode, idx).toInt());

    // ODE step size
    cbODE_stepSizeFactor->setCurrentIndex(gs->value(gs_group, gsk_ode_stepSizeFactor, 0).toInt());
}


void FT_NumericParamTable::onCbODEEdited(int idx)
{
    if(idx < 0) return;

    GuiSettings* gs = Core::instance()->guiSettings;

    int ident = cbODE->itemData(idx).toInt();
    // write to settings
    gs->setValue(gs_group, gsk_ode, ident);
}


void FT_NumericParamTable::onCbODEStepSizeFactorEdited(int idx)
{
    if(idx < 0) return;

    GuiSettings* gs = Core::instance()->guiSettings;

    int ident = cbODE_stepSizeFactor->itemData(idx).toInt();
    // write to settings
    gs->setValue(gs_group, gsk_ode_stepSizeFactor, ident);
}



/**
 * @brief FT_NumericParamTable::maxIterationCount get user selection of
 * numerical parameter: maximum iteration count
 * @return
 */
int FT_NumericParamTable::iterations()
{
    return (int)le_it->getValue();
}

/**
 * @brief FT_NumericParamTable::ODE index represented by Numeric::ODE_Algorithm
 * @return Numeric::ODE_Algorithm;
 */
int FT_NumericParamTable::ODE()
{
     return cbODE->currentIndex();;
}


/**
 * @brief FT_NumericParamTable::ODE_stepSize step size factor for ODE solver (default = 1)
 * @return
 */
int FT_NumericParamTable::ODE_stepSizeFactor()
{
    // power of two (x1, x2, x4, x8, x16,...)
    return pow(2, cbODE_stepSizeFactor->currentIndex());
}

