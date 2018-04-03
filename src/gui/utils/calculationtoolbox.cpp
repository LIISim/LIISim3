#include "calculationtoolbox.h"

#include "core.h"

CalculationToolbox::CalculationToolbox(QWidget *parent) : RibbonToolBox("CALCULATION", parent)
{
    identifier_settings_group = "se";
    identifier_calcMode = "calcMode";
    identifier_calcRaw = "calcRaw";
    identifier_calcAbs = "calcAbs";
    identifier_calcTemp = "calcTemp";
    identifier_dbscan = "rescanDBBeforeCalculation";

    buttonRecalc = new QToolButton(this);
    buttonRecalc->setIcon(QIcon(Core::rootDir + "resources/icons/calculator.png"));
    buttonRecalc->setIconSize(QSize(16,16));
    buttonRecalc->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buttonRecalc->setText("Calculate");
    buttonRecalc->setToolTip("Calculate processing chain");
    buttonRecalc->setMinimumWidth(75);

    buttonCancel = new QToolButton(this);
    buttonCancel->setIcon(QIcon(Core::rootDir + "resources/icons/calculator_delete.png"));
    buttonCancel->setIconSize(QSize(16,16));
    buttonCancel->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buttonCancel->setText("Cancel");
    buttonCancel->setToolTip("Cancel calculation");
    buttonCancel->setMinimumWidth(75);

    buttonReset = new QToolButton(this);
    buttonReset->setIcon(QIcon(Core::rootDir + "resources/icons/table_refresh.png"));
    buttonReset->setIconSize(QSize(16,16));
    buttonReset->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buttonReset->setText("Reset");
    buttonReset->setToolTip("Reset calculation status column of all runs");
    buttonReset->setMinimumWidth(75);


    bgCalcModes = new QButtonGroup(this);

    rbCalcModeSingle = new QRadioButton("Single Run");
    rbCalcModeSingle->setToolTip("Calculate currently selected run only");
    rbCalcModeGroup = new QRadioButton("Group");
    rbCalcModeGroup->setToolTip("Calculate MRuns in currently selected group");
    rbCalcModeAll = new QRadioButton("All Runs");
    rbCalcModeAll->setToolTip("Calculate all loaded Runs");

    bgCalcModes->addButton(rbCalcModeSingle);
    bgCalcModes->addButton(rbCalcModeGroup);
    bgCalcModes->addButton(rbCalcModeAll);

    checkboxCalcRaw = new QCheckBox("Raw", this);
    checkboxCalcRaw->setToolTip("Calculate raw signals");
    checkboxCalcAbs = new QCheckBox("Absolute", this);
    checkboxCalcAbs->setToolTip("Calculate absolute signals");
    checkboxCalcTemp = new QCheckBox("Temperature  ", this);
    checkboxCalcTemp->setToolTip("Calculate temperature signals");

    materialComboBox = new MaterialComboBox;

    checkboxRescanDBAtRecalc = new QCheckBox("Rescan database before calculation", this);
    checkboxRescanDBAtRecalc->setToolTip("Rescan database files before calculating processing chain (if text files are edited manually)");

    layoutGrid->setContentsMargins(10,5,10,5);
    layoutGrid->setColumnMinimumWidth(0,90);
    layoutGrid->setColumnMinimumWidth(1,90);
    layoutGrid->setColumnMinimumWidth(2,90);
    layoutGrid->setColumnMinimumWidth(3,120);

    addWidget(buttonRecalc, 0, 0);
    addWidget(buttonCancel, 1, 0);
    addWidget(buttonReset, 2, 0);

    addWidget(rbCalcModeSingle, 0, 1);
    addWidget(rbCalcModeGroup, 1, 1);
    addWidget(rbCalcModeAll, 2, 1);

    addWidget(checkboxRescanDBAtRecalc, 0, 3);

    addWidget(checkboxCalcRaw, 0, 2);
    addWidget(checkboxCalcAbs, 1, 2);
    addWidget(checkboxCalcTemp, 2, 2);

    addWidget(new QLabel("Material for Spectroscopic Model:"), 1, 3);

    addWidget(materialComboBox, 2, 3);

    connect(buttonRecalc, SIGNAL(clicked(bool)), SLOT(onRecalcClicked()));
    connect(buttonReset, SIGNAL(clicked(bool)), SLOT(onResetClicked()));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(onCancelClicked()));

    connect(bgCalcModes, SIGNAL(buttonToggled(QAbstractButton*,bool)), SLOT(onCalcModeToggled(QAbstractButton*,bool)));

    connect(checkboxCalcRaw, SIGNAL(stateChanged(int)), SLOT(onCheckboxCalcTypeChanged()));
    connect(checkboxCalcAbs, SIGNAL(stateChanged(int)), SLOT(onCheckboxCalcTypeChanged()));
    connect(checkboxCalcTemp, SIGNAL(stateChanged(int)), SLOT(onCheckboxCalcTypeChanged()));

    connect(checkboxRescanDBAtRecalc, SIGNAL(stateChanged(int)), SLOT(onCheckboxRescanDBChanged()));

    connect(Core::instance()->getSignalManager(), SIGNAL(processingStateChanged(bool)), SLOT(onProcessingStateChanged(bool)));

    connect(Core::instance()->guiSettings, SIGNAL(guiSettingsChanged(QString,QString,QVariant)),
            SLOT(onGuiSettingsChanged(QString,QString,QVariant)));
    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGuiSettingsChanged()));
}


