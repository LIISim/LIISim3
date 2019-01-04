#include "liisettingsnamedialog.h"

#include <QLabel>

#include "database/structure/liisettings.h"

LIISettingsNameDialog::LIISettingsNameDialog(QWidget *parent) : DBElementNameDialog(typeid(LIISettings), parent)
{
    QHBoxLayout *layoutChannels = new QHBoxLayout;
    layoutChannels->setContentsMargins(10, 0, 10, 0);

    QLabel *labelChannels = new QLabel("Channel count:", this);
    comboboxChannels = new QComboBox(this);
    QStringList channels;
    channels << "1" << "2" << "3" << "4";
    comboboxChannels->addItems(channels);
    comboboxChannels->setCurrentIndex(comboboxChannels->count()-1);

    layoutChannels->addWidget(labelChannels);
    layoutChannels->addWidget(comboboxChannels);
    layoutChannels->addStretch(-1);

    layout->insertLayout(1, layoutChannels);
}


int LIISettingsNameDialog::getChannelCount()
{
    return comboboxChannels->currentText().toInt();
}
