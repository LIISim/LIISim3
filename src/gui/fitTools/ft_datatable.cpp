#include "ft_datatable.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QShortcut>

#include "../../calculations/fit/fititerationresult.h"


FT_DataTable::FT_DataTable(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    it_selection = -1;

    tableWidget = new ExtendedTableWidget(this);

    headerLabels << "Dp /nm" << "Δ Dp" << "Tg /K" << "Δ Tg" << "Tpeak /K" << "Δ Tpeak" << "Chisquare" << "Lambda";

    tableWidget->setColumnCount(8);
    tableWidget->setAllRowHeight(22);
    tableWidget->setHorizontalHeaderLabels(headerLabels);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setEditable(false);

    int width = 70;
    tableWidget->setColumnWidth(0, width);
    tableWidget->setColumnWidth(1, width);
    tableWidget->setColumnWidth(2, width);
    tableWidget->setColumnWidth(3, width);
    tableWidget->setColumnWidth(4, width);
    tableWidget->setColumnWidth(5, width);
    tableWidget->setColumnWidth(6, width+10);
    tableWidget->setColumnWidth(7, width+10);

    QShortcut *shortcut_selUp = new QShortcut(QKeySequence(Qt::Key_Up), this);
    connect(shortcut_selUp, SIGNAL(activated()), SLOT(scrollResultsUp()));

    QShortcut *shortcut_selDown = new QShortcut(QKeySequence(Qt::Key_Down), this);
    connect(shortcut_selDown, SIGNAL(activated()), SLOT(scrollResultsDown()));

    layout()->addWidget(tableWidget);

    QHeaderView *vHeader = tableWidget->verticalHeader();
    connect(vHeader, SIGNAL(sectionClicked(int)), SLOT(onCellClicked(int)));
    connect(tableWidget, SIGNAL(cellClicked(int,int)), SLOT(onCellClicked(int,int)));
}


void FT_DataTable::update(FitData &fitData, int iteration)
{
    QList<QList<QString>> data;

    QList<QString> iterDataDp;
    QList<QString> iterDataDeltaDp;
    QList<QString> iterDataTg;
    QList<QString> iterDataDeltaTg;
    QList<QString> iterDataTpeak;
    QList<QString> iterDataDeltaTpeak;
    QList<QString> iterDataChisquare;
    QList<QString> iterDataLambda;

    for(int i = 0; i < fitData.iterationResCount(); i++)
    {
        iterDataDp.push_back(QString::number(fitData.iterationResult(i).at(2), 'f', 1));
        iterDataDeltaDp.push_back(QString::number(fitData.iterationResult(i).at(3), 'f', 3));
        iterDataTg.push_back(QString::number(fitData.iterationResult(i).at(4), 'f', 0));
        iterDataDeltaTg.push_back(QString::number(fitData.iterationResult(i).at(5), 'f', 2));
        iterDataTpeak.push_back(QString::number(fitData.iterationResult(i).at(6), 'f', 0));
        iterDataDeltaTpeak.push_back(QString::number(fitData.iterationResult(i).at(7), 'f', 2));
        iterDataChisquare.push_back(QString::number(fitData.iterationResult(i).at(0)));
        iterDataLambda.push_back(QString::number(fitData.iterationResult(i).at(1)));
    }

    data.push_back(iterDataDp);
    data.push_back(iterDataDeltaDp);
    data.push_back(iterDataTg);
    data.push_back(iterDataDeltaTg);
    data.push_back(iterDataTpeak);
    data.push_back(iterDataDeltaTpeak);
    data.push_back(iterDataChisquare);
    data.push_back(iterDataLambda);

    tableWidget->setData(headerLabels, data);

    tableWidget->scrollToBottom();

    tableWidget->selectRow(iteration);
}


void FT_DataTable::clear()
{
    tableWidget->setRowCount(0);
}


void FT_DataTable::onCellClicked(int row, int column)
{
    it_selection = row;

    tableWidget->blockSignals(true);
    tableWidget->selectRow(it_selection);
    tableWidget->blockSignals(false);

    emit rowClicked(row);
}


void FT_DataTable::scrollResultsUp()
{
    if(it_selection > 0)
    {
        it_selection--;
        onCellClicked(it_selection, 0);
    }
}


void FT_DataTable::scrollResultsDown()
{
    if(it_selection < tableWidget->rowCount()-1)
    {
        it_selection++;
        onCellClicked(it_selection, 0);
    }
}

