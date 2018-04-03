#ifndef MINIMIZABLEWIDGET_H
#define MINIMIZABLEWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QVBoxLayout>

class MinimizableWidget : public QWidget
{
    Q_OBJECT
public:
    MinimizableWidget(QWidget *parent = 0, bool invArrow = false);

    void setTitle(QString title);

    void setWidget(QWidget *widget);

    bool invertArrow;

    QToolBar *toolbar;
    QAction *toggleVisibility;
    QLabel *labelTitle;

    QWidget *togglingWidget;

private slots:
    void onActionToggleVisibility();
};

#endif // MINIMIZABLEWIDGET_H
