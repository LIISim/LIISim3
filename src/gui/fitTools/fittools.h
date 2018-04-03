#ifndef FITTOOLS_H
#define FITTOOLS_H

#include <QWidget>
#include <QToolBar>
#include <QStackedWidget>
#include <QActionGroup>
#include "../utils/ribbontoolbox.h"
#include "../../general/LIISimException.h"

class Core;

/**
 * @brief The FitTools class serves as a parent container for
 * the FitGenerationTool and the FitAnalysisTool and is shown
 * as a tab in the MasterWindow.
 */
class FitTools : public QWidget
{
    Q_OBJECT
public:
    explicit FitTools(QWidget *parent = 0);
    ~FitTools();

    inline QToolBar* toolbar(){return m_toolbar;}

private:

    QToolBar* m_toolbar;

    RibbonToolBox* viewToolbox;
    RibbonToolBox* rtbTool;

    QLabel* labelToolIcon;
    QLabel *labelToolName;

    QStackedWidget* widgetStack;
    QActionGroup* viewActionGroup;
    QAction* actionCalc;

signals:

public slots:

private slots:

    void onViewActionTriggered(QAction* action);

    void onCalcActionTriggered();
    void onProcessingStateChanged(bool state);

};

#endif // FITTOOLS_H
