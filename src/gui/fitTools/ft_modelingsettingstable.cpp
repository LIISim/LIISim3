#include "ft_modelingsettingstable.h"

#include "../../core.h"


FT_ModelingSettingsTable::FT_ModelingSettingsTable(QWidget *parent) : QWidget(parent),
    unitConversion_pressure(1E-2) // Pa to mbar, for UI
{
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setMargin(5);
    mainLayout->setVerticalSpacing(0);
    setLayout(mainLayout);

    cbHtm = new QComboBox;
    cbGasmix = new QComboBox;
    cbMaterial = new QComboBox;
    lePressure = new NumberLineEdit(NumberLineEdit::DOUBLE, "Process pressure in mbar");
    lePressure->setMinValue(0.0);
    lePressure->setMaxValue(1E5);


    mainLayout->addWidget(new QLabel("Heat Transfer Model", this), 0, 0);
    mainLayout->addWidget(cbHtm, 0, 1);

    mainLayout->addWidget(new QLabel("Gas Mixture", this), 1, 0);
    mainLayout->addWidget(cbGasmix, 1, 1);

    mainLayout->addWidget(new QLabel("Material", this), 2, 0);
    mainLayout->addWidget(cbMaterial, 2, 1);

    mainLayout->addWidget(new QLabel("Process pressure [mbar]", this), 3, 0);
    mainLayout->addWidget(lePressure, 3, 1);

    // init heattransfermode combobox (list does not change during execution!)
    for(int i = 0; i < Core::instance()->heatTransferModels.size();i++)
        cbHtm->addItem(Core::instance()->heatTransferModels[i]->name);

#ifdef LIISIM_FULL
    checkboxEvaporation = new QCheckBox("Evaporation", this);
    checkboxEvaporation->setChecked(true);
    mainLayout->addWidget(checkboxEvaporation, 0, 2);

    checkboxConduction = new QCheckBox("Conduction", this);
    checkboxConduction->setChecked(true);
    mainLayout->addWidget(checkboxConduction, 1, 2);

    checkboxRadiation = new QCheckBox("Radiation", this);
    checkboxRadiation->setChecked(true);
    mainLayout->addWidget(checkboxRadiation, 2, 2);

    connect(checkboxEvaporation, SIGNAL(stateChanged(int)), SLOT(onCheckboxStateChanged()));
    connect(checkboxConduction, SIGNAL(stateChanged(int)), SLOT(onCheckboxStateChanged()));
    connect(checkboxRadiation, SIGNAL(stateChanged(int)), SLOT(onCheckboxStateChanged()));
#endif

    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mainLayout->addWidget(spacer, 0, 3);

    // SIGNALS
    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged(int)),
            SLOT(onDBCchanged(int)));

    connect(Core::instance()->modelingSettings,
            SIGNAL(settingsChanged()),
            SLOT(onModelingSettingsChanged()));


    connect(cbHtm,
            SIGNAL(currentIndexChanged(int)),
            SLOT(onCbHtmEdited(int)));

    connect(cbGasmix,
            SIGNAL(currentIndexChanged(int)),
            SLOT(onCbGasmixEdited(int)));

    connect(cbMaterial,
            SIGNAL(currentIndexChanged(int)),
            SLOT(onCbMaterialEdited(int)));

    onDBCchanged();
    onModelingSettingsChanged();
}


FT_ModelingSettingsTable::~FT_ModelingSettingsTable() {}


/**
 * @brief FT_ModelingSettingsTable::getPressure checks and returns pressure
 * @return pressure in [Pa]
 */
double FT_ModelingSettingsTable::getPressure()
{
    return lePressure->getValueWithinLimits() / unitConversion_pressure;
}


/**
 * @brief FT_ModelingSettingsTable::resizeEvent Custom resizing behavior
 * @param event
 */
/*void FT_ModelingSettingsTable::resizeEvent(QResizeEvent *event)
{
    int wcol0 = 280;
    int wcol1 = 100;

    if(width() > wcol0+wcol1+2)
        wcol1 = width()-wcol0-2;

    setColumnWidth(0,wcol0);
    setColumnWidth(1,wcol1);
}*/


// ----------------------------
// handle program state changes
// ----------------------------

/**
 * @brief FT_ModelingSettingsTable::onDBCchanged This slot
 * is executed when the database content has been modified
 * (eg. rescan ...).
 * @param dbcid
 */
void FT_ModelingSettingsTable::onDBCchanged(int dbcid)
{
    DatabaseManager* dbm = Core::instance()->getDatabaseManager();

    // update material selection options
    cbMaterial->blockSignals(true);
    QString cursel = cbMaterial->currentText();
    cbMaterial->clear();
    QList<DatabaseContent*> materials = *dbm->getMaterials();
    int curidx = -1;
    for(int i = 0; i < materials.size(); i++)
    {
        cbMaterial->addItem(materials[i]->name);
        cbMaterial->setItemData(i,materials[i]->ident);
        if(materials[i]->name == cursel)
            curidx = i;
    }
    if(curidx > -1)
        cbMaterial->setCurrentIndex(curidx);
    cbMaterial->blockSignals(false);

    // update gasmixture selection options
    cbGasmix->blockSignals(true);
    cursel = cbGasmix->currentText();
    cbGasmix->clear();
    QList<DatabaseContent*> gasmixes = *dbm->getGasMixtures();
    curidx = -1;
    for(int i = 0; i < gasmixes.size(); i++)
    {
        cbGasmix->addItem(gasmixes[i]->name);
        cbGasmix->setItemData(i,gasmixes[i]->ident);
        if(gasmixes[i]->name == cursel)
            curidx = i;
    }
    if(curidx > -1)
        cbGasmix->setCurrentIndex(curidx);
    cbGasmix->blockSignals(false);
}


