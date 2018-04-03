#include "liifiltercombobox.h"
#include "core.h"


LIIFilterComboBox::LIIFilterComboBox(QWidget *parent) : QComboBox(parent)
{
    setToolTip("Current filter selected");

    connect(this, SIGNAL(currentIndexChanged(int)), SLOT(onIndexChanged(int)));
}


LIIFilterComboBox::~LIIFilterComboBox()
{

}


QString LIIFilterComboBox::currentFilterIdentifier()
{
    return this->currentText();
}


void LIIFilterComboBox::setAvailableFilters(LIISettings liiSettings)
{
    this->blockSignals(true);

    this->clear();
    for(int i = 0; i < liiSettings.filters.size(); i++)
    {
        this->addItem(liiSettings.filters.at(i).identifier);
    }

    if(Core::instance()->guiSettings->value("acquisition", "liifilter", "").toString() != "no Filter")
    {
        int findres = this->findText(Core::instance()->guiSettings->value("acquisition", "liifilter", "").toString());
        if(findres > -1)
        {
            this->setCurrentIndex(findres);
        }
        else
        {
            Core::instance()->guiSettings->setValue("acquisition", "liifilter", this->currentText());
        }
    }

    this->blockSignals(false);
}


void LIIFilterComboBox::onIndexChanged(int index)
{
    Core::instance()->guiSettings->setValue("acquisition", "liifilter", this->currentText());
}
