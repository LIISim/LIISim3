#ifndef GASEDITOR_H
#define GASEDITOR_H

#include "dbeditorwidget.h"
#include "../../general/LIISimMessageType.h"
#include "tablerowelement.h"

#include <QLineEdit>
#include <QTableWidget>

/**
 * @brief The GasEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class GasEditor : public DbEditorWidget
{
    Q_OBJECT

    QTableWidget* table;

public:
    explicit GasEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager* dbm);

private:
    void updateView();

    QList<DBETableRowElement*> _tableRows;
    bool _rowDataChanged;
    bool _blockInit;

signals:
    /**
     * @brief emitted if changes on the database's gasproperties has been apllied
     */
    void signal_gasesUpdated();

private slots:
    void onRowDataChanged();

public slots:
    void initData();
    void onSelectionChanged(const QItemSelection &selection);
    void onApplyChanges();
    void onAddItemToList();
    void onRemoveCurrentSelectionFromList();
};

#endif // GASEDITOR_H