/**
 * @brief CalculationToolbox::onRecalcClicked This slot is executed when the "Calculate"-button
 * is clicked. Checks if the DB should be scanned and if there is already a processing task
 * running. Emits a signal containing which signal type should be processes.
 */
void CalculationToolbox::onRecalcClicked()
{
    if(checkboxRescanDBAtRecalc->isChecked())
        Core::instance()->getDatabaseManager()->slot_scanDatabase();

    if(Core::instance()->getSignalManager()->isBusy())
    {
        QString msg = "Cannot recalculate (active background tasks!)";
        MSG_WARN(msg);
        MSG_STATUS(msg);
        return;
    }

    QList<Signal::SType> typeList;
    if(checkboxCalcRaw->isChecked())
        typeList.push_back(Signal::RAW);
    if(checkboxCalcAbs->isChecked())
        typeList.push_back(Signal::ABS);
    if(checkboxCalcTemp->isChecked())
        typeList.push_back(Signal::TEMPERATURE);
    if(!typeList.isEmpty())
        emit recalc(typeList);
}


/**
 * @brief CalculationToolbox::onResetClicked This slot is executed
 * when the "Reset" Button (in toolbar) has been clicked. The calculation states
 * all runs will be reset
 */
void CalculationToolbox::onResetClicked()
{
    QList<MRun*> runs = Core::instance()->dataModel()->mrunList();
    for(int i = 0; i < runs.size(); i++)
        runs[i]->calculationStatus()->reset();
}


/**
 * @brief CalculationToolbox::onCancelClicked This slot is executed when the "Cancel"-button
 * has been clicked. All running processing tasks will be canceled.
 */
void CalculationToolbox::onCancelClicked()
{
    Core::instance()->getSignalManager()->cancelProcessingTasks();
}


/**
 * @brief CalculationToolbox::onCalcModeToggled This slot is executed
 * when the user changed the state of one of the calclulation mode radio buttons
 * @param button button which has been toggled
 * @param state new button state
 */
void CalculationToolbox::onCalcModeToggled(QAbstractButton *button, bool state)
{
    if(!state)
        return;

    int calcMode = 0;
    if(button == rbCalcModeSingle)
        calcMode = 0;
    else if(button == rbCalcModeGroup)
        calcMode = 1;
    else if(button == rbCalcModeAll)
        calcMode = 2;

    Core::instance()->getSignalManager()->setCalculationMode(calcMode);
    Core::instance()->guiSettings->setValue(identifier_settings_group, identifier_calcMode, calcMode, true);
}


/**
 * @brief CalculationToolbox::onCheckboxCalcTypeChanged This slot is executed when the user
 * changes the state of any calculation type checkboxes. Saves the state in the GuiSettings.
 */
void CalculationToolbox::onCheckboxCalcTypeChanged()
{
    if(QObject::sender() == checkboxCalcRaw)
        Core::instance()->guiSettings->setValue(identifier_settings_group, identifier_calcRaw, checkboxCalcRaw->isChecked(), true);
    else if(QObject::sender() == checkboxCalcAbs)
        Core::instance()->guiSettings->setValue(identifier_settings_group, identifier_calcAbs, checkboxCalcAbs->isChecked(), true);
    else if(QObject::sender() == checkboxCalcTemp)
        Core::instance()->guiSettings->setValue(identifier_settings_group, identifier_calcTemp, checkboxCalcTemp->isChecked(), true);
}


