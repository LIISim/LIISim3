#include "liisettingscombobox.h"

#include "../../core.h"


/**
 * @brief LIISettingsComboBox::LIISettingsComboBox Constructor
 * @param parent
 */
LIISettingsComboBox::LIISettingsComboBox(QWidget *parent) : QComboBox(parent)
{
    setToolTip("current LIISettings selection");

    // subscribe to dbm
    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged(int)),
            SLOT(onDatabaseContentChanged(int)));

    onDatabaseContentChanged();
}


/**
 * @brief LIISettingsComboBox::~LIISettingsComboBox Destructor
 */
LIISettingsComboBox::~LIISettingsComboBox()
{
}


/**
 * @brief LIISettingsComboBox::onDatabaseContentChanged This
 * slot is executed when the DatabaseManager has emitted
 * the databasecontentChanged() signal. The Combobox updates
 * its list of entries and tries to reselect the last selection.
 * @param id
 */
void LIISettingsComboBox::onDatabaseContentChanged(int id)
{
    blockSignals(true);
    int lastIndex = currentIndex();
    QString lastText = currentText();
    QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getLIISettings();
    clear();
    int newIndex = -1;

    for(int i = 0; i < dbc.size(); i++)
    {
        this->addItem(dbc[i]->name);
        if(lastText == dbc[i]->name)
            newIndex = i;
    }

    if(newIndex > -1)
        setCurrentIndex(newIndex);
    else if(lastIndex > -1 && lastIndex < count())
        setCurrentIndex(lastIndex);

    blockSignals(false);
}


/**
 * @brief LIISettingsComboBox::currentLIISettings
 * returns current selection
 * @return
 */
LIISettings LIISettingsComboBox::currentLIISettings()
{
    LIISettings l;
    QString curtxt = currentText();
    QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getLIISettings();
    for(int i = 0; i < dbc.size(); i++)
    {
        if(curtxt == dbc[i]->name)
        {
            l = *Core::instance()->getDatabaseManager()->liiSetting(dbc[i]->ident);
            return l;
        }
    }
    return l;
}

