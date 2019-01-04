#ifndef MATERIALEDITOR_H
#define MATERIALEDITOR_H

#include "dbeditorwidget.h"
#include "../../general/LIISimMessageType.h"

#include <QLineEdit>
#include <QTableWidget>

#include "tablerowelement.h"

/**
 * @brief The MaterialEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class MaterialEditor : public DbEditorWidget
{
    Q_OBJECT
public:
    explicit MaterialEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager* dbm);

private:
    void updateCurrentView();
    void updateCalculatedProperties();
    void showPropertyInRow(const Property& prop, int row);
    void showOpticalPropertyInRow(const OpticalProperty &prop, int &row);

    QTableWidget* table;
    QTableWidget* table_calc;

    QList<DBETableRowElement*> _tableRows;
    QList<DBETableRowElement*> _emTableRows;
    QList<Property> _emValues;
    int _emRowStart;
    bool _rowDataChanged;
    bool _blockInit;

private slots:
    void onRowDataChanged();
    void onEmRowDataChanged();
    void onEmRowDeleteRequest();
    void onEmRowAddRequest();

public slots:

    void initData();
    void onSelectionChanged(const QItemSelection &selection);
    void onApplyChanges();
    void onAddItemToList();
    void onRemoveCurrentSelectionFromList();

};

#endif // MATERIALEDITOR_H
