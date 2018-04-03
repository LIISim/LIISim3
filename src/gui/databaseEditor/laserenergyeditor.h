#ifndef LASERENERGYEDITOR_H
#define LASERENERGYEDITOR_H

#include "dbeditorwidget.h"
#include "../../general/LIISimMessageType.h"

#include <QLineEdit>
#include <QTableWidget>

class LaserEnergyEditor : public DbEditorWidget
{
    Q_OBJECT

    QTableWidget *table;

public:
    explicit LaserEnergyEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager *dbm);

private:

signals:

public slots:
    void initData();
    void onSelectionChanged(const QItemSelection &selection);
    void onApplyChanges();
    void onAddItemToList();
    void onRemoveCurrentSelectionFromList();

};

#endif // LASERENERGYEDITOR_H
