#include "generalsettingswidget.h"

#include <QThread>

#include "../../core.h"

/**
 * @brief GeneralSettingsWidget::GeneralSettingsWidget Constructor
 * @param parent parent widget
 */
GeneralSettingsWidget::GeneralSettingsWidget(QWidget *parent) :   QGroupBox(parent)
{

    setTitle("General Settings");

    // init gui

    layMainV = new QVBoxLayout;
    setLayout(layMainV);

    cbImportCores = new LabeledComboBox("CPU-Cores used for import:");
    cbImportCores->setToolTip("Adjust the amount of CPU-Cores, which are used during data import.");
    layMainV->addWidget(cbImportCores);

    int maxCores = QThread::idealThreadCount();
    for(int i = 0; i < maxCores; i++)
    {

        cbImportCores->addStringItem(QString("%0").arg(i+1));

    }

#ifdef LIISIM_FULL
    cbLocales = new LabeledComboBox("Number Format:");
    cbLocales->addStringItem("0.00");
    cbLocales->addStringItem("0,00");
    layMainV->addWidget(cbLocales);

    connect(cbLocales, SIGNAL(currentIndexChanged(int)),
            SLOT(onLocalesCurrentIndexChanged(int)));
#endif

    QHBoxLayout *layoutAutoSaveSettings = new QHBoxLayout;
    checkboxAutoSaveSettings = new QCheckBox("Auto save session every", this);
    spinboxAutoSaveTime = new QSpinBox(this);

    layoutAutoSaveSettings->setMargin(0);
    layoutAutoSaveSettings->addWidget(checkboxAutoSaveSettings);
    layoutAutoSaveSettings->addWidget(spinboxAutoSaveTime);
    layoutAutoSaveSettings->addWidget(new QLabel("minutes", this));

    layMainV->addLayout(layoutAutoSaveSettings);

    buttonTutorial = new QPushButton("Show all tutorials again", this);

    layMainV->addWidget(buttonTutorial);


    buttonResetSplitter = new QPushButton("Reset saved splitter positions", this);
    buttonResetSplitter->setToolTip("Restores default window splitter positions. Will take effect after a restart.");

    layMainV->addWidget(buttonResetSplitter);


    // init connections

    connect(cbImportCores,SIGNAL(currentIndexChanged(int)),
            SLOT(onCoreCountImportEdited(int)));

    connect(Core::instance()->generalSettings,SIGNAL(settingsChanged()),
            SLOT(onGeneralSettingsChanged()));

    connect(checkboxAutoSaveSettings, SIGNAL(stateChanged(int)),
            SLOT(onCheckboxAutoSaveSettingsStateChanged(int)));

    connect(spinboxAutoSaveTime, SIGNAL(valueChanged(int)),
            SLOT(onSpinboxAutoSaveTimeValueChanged(int)));

    connect(buttonResetSplitter, SIGNAL(clicked(bool)), SLOT(onButtonClicked()));

    onGeneralSettingsChanged();
}


/**
 * @brief GeneralSettingsWidget::onCoreCountImportEdited This slot is executed
 * if the user changed the selected value in the "CPU-Cores used for import" combobox.
 * In this case we need to update the GeneralSettings object.
 * @param idx new selected index
 */
void GeneralSettingsWidget::onCoreCountImportEdited(int idx)
{
    Core::instance()->generalSettings->setCoreCountImport(idx+1);
}


/**
 * @brief GeneralSettingsWidget::onGeneralSettingsChanged This slot is
 * executed if any settings value of the global GeneralSettings has been
 * changed (the global GeneralSettings are stored within the Core object).
 * All GUI-Elements should be updated here.
 */
void GeneralSettingsWidget::onGeneralSettingsChanged()
{
    int cci = Core::instance()->generalSettings->coreCountImport();
    if(cbImportCores->getCurrentIndex() != cci-1 )
        cbImportCores->setCurrentIndex(cci-1);

#ifdef LIISIM_FULL
    if(Core::instance()->generalSettings->getLocale().language() == QLocale::German)
    {
        cbLocales->setCurrentIndex(1);
        onLocalesCurrentIndexChanged(1);
    }
    else
    {
        onLocalesCurrentIndexChanged(0);
        cbLocales->setCurrentIndex(0);
    }
#endif

    if(checkboxAutoSaveSettings->isChecked() != Core::instance()->generalSettings->getAutoSaveSettings())
    {
        Core::instance()->autoSaveTimer->setInterval(Core::instance()->generalSettings->getAutoSaveSettingsTime() * 60000);
        if(Core::instance()->generalSettings->getAutoSaveSettings())
            Core::instance()->autoSaveTimer->start();
    }

    checkboxAutoSaveSettings->blockSignals(true);
    checkboxAutoSaveSettings->setChecked(Core::instance()->generalSettings->getAutoSaveSettings());
    checkboxAutoSaveSettings->blockSignals(false);

    spinboxAutoSaveTime->blockSignals(true);
    spinboxAutoSaveTime->setValue(Core::instance()->generalSettings->getAutoSaveSettingsTime());
    spinboxAutoSaveTime->blockSignals(false);
}


void GeneralSettingsWidget::onLocalesCurrentIndexChanged(int index)
{
#ifdef LIISIM_FULL
    if(index == 0)
        Core::instance()->generalSettings->setLocale(QLocale(QLocale::C));
    else if(index == 1)
        Core::instance()->generalSettings->setLocale(QLocale(QLocale::German));
#endif
}

/**
 * @brief GeneralSettingsWidget::onCheckboxAutoSaveSettingsStateChanged
 * @param state
 * Called when the state of checkboxAutoSaveSettings changes. Saves the state
 * in GeneralSettings, sets and (re)starts the timer.
 */
void GeneralSettingsWidget::onCheckboxAutoSaveSettingsStateChanged(int state)
{
    if(state == Qt::Checked)
    {
        Core::instance()->generalSettings->setAutoSaveSettings(true);
        Core::instance()->autoSaveTimer->setInterval(spinboxAutoSaveTime->value() * 60000);
        Core::instance()->autoSaveTimer->start();
    }
    else
    {
        Core::instance()->generalSettings->setAutoSaveSettings(false);
        Core::instance()->autoSaveTimer->stop();
    }
}


/**
 * @brief GeneralSettingsWidget::onSpinboxAutoSaveTimeValueChanged
 * @param value
 * Called when the value of spinboxAutoSaveTime changes. Saves the value in
 * GeneralSettings, sets and (re)starts the timer.
 */
void GeneralSettingsWidget::onSpinboxAutoSaveTimeValueChanged(int value)
{
    Core::instance()->generalSettings->setAutoSaveSettingsTime(value);
    Core::instance()->autoSaveTimer->setInterval(value * 60000);
    if(checkboxAutoSaveSettings->isChecked())
        Core::instance()->autoSaveTimer->start();
}


void GeneralSettingsWidget::onButtonClicked()
{
    if(QObject::sender() == buttonResetSplitter)
    {
        Core::instance()->guiSettings->resetSplitterPositions();
    }
}



