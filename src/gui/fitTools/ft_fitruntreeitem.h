#ifndef FT_FITRUNTREEITEM_H
#define FT_FITRUNTREEITEM_H

#include <QTreeWidgetItem>

#include "ft_resultvisualization.h"
#include "../../calculations/fit/fitrun.h"

class FT_FitRunTreeItem : public QObject, public QTreeWidgetItem
{
    friend class FT_FitList;
    friend class FT_FitRunTreeItem;

    Q_OBJECT
public:
    explicit FT_FitRunTreeItem(FT_ResultVisualization *visualizationWidget, FitRun *fitRun, QTreeWidgetItem *parent);
    ~FT_FitRunTreeItem();

private:
    void cleanup();

    FitRun *mFitRun;
    FT_ResultVisualization *mVisualizationWidget;
    QCheckBox *checkboxFitRun;
    QPushButton *buttonText;

private slots:
    void onFitFinished();
    void onChildStateChanged();
    void onCheckboxStateChanged(int state);
    void onButtonTextClicked();

signals:
    void changed(FT_FitRunTreeItem *item);
};

#endif // FT_FITRUNTREEITEM_H
