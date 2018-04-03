#include "datatablewidget.h"


#include <QStringList>
#include <QTableWidgetItem>
#include <QDebug>
#include <QHeaderView>
#include <QShortcut>
#include <QClipboard>
#include <QApplication>

#include <QMenu>
#include <QTime>
#include <QThreadPool>

#include "customQwtPlot/baseplotcurve.h"
#include "core.h"

/**
 * @brief DataTableWidget::DataTableWidget constructor
 * @param curves list of plot curves
 * @param parent parent widget
 *
 * TODO: pass List of Signals instead of plot curves directly!!!
 * TODO: fill table in seperate qrunnable to avoid gui freezing !!!
 */
DataTableWidget::DataTableWidget(QList<BasePlotCurve *> *curves, Signal::SType stype, QWidget *parent) :  QWidget(parent)
{
    this->curves = curves;
    this->stype = stype;
    this->stdevCurves = nullptr;
    setup();
    //sigManager = NULL;

}


DataTableWidget::DataTableWidget(QList<BasePlotCurve*> *curves, QList<SignalPlotIntervalCurve*> *stdevCurves, Signal::SType stype, QWidget *parent) : QWidget(parent), _number_precision(6)
{
    this->curves = curves;
    this->stype = stype;
    this->stdevCurves = stdevCurves;
    setup();
}


void DataTableWidget::setup()
{
    this->setGeometry(400,400,400,350);
    layMain = new QGridLayout(this);
    setLayout(layMain);
    layMain->setMargin(0);

    _number_precision = 6;

    //table =new QTableWidget(5,5);
    //table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //table->verticalHeader()->hide();
    tableView = new QTableView(this);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->verticalHeader()->hide();

    menuBar = new QMenuBar;
    menuBar->setMinimumHeight(22);

    labelNoCurveData = new QLabel(" No curve data available");
    labelNoCurveData->setStyleSheet("QLabel { color : red }");
    labelNoCurveData->setVisible(false);

    layMain->addWidget(menuBar, 0, 0);
    layMain->addWidget(labelNoCurveData, 1, 0);
    layMain->addWidget(tableView, 2, 0);

    layMain->setAlignment(menuBar, Qt::AlignTop);
    layMain->setAlignment(labelNoCurveData, Qt::AlignCenter | Qt::AlignTop);


    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+C"), this);

    actionCopyToClipboard = new QAction(tr("copy selection to clipboard"),this);
    connect(actionCopyToClipboard,SIGNAL(triggered()),this,SLOT(onCopySelection()));
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(onContextMenuEvent(QPoint)));
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(shortcut, SIGNAL(activated()), this, SLOT(onCopySelection()));
    updateView();

    mOptions = new QMenu(tr("Options"),this);
    menuBar->addMenu(mOptions);

    actionShowStdev = new QAction("Show Standard Deviation Signals", this);
    actionShowStdev->setCheckable(true);
    actionShowStdev->setChecked(false);

    actionOmitHeader = new QAction("Omit Header Text", this);
    actionOmitHeader->setCheckable(true);
    actionOmitHeader->setChecked(Core::instance()->guiSettings->value("dataTable", "omitHeader", false).toBool());

    actionInterpolate = new QAction("Interpolate to smallest dt", this);
    actionInterpolate->setCheckable(true);
    actionInterpolate->setChecked(false);

    if(stdevCurves != nullptr)
        mOptions->addAction(actionShowStdev);
    mOptions->addAction(actionOmitHeader);
    mOptions->addAction(actionInterpolate);

    connect(actionShowStdev, SIGNAL(triggered(bool)), SLOT(onActionShowStdev()));
    connect(actionOmitHeader, SIGNAL(toggled(bool)), SLOT(onActionOmitHeader(bool)));
    connect(actionInterpolate, SIGNAL(triggered(bool)), SLOT(onActionInterpolate()));

    mrunPostMode = false;
}


/**
 * @brief DataTableWidget::updateView updates the data table
 * visualization.
 * @details This method shuld be called if we have changed the contents
 * of the plot curves.
 */
