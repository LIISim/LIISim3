#include "ft_fitparamtable.h"

#include <QDebug>
#include <QCheckBox>

#include "../../core.h"
#include "../utils/numberlineedit.h"

/**
 * @brief FT_FitParamTable::FT_FitParamTable constructor
 * @param parent
 */
FT_FitParamTable::FT_FitParamTable(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QGridLayout;
    mainLayout->setMargin(5);
    mainLayout->setVerticalSpacing(0);
    setLayout(mainLayout);

    mainLayout->addWidget(new QLabel("Fixed", this), 0, 1);
    mainLayout->addWidget(new QLabel("Value", this), 0, 2);
    mainLayout->addWidget(new QLabel("Lower Boundary", this), 0, 3);
    mainLayout->addWidget(new QLabel("Upper Boundary", this), 0, 4);
    mainLayout->addWidget(new QLabel("Max Delta", this), 0, 5);

    identifierFitInitParam = "FitInitialParameters";

    identifierParDiaValue = "ParticleDiameterValue";
    identifierParDiaLowBound = "ParticleDiameterLowerBoundary";
    identifierParDiaUppBound = "ParticleDiameterUpperBoundary";
    identifierParDiaMaxDelta = "ParticleDiameterMaxDelta";

    identifierGasTempValue = "GasTemperatureValue";
    identifierGasTempLowBound = "GasTemperatureLowerBoundary";
    identifierGasTempUppBound = "GasTemperatureUpperBoundary";
    identifierGasTempMaxDelta = "GasTemperatureMaxDelta";

    identifierPeakTempValue = "PeakTemperatureValue";
    identifierPeakTempLowBound = "PeakTemperatureLowerBoundary";
    identifierPeakTempUppBound = "PeakTemperatureUpperBoundary";
    identifierPeakTempMaxDelta = "PeakTemperatureMaxDelta";

    fparams = FitSettings::availableFitParameters();

    for(int i = 0; i < fparams.size(); i++)
    {
        //setRowHeight(i,rh);
        FitParameter p = fparams[i];

        // generate label/tooltip text

        QString labelText = p.name();
        labelText.append(" [");
        labelText.append( p.unit() );
        labelText.append("]");

        QString tooltipText = p.name();
        tooltipText.append("\nunit: ");
        tooltipText.append( p.unit() );
        tooltipText.append("\nmust be > ");
        tooltipText.append( QString::number(p.minAllowedValue()) );
        tooltipText.append("\nmust be < ");
        tooltipText.append( QString::number(p.maxAllowedValue()) );

        // create items/cellwidgets

        // parameter name label
        QLabel *label = new QLabel(labelText, this);
        label->setToolTip(tooltipText);
        mainLayout->addWidget(label, i+1, 0);

        //setItem(i,0, new QTableWidgetItem(labelText));
        //item(i,0)->setToolTip(tooltipText);
        //item(i,0)->setFlags(itemFlags_col0);

        // fix parameter
        QCheckBox* ch_v = new QCheckBox();
        ch_v->setChecked(false);
        QString ttip = "If checked, parameter is fixed during fitting";
        ch_v->setToolTip(ttip);
        constantCheckboxes.push_back(ch_v);

        //center checkboxes
        QWidget *pWidget = new QWidget();
        QHBoxLayout *pLayout = new QHBoxLayout(pWidget);
        pLayout->addWidget(ch_v);
        pLayout->setAlignment(Qt::AlignCenter);
        pLayout->setContentsMargins(0,0,0,0);
        pWidget->setLayout(pLayout);

        mainLayout->addWidget(pWidget, i+1, 1);

        // initial value
        NumberLineEdit* le_v = new NumberLineEdit(NumberLineEdit::DOUBLE);
        le_v->setValue( p.value() );
        le_v->setMinValue( p.minAllowedValue() );
        le_v->setMaxValue( p.maxAllowedValue() );
        ttip = "initial value for ";
        ttip.append(tooltipText);
        le_v->setToolTip(ttip);
        //setCellWidget(i,2,le_v);
        mainLayout->addWidget(le_v, i+1, 2);
        valueNLE.append(le_v);

        // lower fit boundary
        NumberLineEdit* le_l = new NumberLineEdit(NumberLineEdit::DOUBLE);
        le_l->setValue( p.lowerBound() );
        le_l->setMinValue( p.minAllowedValue() );
        le_l->setMaxValue( p.maxAllowedValue() );
        ttip = "lower fit boundary for ";
        ttip.append(tooltipText);
        le_l->setToolTip(ttip);
        //setCellWidget(i,3,le_l);
        mainLayout->addWidget(le_l, i+1, 3);
        lowBoundNLE.append(le_l);

        // upper fit boundary
        NumberLineEdit* le_u = new NumberLineEdit(NumberLineEdit::DOUBLE);
        le_u->setValue( p.upperBound() );
        le_u->setMinValue( p.minAllowedValue() );
        le_u->setMaxValue( p.maxAllowedValue() );
        ttip = "upper fit boundary for ";
        ttip.append(tooltipText);
        le_u->setToolTip(ttip);
        //setCellWidget(i,4,le_u);
        mainLayout->addWidget(le_u, i+1, 4);
        uppBoundNLE.append(le_u);

        // max change per iteration
        NumberLineEdit* le_delta = new NumberLineEdit(NumberLineEdit::DOUBLE);
        le_delta->setValue( p.maxDelta() );
        //le_u->setMinValue( p.minAllowedValue() );
        //le_u->setMaxValue( p.maxAllowedValue() );
        ttip = "max change per iteration ";
        ttip.append(tooltipText);
        le_delta->setToolTip(ttip);
        //setCellWidget(i,5,le_delta);
        mainLayout->addWidget(le_delta, i+1, 5);
        maxDeltaNLE.append(le_delta);

        connect(ch_v, SIGNAL(toggled(bool)), SLOT(onCheckboxToggled(bool)));
        connect(le_v, SIGNAL(editingFinished()), SLOT(onValueChanged()));
        connect(le_u, SIGNAL(editingFinished()), SLOT(onValueChanged()));
        connect(le_l, SIGNAL(editingFinished()), SLOT(onValueChanged()));
        connect(le_delta, SIGNAL(editingFinished()), SLOT(onValueChanged()));

        connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGuiSettingsChanged()));

        // TODO: checkbox vor variate status
    }
}


