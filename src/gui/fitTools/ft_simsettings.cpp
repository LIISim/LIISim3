#include "ft_simsettings.h"

#include "core.h"
#include "../../settings/numericsettings.h"

FT_SimSettings::FT_SimSettings(QWidget *parent) : QWidget(parent), unitConversion_time(1E9), identifierGroup("ft_npt"),
    identifierStepSize("step"), identifierStartTime("start_time"), identifierLength("length")
{
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setMargin(5);
    mainLayout->setVerticalSpacing(0);
    setLayout(mainLayout);

    leStartTime = new NumberLineEdit(NumberLineEdit::DOUBLE, "Start time");
    leLength    = new NumberLineEdit(NumberLineEdit::DOUBLE, "Length");
    leStepSize  = new NumberLineEdit(NumberLineEdit::DOUBLE, "Step size");

    // limits
    leStepSize->setMinValue(1E-4);
    leStepSize->setMaxValue(10);

    // defaults [ns]
    def_startTime   = 0.0;
    def_length      = 2000.0;
    def_stepSize    = NumericSettings::defaultStepSize() * unitConversion_time;

    QLabel *labelStartTime = new QLabel("Start time [ns]", this);
    mainLayout->addWidget(labelStartTime, 0, 0);
    mainLayout->addWidget(leStartTime, 0, 1);

    QLabel *labelLength = new QLabel("Simulation length [ns]", this);
    mainLayout->addWidget(labelLength, 1, 0);
    mainLayout->addWidget(leLength, 1, 1);

    QLabel *labelStepSize = new QLabel("Step size [ns]", this);
    mainLayout->addWidget(labelStepSize, 2, 0);
    mainLayout->addWidget(leStepSize, 2, 1);

    QString tooltip = QString("Integration step size [n]\nmust be > %0\nmust be <%1")
                                .arg(leStepSize->getMinValue())
                                .arg(leStepSize->getMaxValue());

    labelStepSize->setToolTip(tooltip);
    leStepSize->setToolTip(tooltip);

    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(spacer, 0, 2);

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));

    onGUISettingsChanged();
}


FT_SimSettings::~FT_SimSettings()
{
    GuiSettings* gs = Core::instance()->guiSettings;
    gs->setValue(identifierGroup, identifierStepSize, leStepSize->getValue());
    gs->setValue(identifierGroup, identifierStartTime, leStartTime->getValue());
    gs->setValue(identifierGroup, identifierLength, leLength->getValue());  
}


/**
 * @brief FT_SimSettings::startTime
 * @return Start time in seconds [s]
 */
double FT_SimSettings::startTime()
{
    return leStartTime->getValue() / unitConversion_time;
}


/**
 * @brief FT_SimSettings::length
 * @return Simulation length in seconds [s]
 */
double FT_SimSettings::length()
{
    return leLength->getValue() / unitConversion_time;
}


/**
 * @brief FT_SimSettings::stepSize
 * @return step size in seconds [s]
 */
double FT_SimSettings::stepSize()
{
    return leStepSize->getValueWithinLimits() / unitConversion_time;
}


void FT_SimSettings::onGUISettingsChanged()
{
    GuiSettings* gs = Core::instance()->guiSettings;

    leStepSize->setValue(gs->value(identifierGroup, identifierStepSize, def_stepSize).toDouble());
    leStartTime->setValue(gs->value(identifierGroup, identifierStartTime, def_startTime).toDouble());
    leLength->setValue(gs->value(identifierGroup, identifierLength, def_length).toDouble());
}
