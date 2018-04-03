#ifndef FT_FITLIST_H
#define FT_FITLIST_H

#include <QWidget>
#include <QTreeView>
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QSpinBox>
#include <QPushButton>

class FT_ResultVisualization;
class FT_DataVisualization;
class SimRun;
class FitRun;

class FT_FitList : public QWidget
{
    Q_OBJECT
public:
    FT_FitList(FT_ResultVisualization *visualizationWidget, FT_DataVisualization *dataVisualization, QWidget *parent = 0);

private:
    QTreeWidget *treeWidget;
    FT_ResultVisualization *visualizationWidget;
    FT_DataVisualization *dataVisualization;
    QPushButton *buttonClearList;
    QPushButton *buttonFit;
    QPushButton *buttonSim;
    QPushButton *buttonCancel;

    QAction *actionDelete;

private slots:
    void onNewFitRunRegistered(FitRun *fitRun);

    void onDataTableCellClicked(int row);

    void onButtonClearListClicked();
    void onButtonFitClicked();
    void onButtonSimClicked();
    void onButtonCancelClicked();

    void onFitStateChanged(bool state);

    void onContextMenuRequested(const QPoint &point);

    void onActionDelete();

public slots:
    void onNewSimRun(SimRun *simRun);
    //void resizeEvent(QResizeEvent *event);

    void onTreeWidgetItemClicked(QTreeWidgetItem *item, int column);

signals:
    void startFittingClicked();
    void startSimulationClicked();
    void cancelClicked();

};

#endif // FT_FITLIST_H
