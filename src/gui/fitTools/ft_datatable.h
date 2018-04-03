#ifndef FT_DATATABLE_H
#define FT_DATATABLE_H

#include "../utils/extendedtablewidget.h"
#include "../../calculations/fit/fitdata.h"

class FT_DataTable : public QWidget
{
    Q_OBJECT
public:
    FT_DataTable(QWidget *parent = 0);

    void update(FitData &fitData, int iteration);
    void clear();

private:
    ExtendedTableWidget *tableWidget;
    QStringList headerLabels;

    int it_selection;

signals:
    void rowClicked(int row);

private slots:
    void onCellClicked(int row, int column = 0);

    void scrollResultsUp();
    void scrollResultsDown();
};

#endif // FT_DATATABLE_H