FT_FitParamTable::~FT_FitParamTable()
{

}


/**
 * @brief FT_FitParamTable::fitParameters returns a list of
 * fit parameters representing the current user selection
 * @return
 */
QList<FitParameter> FT_FitParamTable::fitParameters()
{
    return fparams;
}


/**
 * @brief FT_FitParamTable::onCheckboxToggled This slot is executed if Fixparameter checkbox is clicked
 * @param checked
 */
void FT_FitParamTable::onCheckboxToggled(bool state)
{
    QCheckBox* sender = dynamic_cast<QCheckBox*>(QObject::sender());
    if(!sender)
        return;

    int row = constantCheckboxes.indexOf(sender);

    fparams[row].setEnabled(!state);

    //qDebug() << "FitParamTable: Row:" << row << sender->parentWidget()->pos() << " - " << !state;*/
}


/**
 * @brief FT_FitParamTable::onValueChanged This slot is executed
 * when the value of any double input field (initial value, lower/upper boundary)
 * has been edited by the user.
 */
void FT_FitParamTable::onValueChanged()
{
    NumberLineEdit* sender = dynamic_cast<NumberLineEdit*>(QObject::sender());
    if(!sender)
        return;

    sender->blockSignals(true);

    int row, col, rspan, cspan, index;
    index = mainLayout->indexOf(sender);
    mainLayout->getItemPosition(index, &row, &col, &rspan, &cspan);
    row -= 1;

    double val = sender->getValue();

    if(col == 2) // initial value changed
    {
        bool res = fparams[row].setValue(val);
        if(!res)
        {
            sender->setValue(fparams[row].value());
            val = fparams[row].value();
        }

        if(row == 0)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierParDiaValue, val);
        else if(row == 1)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierGasTempValue, val);
        else if(row == 2)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierPeakTempValue, val);
    }
    else if(col == 3) // lower boundary changed
    {
        bool res = fparams[row].setLowerBoundary(val);
        if(!res)
        {
            sender->setValue(fparams[row].lowerBound());
            val = fparams[row].lowerBound();
        }

        if(row == 0)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierParDiaLowBound, val);
        else if(row == 1)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierGasTempLowBound, val);
        else if(row == 2)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierPeakTempLowBound, val);
    }
    else if(col == 4) // upper boundary changed
    {
        bool res = fparams[row].setUpperBoundary(val);
        if(!res)
        {
            sender->setValue(fparams[row].upperBound());
            val = fparams[row].upperBound();
        }

        if(row == 0)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierParDiaUppBound, val);
        else if(row == 1)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierGasTempUppBound, val);
        else if(row == 2)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierPeakTempUppBound, val);
    }
    else if(col == 5) // max delta
    {
        bool res = fparams[row].setMaxDelta(val);
        if(!res)
        {
            sender->setValue(fparams[row].maxDelta());
            val = fparams[row].maxDelta();
        }

        if(row == 0)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierParDiaMaxDelta, val);
        else if(row == 1)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierGasTempMaxDelta, val);
        else if(row == 2)
            Core::instance()->guiSettings->setValue(identifierFitInitParam, identifierPeakTempMaxDelta, val);
    }

    sender->blockSignals(false);
}


