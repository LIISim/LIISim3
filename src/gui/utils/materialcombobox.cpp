#include "materialcombobox.h"

#include "../../core.h"

MaterialComboBox::MaterialComboBox(QWidget *parent) : LabeledComboBox(parent)
{
    // USES SPECTROSCOPIC MATERIAL

    // indentation: remove description (is defined in CalculationToolBox)
    layMainH->removeWidget(description);

    mute = false;

    setToolTip("Material used for temperature signal calculation");

    setDatabaseContent(Core::instance()->getDatabaseManager()->getMaterials());
    onDatabaseContentChanged(-1);
    setCurrentItem(Core::instance()->modelingSettings->materialSpec().ident);

    connect(Core::instance()->modelingSettings,
            SIGNAL(settingsChanged()),
            SLOT(onModelingSettingsChanged()));

    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged(int)),
            SLOT(onDatabaseContentChanged(int)));

    connect(Core::instance()->modelingSettings,
            SIGNAL(materialSpecChanged()),
            SLOT(onModelingSettingsChanged()));
}


MaterialComboBox::~MaterialComboBox()
{
}


void MaterialComboBox::onCurrentIndexChanged(int idx)
{
    if(mute)
        return;

    MSG_ONCE_RESET_GROUP("SpectroscopicMaterial");

    blockSignals(true);    
    DatabaseContent* dbc = getSelectedDbContent();
    if(!dbc)
        return;
    mute = true;
    Core::instance()->modelingSettings->setMaterialSpec(dbc->filename);
    mute = false;
    blockSignals(false);
}


void MaterialComboBox::onModelingSettingsChanged()
{
    if(mute)
        return;
    blockSignals(true);
    setCurrentItem(Core::instance()->modelingSettings->materialSpec().ident);
    blockSignals(false);
}


void MaterialComboBox::onDatabaseContentChanged(int id)
{
    blockSignals(true);
    mute = true;
    slot_onDBcontentChanged();
    mute = false;
    blockSignals(false);

    if(id == -1)
        onModelingSettingsChanged();
}
