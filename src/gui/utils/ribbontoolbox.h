#ifndef RIBBONTOOLBOX_H
#define RIBBONTOOLBOX_H

#include <QWidget>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

/**
 * @brief The RibbonToolBox
 * class For each tool tab:
 * buttons are grouped wihtin a RibbonToolBox
 * (for example AnalysisTools->SIGNAL DATA or VIEW)
 */
class RibbonToolBox : public QWidget
{
    Q_OBJECT
public:
    explicit RibbonToolBox(QString title,QWidget *parent = 0);
    ~RibbonToolBox();

    QVBoxLayout* layoutMainV;
    QGridLayout* layoutGrid;

    QLabel* sectionTitle;

signals:

public slots:

    void addWidget(QWidget* w, int row,int col, int rowspan = 1, int colspan = 1);

    void addViewAction(QAction* a,  int row, int col, int rowspan, int colspan);

    void addAction(QAction* a, int row, int col, int rowspan = 1, int colspan = 1,int iconsize = -1);

};

#endif // RIBBONTOOLBOX_H
