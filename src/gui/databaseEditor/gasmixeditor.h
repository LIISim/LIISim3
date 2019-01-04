#ifndef GASMIXEDITORWIDGET_H
#define GASMIXEDITORWIDGET_H

#include "dbeditorwidget.h"

#include <QTableWidget>

#include "tablerowelement.h"
#include "gasmixturerow.h"

/**
 * @brief The GasMixEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class GasMixEditor : public DbEditorWidget
{
    Q_OBJECT
public:
    explicit GasMixEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager* dbm);

    //void paintEvent(QPaintEvent *event);

private:
    void updateGasTable();
    void updateProperties();
    void updateCalculatedProperties();
    void showPropertyInRow(const Property& prop, int row);

    QTableWidget* table_gases;
    QTableWidget* table_props;
    QTableWidget* table_calc;

    QList<DBETableRowElement*> _tableRows;
    DBEGasMixtureRow *_mixtureRow;
    bool _rowDataChanged;
    bool _blockInit;

private slots:
    void onRowDataChanged();

public slots:
    void initData();
    void onSelectionChanged(const QItemSelection &selection);
    void onApplyChanges();
    void onAddItemToList();
    void onRemoveCurrentSelectionFromList();
    void updateCurrentView();

};

#endif // GASMIXEDITORWIDGET_H
