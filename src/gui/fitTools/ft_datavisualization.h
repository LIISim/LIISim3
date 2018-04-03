#ifndef FT_DATAVISUALIZATION_H
#define FT_DATAVISUALIZATION_H

#include <QWidget>
#include <QScrollArea>

class FitData;
class FitRun;
class SimRun;

class FT_DataTable;
class FT_ParameterTable;

class FT_DataVisualization : public QWidget
{
    Q_OBJECT
public:
    FT_DataVisualization(QWidget *parent = 0);

    void update(FitData &fitData, int iteration = 0);
    void update(FitRun *fitRun);
    void update(SimRun *simRun);

    void clearDataTable();
    void clearParameterTable();

private:
    FT_DataTable *mDataTable;
    FT_ParameterTable *mParameterTable;
    QScrollArea *mPTScrollArea;

signals:
    void dataTableRowClicked(int row);

private slots:
    void onDataTableRowClicked(int row);

};

#endif // FT_DATAVISUALIZATION_H
