#ifndef RIBBONTABBAR_H
#define RIBBONTABBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabBar>
#include <QToolBar>
#include <QPushButton>
#include <QButtonGroup>
#include <QAction>
#include <QStackedWidget>

/**
 * @brief The RibbonTabBar class MasterWindow Tab bar
 */
class RibbonTabBar : public QWidget
{
    Q_OBJECT
public:
    explicit RibbonTabBar(QWidget *parent = 0);
    ~RibbonTabBar();

private:

    int topBarHeight;
    int toolBarHeight;
    int minTabWidth;

    QAction* actionShowHideTB;

    QVBoxLayout*  layoutOuter;
    QHBoxLayout* layoutTabBar;

    QHBoxLayout* layoutTabButtons;

    QToolBar* topToolBarRight;
    QToolBar* topToolBarLeft;
    QButtonGroup* tabGroup;

    QStackedWidget* toolBarStack;


signals:

    void selectedTabChanged(int i);

public slots:

    void setSelectedTab(int i);
    void addTab(const QString& name,QToolBar* toolbar = 0);

    void addTopToolbarAction(QAction* action);

private slots:

    void onButtonToggled(int id, bool state);
    void onActionShowHideTb();

};

#endif // RIBBONTABBAR_H
