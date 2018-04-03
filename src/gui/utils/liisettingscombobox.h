#ifndef LIISETTINGSCOMBOBOX_H
#define LIISETTINGSCOMBOBOX_H

#include <QComboBox>
#include "../../database/structure/liisettings.h"


/**
 * @brief The LIISettingsComboBox class represents
 * an intelligent combobox for the selection of LIISettings.
 * It listens to changes of the DatabaseManager and updates
 * its selection entries based on the available LIISettings.
 */
class LIISettingsComboBox : public QComboBox
{
    Q_OBJECT

    public:
        explicit LIISettingsComboBox(QWidget *parent = 0);
        ~LIISettingsComboBox();

        LIISettings currentLIISettings();

    signals:

    public slots:

    private slots:

        void onDatabaseContentChanged(int id = -1);
};

#endif // LIISETTINGSCOMBOBOX_H
