#ifndef MATERIALEDITOR_H
#define MATERIALEDITOR_H

#include "dbeditorwidget.h"
#include "../../general/LIISimMessageType.h"

#include <QLineEdit>
#include <QTableWidget>

/**
 * @brief The MaterialEditor class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class MaterialEditor : public DbEditorWidget
{
    Q_OBJECT

    QTableWidget* table;
    QTableWidget* table_calc;

public:

    explicit MaterialEditor(QWidget *parent = 0);

    void setDatabaseManager(DatabaseManager* dbm);

private:

    void showPropertyInRow(const Property& prop, int row);
    void showOpticalPropertyInRow(const OpticalProperty &prop, int &row);

signals:

public slots:

    void initData();
    void onSelectionChanged(const QItemSelection &selection);
    void onApplyChanges();
    void onAddItemToList();
    void onRemoveCurrentSelectionFromList();

     void updateCurrentView();

};

#endif // MATERIALEDITOR_H