/**
 * @brief CalculationToolbox::onCheckboxRescanDBChanged This slot is executed when the user
 * changes the state of the "Rescan DB"-checkbox. Saves the state in the GuiSettings.
 */
void CalculationToolbox::onCheckboxRescanDBChanged()
{
    Core::instance()->guiSettings->setValue(identifier_settings_group, identifier_dbscan, checkboxRescanDBAtRecalc->isChecked(), true);
}


/**
 * @brief CalculationToolbox::onProcessingStateChanged Connected with the SignalManager to
 * be notified when the processing state changes. Enables/disables ui elements.
 * @param state
 */
void CalculationToolbox::onProcessingStateChanged(bool state)
{
    buttonRecalc->setEnabled(!state);
    buttonReset->setEnabled(!state);
}


/**
 * @brief CalculationToolbox::onGuiSettingsChanged Connected with the GuiSettings to enable
 * settings consistency between multiple CalculationToolboxes. For parameter see GuiSettings.
 * @param subgroup
 * @param key
 * @param value
 */
void CalculationToolbox::onGuiSettingsChanged(QString subgroup, QString key, QVariant value)
{
    if(subgroup == identifier_settings_group)
    {
        if(key == identifier_calcMode)
        {
            int calcMode = value.toInt();
            bgCalcModes->blockSignals(true);
            if(calcMode == 0)
                rbCalcModeSingle->setChecked(true);
            else if(calcMode == 1)
                rbCalcModeGroup->setChecked(true);
            else if(calcMode == 2)
                rbCalcModeAll->setChecked(true);
            bgCalcModes->blockSignals(false);
        }
        else if(key == identifier_calcRaw)
        {
            checkboxCalcRaw->blockSignals(true);
            checkboxCalcRaw->setChecked(value.toBool());
            checkboxCalcRaw->blockSignals(false);
        }
        else if(key == identifier_calcAbs)
        {
            checkboxCalcAbs->blockSignals(true);
            checkboxCalcAbs->setChecked(value.toBool());
            checkboxCalcAbs->blockSignals(false);
        }
        else if(key == identifier_calcTemp)
        {
            checkboxCalcTemp->blockSignals(true);
            checkboxCalcTemp->setChecked(value.toBool());
            checkboxCalcTemp->blockSignals(false);
        }
        else if(key == identifier_dbscan)
        {
            checkboxRescanDBAtRecalc->blockSignals(true);
            checkboxRescanDBAtRecalc->setChecked(value.toBool());
            checkboxRescanDBAtRecalc->blockSignals(false);
        }
    }
}


/**
 * @brief CalculationToolbox::onGuiSettingsChanged Executed when the GuiSettings are loaded.
 */
void CalculationToolbox::onGuiSettingsChanged()
{
    GuiSettings* gs = Core::instance()->guiSettings;

    // set Calculation Mode

    int calcMode = gs->value(identifier_settings_group, identifier_calcMode ,0).toInt();
    Core::instance()->getSignalManager()->setCalculationMode(calcMode);

    bgCalcModes->blockSignals(true);
    if(calcMode == 0)
        rbCalcModeSingle->setChecked(true);
    else if(calcMode == 1)
        rbCalcModeGroup->setChecked(true);
    else if(calcMode == 2)
        rbCalcModeAll->setChecked(true);
    bgCalcModes->blockSignals(false);

    checkboxRescanDBAtRecalc->blockSignals(true);
    checkboxRescanDBAtRecalc->setChecked(gs->value(identifier_settings_group, identifier_dbscan, false).toBool());
    checkboxRescanDBAtRecalc->blockSignals(false);

    checkboxCalcRaw->blockSignals(true);
    checkboxCalcAbs->blockSignals(true);
    checkboxCalcTemp->blockSignals(true);
    checkboxCalcRaw->setChecked(gs->value(identifier_settings_group, identifier_calcRaw, true).toBool());
    checkboxCalcAbs->setChecked(gs->value(identifier_settings_group, identifier_calcAbs, true).toBool());
    checkboxCalcTemp->setChecked(gs->value(identifier_settings_group, identifier_calcTemp, true).toBool());
    checkboxCalcRaw->blockSignals(false);
    checkboxCalcAbs->blockSignals(false);
    checkboxCalcTemp->blockSignals(false);
}



