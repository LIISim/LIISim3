#ifndef GASMIXEDITORWIDGET_H
#define GASMIXEDITORWIDGET_H

#include "dbeditorwidget.h"

#include <QTableWidget>

/**
 * @brief The GasMixEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class GasMixEditor : public DbEditorWidget
{
    Q_OBJECT

    QTableWidget* table_gases;
    QTableWidget* table_props;
    QTableWidget* table_calc;

public:
    explicit GasMixEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager* dbm);

    //void paintEvent(QPaintEvent *event);

private:
    void showPropertyInRow(const Property& prop, int row);

signals:


public slots:

    void initData();
    void onSelectionChanged(const QItemSelection &selection);
    void onApplyChanges();
    void onAddItemToList();
    void onRemoveCurrentSelectionFromList();

    void updateCurrentView();
};

#endif // GASMIXEDITORWIDGET_H