void DataTableWidget::updateView()
{
    // update view only if widget is visible
    if(!this->isVisible())
        return;

    if(mrunPostMode)
        return;

    if(curves == NULL || curves->isEmpty())
    {
        QStandardItemModel *newModel = new QStandardItemModel;
        newModel->setColumnCount(0);
        newModel->setRowCount(0);
        QAbstractItemModel *oldModel = tableView->model();
        tableView->setModel(newModel);
        delete oldModel;

        labelNoCurveData->setVisible(true);
        tableView->setVisible(false);

        return;
    }

    labelNoCurveData->setVisible(false);
    tableView->setVisible(true);

    QStandardItemModel *model = new QStandardItemModel;
    model->setColumnCount(0);
    model->setRowCount(0);

    QStringList columnNames;

    if(actionInterpolate->isChecked())
    {
        QMap<QString, Signal> tempData;

        columnNames << "Time (s)";

        model->setColumnCount(1);
        model->setHorizontalHeaderLabels(columnNames);

        // determine a common starttime and delta time
        double ct_1 = 0.0;  // curves->at(0)->data()->sample(1).x();
        double cdt  = 0.0;  // ct_1 - curves->at(0)->data()->sample(0).x();

        for(int i = 0; i < curves->size(); i++)
        {
            // loops until first curve with signal data is found (-> dt)
            if(curves->at(i)->data()->size() > 0)
            {
                ct_1    = curves->at(0)->data()->sample(1).x();
                cdt     = ct_1 - curves->at(0)->data()->sample(0).x();
                break;
            }
        }

        if(ct_1 == 0 && cdt == 0)
        {
            labelNoCurveData->setVisible(true);
            tableView->setVisible(false);
            return;
        }

        // find latest signal time of all plot curves -> max time
        double max_t = 0;
        for(int i = 0; i < curves->size(); i++)
        {
            size_t noSamples = curves->at(i)->data()->size();
            if(noSamples > 0 && curves->at(i)->data()->sample(noSamples-1).x() > max_t)
            {
                max_t = curves->at(i)->data()->sample(noSamples-1).x();
            }
        }

        // find earliest signal time of all plot curves -> min time
        double min_t = max_t;
        for(int i = 0; i < curves->size(); i++)
        {
            size_t noSamples = curves->at(i)->data()->size();
            if(noSamples > 0 && curves->at(i)->data()->sample(0).x() < min_t)
            {
                min_t = curves->at(i)->data()->sample(0).x();
            }
        }

        int noPoints = round((max_t - min_t) / cdt) + 1;

        model->setRowCount(noPoints);

        for(int j = 0; j < noPoints; j++)
        {
            //model->setRowHeight(j,14);
            double t = (min_t + j*cdt) * 1E-9;
            model->setItem(j, 0, new QStandardItem(QString::number(t, 'g', _number_precision)));
        }

        for(int i = 0; i < curves->size(); i++)
        {
            if(curves->at(i)->data()->size() < 2)
                continue;

            // determine  starttime and delta time
            double t_start = curves->at(i)->data()->sample(0).x();
            double t_1 = curves->at(i)->data()->sample(1).x();
            double dt = t_1 - t_start;

            // create a signal from plotcurve
            Signal signal;
            signal.start_time = t_start;
            signal.dt = dt;
            signal.data.fill(0.0, curves->at(i)->dataSize());

            for(int j = 0; j < curves->at(i)->dataSize(); j++)
                signal.data[j] = curves->at(i)->sample(j).y();

            tempData.insert(curves->at(i)->title().text(), signal);
        }

        if(actionShowStdev->isChecked() && stdevCurves != nullptr)
        {
            for(int i = 0; i < stdevCurves->size(); i++)
            {
                if(stdevCurves->at(i)->data()->size() < 2)
                    continue;

                // determine  starttime and delta time
                double t_start = stdevCurves->at(i)->data()->sample(0).value;
                double t_1 = stdevCurves->at(i)->data()->sample(1).value;
                double dt = t_1 - t_start;

                // create a signal from plotcurve
                Signal signal;
                signal.start_time = t_start;
                signal.dt = dt;
                signal.data.fill(0.0, stdevCurves->at(i)->dataSize());

                for(int j = 0; j < stdevCurves->at(i)->dataSize(); j++)
                    signal.data[j] = (stdevCurves->at(i)->sample(j).interval.maxValue() - stdevCurves->at(i)->sample(j).interval.minValue()) / 2;

                tempData.insert(stdevCurves->at(i)->title().text(), signal);
            }
        }

        model->setColumnCount(tempData.size());
        QList<QString> keys = tempData.keys();
        for(int i = 0; i < keys.size(); i++)
        {
            columnNames << keys.at(i);
            Signal signal = tempData.value(keys.at(i));

            double t = min_t;
            for(int j = 0; j < noPoints; j++)
            {
                model->setItem(j, i+1, new QStandardItem(QString::number(signal.at(t), 'g', _number_precision)));
                t += cdt;
            }
        }
    }
    else
    {
        int column_counter = 0;
        int difference_counter = 0;
        double current_dt = 0.0f;
        double current_start_time = 0.0f;

        for(int i = 0; i < curves->size(); i++)
        {
            if(curves->at(i)->data()->size() < 2)
                continue;

            if(model->rowCount() < curves->at(i)->data()->size())
                model->setRowCount(curves->at(i)->data()->size());

            if(((curves->at(i)->data()->sample(1).x() - curves->at(i)->data()->sample(0).x()) != current_dt ) ||
                    curves->at(i)->data()->sample(0).x() != current_start_time)
            {
                current_dt = curves->at(i)->data()->sample(1).x() - curves->at(i)->data()->sample(0).x();
                current_start_time = curves->at(i)->data()->sample(0).x();
                difference_counter++;

                //build new time column
                columnNames << QString("Time (X%0) [s]").arg(difference_counter);

                for(int j = 0; j < curves->at(i)->data()->size(); j++)
                {
                    model->setItem(j, column_counter, new QStandardItem(QString::number(curves->at(i)->data()->sample(j).x() * 1E-9, 'g', _number_precision)));
                }

                column_counter++;
            }

            //add the signal data
            columnNames << QString("%0 (Y%1)").arg(curves->at(i)->title().text()).arg(difference_counter);

            for(int j = 0; j < curves->at(i)->data()->size(); j++)
            {
                model->setItem(j, column_counter, new QStandardItem(QString::number(curves->at(i)->data()->sample(j).y(), 'g', _number_precision)));
            }

            column_counter++;
        }

        if(actionShowStdev->isChecked() && stdevCurves != nullptr)
        {
            for(int i = 0; i < stdevCurves->size(); i++)
            {
                if(stdevCurves->at(i)->data()->size() < 2)
                    continue;

                if(model->rowCount() < stdevCurves->at(i)->data()->size())
                    model->setRowCount(stdevCurves->at(i)->data()->size());

                if(((stdevCurves->at(i)->data()->sample(1).value - stdevCurves->at(i)->data()->sample(0).value) != current_dt) ||
                        stdevCurves->at(i)->data()->sample(0).value != current_start_time)
                {
                    current_dt = stdevCurves->at(i)->data()->sample(1).value - stdevCurves->at(i)->data()->sample(0).value;
                    current_start_time = stdevCurves->at(i)->data()->sample(0).value;
                    difference_counter++;

                    //build new time column
                    columnNames << QString("Time (X%0) [s]").arg(difference_counter);

                    for(int j = 0; j < stdevCurves->at(i)->data()->size(); j++)
                    {
                        model->setItem(j, column_counter, new QStandardItem(QString::number(stdevCurves->at(i)->data()->sample(j).value, 'g', _number_precision)));
                    }

                    column_counter++;
                }

                //add the stdev signal data
                columnNames << QString("%0 (Y%1)").arg(stdevCurves->at(i)->title().text()).arg(difference_counter);

                for(int j = 0; j < stdevCurves->at(i)->data()->size(); j++)
                {
                    model->setItem(j, column_counter, new QStandardItem(QString::number((stdevCurves->at(i)->sample(j).interval.maxValue() - stdevCurves->at(i)->sample(j).interval.minValue()) / 2, 'g', _number_precision)));
                }

                column_counter++;
            }
        }
    }

    model->setHorizontalHeaderLabels(columnNames);

    QAbstractItemModel *oldModel = tableView->model();
    tableView->setModel(model);
    delete oldModel;

    for(int i = 0; i < model->rowCount(); i++)
        tableView->setRowHeight(i, 17);
}


