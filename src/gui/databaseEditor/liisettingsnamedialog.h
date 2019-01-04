#ifndef LIISETTINGSNAMEDIALOG_H
#define LIISETTINGSNAMEDIALOG_H

#include "dbelementnamedialog.h"

#include <QComboBox>

class LIISettingsNameDialog : public DBElementNameDialog
{
    Q_OBJECT
public:
    LIISettingsNameDialog(QWidget *parent = 0);

    int getChannelCount();

protected:
    QComboBox *comboboxChannels;

};

#endif // LIISETTINGSNAMEDIALOG_H
