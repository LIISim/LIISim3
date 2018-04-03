#ifndef LIISETTINGSEDITOR_H
#define LIISETTINGSEDITOR_H

#include "dbeditorwidget.h"
#include <QTableWidget>

/**
 * @brief The LiiSettingsEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class LiiSettingsEditor : public DbEditorWidget
{
    Q_OBJECT

    public:

        explicit LiiSettingsEditor(QWidget *parent = 0);

        void setDatabaseManager(DatabaseManager* dbm);

        QTableWidget* table_channel;
        QTableWidget* table_filters;

    private:
        void showChannels(LIISettings* dbi);
        void showFilters(LIISettings* dbi);

    signals:

    public slots:

        void initData();
        void onSelectionChanged(const QItemSelection &selection);
        void onApplyChanges();
        void onAddItemToList();
        void onRemoveCurrentSelectionFromList();


};

#endif // LIISETTINGSEDITOR_H