/**
 * @brief FT_ModelingSettingsTable::onModelingSettingsChanged This
 * slot is executed when the (global) ModelingSettings were changed.
 */
void FT_ModelingSettingsTable::onModelingSettingsChanged()
{
    // update heattransfermodel selection

    if(Core::instance()->modelingSettings->heatTransferModel() != NULL)
    {
        cbHtm->blockSignals(true);
        QString htm = Core::instance()->modelingSettings->heatTransferModel()->identifier;

        for(int i = 0; i < Core::instance()->heatTransferModels.size(); i++)
            if(Core::instance()->heatTransferModels[i]->identifier == htm)
            {
                cbHtm->setCurrentIndex(i);
                cbHtm->setToolTip(Core::instance()->heatTransferModels[i]->description);
                break;
            }
        cbHtm->blockSignals(false);
    }


    // update material selection
    int ident = Core::instance()->modelingSettings->material().ident;
    cbMaterial->blockSignals(true);
    for(int i = 0; i < cbMaterial->count(); i++)
        if(cbMaterial->itemData(i).toInt() == ident)
        {
            cbMaterial->setCurrentIndex(i);
            break;
        }
    cbMaterial->blockSignals(false);

    // update gasmixture selection
    ident = Core::instance()->modelingSettings->gasMixture().ident;
    cbGasmix->blockSignals(true);
    for(int i = 0; i < cbGasmix->count(); i++)
        if(cbGasmix->itemData(i).toInt() == ident)
        {
            cbGasmix->setCurrentIndex(i);
            break;
        }
    cbGasmix->blockSignals(false);

    lePressure->blockSignals(true);
    lePressure->setValue(Core::instance()->modelingSettings->processPressure() * unitConversion_pressure);
    lePressure->blockSignals(false);
}


// ----------------------------
// handle user interaction
// ----------------------------


/**
 * @brief FT_ModelingSettingsTable::onCbHtmEdited This
 * slot is executed when the user changed the selection of
 * the HeatTransferModel combobox.
 * @param idx selected index
 */
void FT_ModelingSettingsTable::onCbHtmEdited(int idx)
{
    if(idx < 0) return;

    // update modeling settings
    Core::instance()->modelingSettings->setHeatTransferModel(idx);

    HeatTransferModel *htm = Core::instance()->modelingSettings->heatTransferModel();

    #ifdef LIISIM_FULL
        checkboxConduction->blockSignals(true);
        checkboxEvaporation->blockSignals(true);
        checkboxRadiation->blockSignals(true);

        checkboxConduction->setChecked(htm->useConduction);
        checkboxEvaporation->setChecked(htm->useEvaporation);
        checkboxRadiation->setChecked(htm->useRadiation);

        checkboxConduction->blockSignals(false);
        checkboxEvaporation->blockSignals(false);
        checkboxRadiation->blockSignals(false);
    #endif
}


/**
 * @brief FT_ModelingSettingsTable::onCbGasmixEdited This
 * slot is executed when the user changed the selection of
 * the gasmixture combobox.
 * @param idx selected index
 */
void FT_ModelingSettingsTable::onCbGasmixEdited(int idx)
{
    if(idx < 0) return;

    // update modeling settings

    int ident = cbGasmix->itemData(idx).toInt();
    GasMixture* g = Core::instance()->getDatabaseManager()->gasMixture(ident);
    if(!g)return;
    Core::instance()->modelingSettings->setGasMixture(g->filename);
}


/**
 * @brief FT_ModelingSettingsTable::onCbMaterialEdited This
 * slot is executed when the user changed the selection of
 * the material combobox.
 * @param idx selected index
 */
void FT_ModelingSettingsTable::onCbMaterialEdited(int idx)
{
    if(idx < 0)  return;

    // update modeling settings

    int ident = cbMaterial->itemData(idx).toInt();
    Material* m = Core::instance()->getDatabaseManager()->material(ident);
    if(!m)return;
    Core::instance()->modelingSettings->setMaterial(m->filename);
}


void FT_ModelingSettingsTable::onCheckboxStateChanged()
{
    HeatTransferModel *htm = Core::instance()->modelingSettings->heatTransferModel();

    if(QObject::sender() == checkboxConduction)
        htm->useConduction = checkboxConduction->isChecked();
    else if(QObject::sender() == checkboxEvaporation)
        htm->useEvaporation = checkboxEvaporation->isChecked();
    else if(QObject::sender() == checkboxRadiation)
        htm->useRadiation = checkboxRadiation->isChecked();
}
