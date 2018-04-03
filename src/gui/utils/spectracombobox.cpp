#include "spectracombobox.h"

#include "../../core.h"


SpectraComboBox::SpectraComboBox(QWidget *parent) : QComboBox(parent)
{

    setToolTip("current Spectra selection");

    // subscribe to dbm
    connect(Core::instance()->getDatabaseManager(),
            SIGNAL(signal_contentChanged(int)),
            SLOT(onDatabaseContentChanged(int)));

    onDatabaseContentChanged();
}


SpectraComboBox::~SpectraComboBox()
{
}


void SpectraComboBox::onDatabaseContentChanged(int id)
{
    blockSignals(true);
    int lastIndex = currentIndex();
    QString lastText = currentText();
    QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getSpectra();
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


SpectrumDBE SpectraComboBox::currentSpectrum()
{
    SpectrumDBE spec;
    QString curtxt = currentText();
    QList<DatabaseContent*> dbc = *Core::instance()->getDatabaseManager()->getSpectra();
    for(int i = 0; i < dbc.size(); i++)
    {
        if(curtxt == dbc[i]->name)
        {
            spec = *Core::instance()->getDatabaseManager()->spectrum(dbc[i]->ident);
            return spec;
        }
    }
    return spec;
}