void FT_FitParamTable::onGuiSettingsChanged()
{
    GuiSettings *guiSettings = Core::instance()->guiSettings;

    //initial values

    if(guiSettings->hasEntry(identifierFitInitParam, identifierParDiaValue))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierParDiaValue, 0).toDouble();
        bool res = fparams[0].setValue(val);
        if(!res)
            val = fparams[0].value();
        static_cast<NumberLineEdit*>(valueNLE.at(0))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierGasTempValue))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierGasTempValue, 0).toDouble();
        bool res = fparams[1].setValue(val);
        if(!res)
            val = fparams[1].value();
        static_cast<NumberLineEdit*>(valueNLE.at(1))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierPeakTempValue))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierPeakTempValue, 0).toDouble();
        bool res = fparams[2].setValue(val);
        if(!res)
            val = fparams[2].value();
        static_cast<NumberLineEdit*>(valueNLE.at(2))->setValue(val);
    }

    // lower boundary

    if(guiSettings->hasEntry(identifierFitInitParam, identifierParDiaLowBound))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierParDiaLowBound, 0).toDouble();
        bool res = fparams[0].setLowerBoundary(val);
        if(!res)
            val = fparams[0].lowerBound();
        static_cast<NumberLineEdit*>(lowBoundNLE.at(0))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierGasTempLowBound))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierGasTempLowBound, 0).toDouble();
        bool res = fparams[1].setLowerBoundary(val);
        if(!res)
            val = fparams[1].lowerBound();
        static_cast<NumberLineEdit*>(lowBoundNLE.at(1))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierPeakTempLowBound))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierPeakTempLowBound, 0).toDouble();
        bool res = fparams[2].setLowerBoundary(val);
        if(!res)
            val = fparams[2].lowerBound();
        static_cast<NumberLineEdit*>(lowBoundNLE.at(2))->setValue(val);
    }

    // upper boundary

    if(guiSettings->hasEntry(identifierFitInitParam, identifierParDiaUppBound))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierParDiaUppBound, 0).toDouble();
        bool res = fparams[0].setUpperBoundary(val);
        if(!res)
            val = fparams[0].upperBound();
        static_cast<NumberLineEdit*>(uppBoundNLE.at(0))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierGasTempUppBound))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierGasTempUppBound, 0).toDouble();
        bool res = fparams[1].setUpperBoundary(val);
        if(!res)
            val = fparams[1].upperBound();
        static_cast<NumberLineEdit*>(uppBoundNLE.at(1))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierPeakTempUppBound))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierPeakTempUppBound, 0).toDouble();
        bool res = fparams[2].setUpperBoundary(val);
        if(!res)
            val = fparams[2].upperBound();
        static_cast<NumberLineEdit*>(uppBoundNLE.at(2))->setValue(val);
    }

    // max delta

    if(guiSettings->hasEntry(identifierFitInitParam, identifierParDiaMaxDelta))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierParDiaMaxDelta, 0).toDouble();
        bool res = fparams[0].setMaxDelta(val);
        if(!res)
            val = fparams[0].maxDelta();
        static_cast<NumberLineEdit*>(maxDeltaNLE.at(0))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierGasTempMaxDelta))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierGasTempMaxDelta, 0).toDouble();
        bool res = fparams[1].setMaxDelta(val);
        if(!res)
            val = fparams[1].maxDelta();
        static_cast<NumberLineEdit*>(maxDeltaNLE.at(1))->setValue(val);
    }

    if(guiSettings->hasEntry(identifierFitInitParam, identifierPeakTempMaxDelta))
    {
        double val = guiSettings->value(identifierFitInitParam, identifierPeakTempMaxDelta, 0).toDouble();
        bool res = fparams[2].setMaxDelta(val);
        if(!res)
            val = fparams[2].maxDelta();
        static_cast<NumberLineEdit*>(maxDeltaNLE.at(2))->setValue(val);
    }
}