/**
 * @brief DataTableWidget::onContextMenuEvent creates a menu on right click
 * @param pos position of context menu event
 */
void DataTableWidget::onContextMenuEvent(QPoint pos)
{
    QMenu menu(this);
    menu.addAction(actionCopyToClipboard);

    menu.exec( QCursor::pos());
}


/**
 * @brief DataTableWidget::onCopySelection copies the current selection
 * to clipboard.
 * @details col delimiter: "\t" decimal-delimiter "."
 * TODO: let user decide which delimiters to use
 */
void DataTableWidget::onCopySelection()
{
    QModelIndexList list = tableView->selectionModel()->selectedIndexes();

    int colMin = 1E10;
    int colMax = 0;
    int rowMin = 1E10;
    int rowMax = 0;

    for(int i = 0; i < list.size(); i++)
    {
        if(list.at(i).column() > colMax)
            colMax = list.at(i).column();
        if(list.at(i).column() < colMin)
            colMin = list.at(i).column();

        if(list.at(i).row() > rowMax)
            rowMax = list.at(i).row();
        if(list.at(i).row() < rowMin)
            rowMin = list.at(i).row();
    }

    QString data = "";

    if(!actionOmitHeader->isChecked())
    {
        for(int i = colMin; i <= colMax; i++)
        {
            data.append(dynamic_cast<QStandardItemModel*>(tableView->model())->horizontalHeaderItem(i)->text());
            data.append("\t");
        }
        data.append("\n");
    }

    for(int i = 0; i < list.size(); i++)
    {
        data.append(list.at(i).data().toString());
        if(list.at(i).column() == colMax)
            data.append("\n");
        else
            data.append("\t");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(data);
}


void DataTableWidget::onActionShowStdev()
{
    updateView();
}


void DataTableWidget::onActionOmitHeader(bool checked)
{
    Core::instance()->guiSettings->setValue("dataTable", "omitHeader", checked);
}


void DataTableWidget::onActionInterpolate()
{
    updateView();
}
