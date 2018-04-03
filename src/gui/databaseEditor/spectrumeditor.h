#ifndef SPECTRUMEDITOR_H
#define SPECTRUMEDITOR_H

#include "dbeditorwidget.h"
#include "../../general/LIISimMessageType.h"
#include "../utils/baseplotwidgetqwt.h"

#include <QLineEdit>
#include <QTableWidget>

class SpectrumEditor : public DbEditorWidget
{
    Q_OBJECT

    QTableWidget *table;
    BasePlotWidgetQwt *basePlot;
    BasePlotCurve *curve;

public:
    explicit SpectrumEditor(QWidget *parent = 0);

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

#endif // SPECTRUMEDITOR_H
