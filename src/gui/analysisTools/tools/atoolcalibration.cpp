#include "atoolcalibration.h"

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_symbol.h>

#include "../../general/LIISimException.h"

#include "../../signal/mrun.h"
#include "../../signal/signalmanager.h"
#include "../../core.h"
#include <QHeaderView>
#include <QClipboard>
#include <QShortcut>
#include <QApplication>

#include "../../utils/customQwtPlot/baseplotcurve.h"

#include "../../utils/numberlineedit.h"
#include "../../utils/liisettingscombobox.h"

#include "../../gui/utils/plotanalysistool.h"

AToolCalibration::AToolCalibration(QWidget *parent) : SignalPlotTool(parent)
{
    m_title = "Gain Calibration";
    m_iconLocation = Core::rootDir + "resources/icons/relative_gain.png";

    xStart  = 0.0;
    xEnd    = 0.0;

    plotter->addPlotAnalyzer("calibration","select data range for calibration",true, false);

    connect(plotter,SIGNAL(rangeSelected(double,double)),this,SLOT(onRangeSelected(double,double)));

    // keep track on MRun selection changed from SignalPlotTool
    connect(this,SIGNAL(mRunSelectionUpdated()),this,SLOT(onMRunSelectionUpdated()));

    connect(Core::instance()->guiSettings,SIGNAL(settingsChanged()),SLOT(onGuiSettingsChanged()));


    // add right vertically splitted box
    QSplitter* verticalSplitterRight = new QSplitter(Qt::Vertical);
    verticalSplitterRight->setContentsMargins(5,5,2,0);
    verticalSplitterRight->setHandleWidth(20);


    /******************
     *  TOP RIGHT BOX
     */

    // add visualization plot
    calPlot  = new BasePlotWidgetQwt;
    calPlot->setXAxisNonTimeType(true);
    calPlot->setZoomMode(BasePlotWidgetQwt::PLOT_PAN);
    calPlot->setDataTableToolName(m_title);
    calPlot->setPlotTitle("Calibration Visualization");
    calPlot->setPlotLabelText("Gain calibration:\n(1) Select a measurement run\n(2) Use the \"calibration\" option from the plotter to select a range for calibration");
    calPlot->plotTextLabelVisible(true);

    verticalSplitterRight->addWidget(calPlot);

    connect(calPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));

    /******************
     *  BOTTOM RIGHT BOX
     */

    // split bottom right box into two
    QSplitter *bottomRightSplitter = new QSplitter(Qt::Horizontal);


    /******************
     *  SETTINGS BOX
     */

    QGridLayout *settingsLayout = new QGridLayout;
    settingsLayout->setMargin(0);

    QWidget *settingsLayoutDummy = new QWidget();
    settingsLayoutDummy->setLayout(settingsLayout);


    // buttons (top)
    QPushButton * clearPlotButton = new QPushButton("clear plot");
    clearPlotButton->setMaximumWidth(60);

    connect(clearPlotButton,
            SIGNAL(released()),
            SLOT(onClearPlotButtonReleased()));

    QPushButton * addPlotButton = new QPushButton("add plot");
    addPlotButton->setMaximumWidth(60);

    connect(addPlotButton,
            SIGNAL(released()),
            SLOT(onAddPlotButtonReleased()));



    QHBoxLayout *infoAndButtonLayout = new QHBoxLayout;

    infoAndButtonLayout->addWidget(clearPlotButton);
    infoAndButtonLayout->addWidget(addPlotButton);    
    infoAndButtonLayout->addStretch(-1);
    settingsLayout->addLayout(infoAndButtonLayout,0,0);


    /******************
     *  SETTINGS TABLE
     */
    calSettingsTableWidget = new QTableWidget();

    int rh = 20; // rowheight

    calSettingsTableWidget->setRowCount(8);
    calSettingsTableWidget->setColumnCount(2);
    calSettingsTableWidget->setColumnWidth(0,100);
    calSettingsTableWidget->setColumnWidth(1,220);
    calSettingsTableWidget->setShowGrid(false);

    QStringList hHeaderList;
    hHeaderList << "Settings" << "";
    calSettingsTableWidget->setHorizontalHeaderLabels(hHeaderList);
    calSettingsTableWidget->verticalHeader()->setVisible(false);



    int rowID = 0;

    // CHECKBOX: "add manually"

    calSettingsTableWidget->setRowHeight(rowID,rh);
    calSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("add manually"));
    checkBoxManAdd = new QCheckBox("Manually add calibration curves");
    calSettingsTableWidget->setCellWidget(rowID,1,checkBoxManAdd);

    connect(checkBoxManAdd,
            SIGNAL(stateChanged(int)),
            SLOT(onChManAddStateChanged(int)));

    rowID++;


    // CHECKBOX: "show fit"

    calSettingsTableWidget->setRowHeight(rowID,rh);
    calSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("show fit"));
    checkBoxShowFit = new QCheckBox("Show fitting curves in calibration plot");
    checkBoxShowFit->setChecked(true);
    calSettingsTableWidget->setCellWidget(rowID,1,checkBoxShowFit);
    rowID++;


    // CHECKBOX: "show LIISettings calibration"

    calSettingsTableWidget->setRowHeight(rowID,rh);
    calSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("show current calibration"));
    checkBoxShowCurrentCal = new QCheckBox("Show current calibration curve");
    checkBoxShowCurrentCal->setChecked(true);
    calSettingsTableWidget->setCellWidget(rowID,1,checkBoxShowCurrentCal);
    rowID++;


    // Combobox: "select LIISettings"

    calSettingsTableWidget->setRowHeight(rowID,rh);
    calSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("current LIISettings"));
    cbliisettings = new LIISettingsComboBox();
    cbliisettings->setCurrentText(Core::instance()->guiSettings->value("atool_calibration","liisettings","").toString());
    calSettingsTableWidget->setCellWidget(rowID,1,cbliisettings);

    connect(cbliisettings,SIGNAL(currentIndexChanged(int)),SLOT(liisettingschanged(int)));

    rowID++;

    // CHECKBOX: "normalize"

    calSettingsTableWidget->setRowHeight(rowID,rh);
    calSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("normalize"));
    checkBoxNormalize = new QCheckBox("");
    checkBoxNormalize->setChecked(true);
    calSettingsTableWidget->setCellWidget(rowID,1,checkBoxNormalize);
    rowID++;


    // INPUT: normalize reference

    calSettingsTableWidget->setRowHeight(rowID,rh);
    calSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("norm reference"));
    input_norm_ref = new NumberLineEdit(NumberLineEdit::DOUBLE);

    // set default value
    input_norm_ref->setValue(0.4);
    calSettingsTableWidget->setCellWidget(rowID,1,input_norm_ref);
    rowID++;


    settingsLayout->addWidget(calSettingsTableWidget);

    bottomRightSplitter->addWidget(settingsLayoutDummy);

   /******************
    *  RESULT TABLE
    */

    // shortcut
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(copyToClipboard()));

    calResultTableWidget = new QTableWidget();
    bottomRightSplitter->addWidget(calResultTableWidget);

    int w1 = 290;
    int w2 = bottomRightSplitter->width()-w1;

    QList<int> wlist;
    wlist << w1 << w2;
    bottomRightSplitter->setSizes(wlist);


    verticalSplitterRight->addWidget(bottomRightSplitter);


    // change main horizontal splitter size
    horizontalSplitter->addWidget(verticalSplitterRight);

    w1 = 300;
    w2 = horizontalSplitter->width()-w1;

    wlist.clear();
    wlist << w1 << w2;
    horizontalSplitter->setSizes(wlist);

    plotterToolbar->addActions(toolbarActions());
}


void AToolCalibration::onToolActivation()
{
    SignalPlotTool::onToolActivation();

    processDataInterval();
}


void AToolCalibration::onMRunSelectionUpdated()
{
    // don't update automatically if "manually add" checkbox is checked
    if(!checkBoxManAdd->isChecked())
        processDataInterval();
}


void AToolCalibration::processDataInterval()
{
    //qDebug() << "AToolCalibration: processDataInterval: " << xStart << " " <<xEnd;

    if(xEnd <= xStart)
        return;

    calPlot->plotTextLabelVisible(false);

    // delete gain calibration text
    calPlot->clearText();

    QVector<double> xData;
    QVector<double> yData;
    QVector<double> sData; // standard deviation
    double x;
    double y;
    double stdev;

    QTableWidgetItem * item;

    calResultTableWidget->setRowCount(selectedRuns().size());
    calResultTableWidget->setColumnCount(12);
    int twidth_g = 60;
    int twidth = 50;
    calResultTableWidget->setColumnWidth(0,twidth_g);
    calResultTableWidget->setColumnWidth(1,twidth);
    calResultTableWidget->setColumnWidth(2,twidth);
    calResultTableWidget->setColumnWidth(3,twidth_g);
    calResultTableWidget->setColumnWidth(4,twidth);
    calResultTableWidget->setColumnWidth(5,twidth);
    calResultTableWidget->setColumnWidth(6,twidth_g);
    calResultTableWidget->setColumnWidth(7,twidth);
    calResultTableWidget->setColumnWidth(8,twidth);
    calResultTableWidget->setColumnWidth(9,twidth_g);
    calResultTableWidget->setColumnWidth(10,twidth);
    calResultTableWidget->setColumnWidth(11,twidth);

    QStringList hHeaderList;

    hHeaderList << "CH 1 (G)" << "(avg)" << "stdev"
                << "CH 2 (G)" << "(avg)" << "stdev"
                << "CH 3 (G)" << "(avg)" << "stdev"
                << "CH 4 (G)" << "(avg)" << "stdev";

    calResultTableWidget->setHorizontalHeaderLabels(hHeaderList);

    // process data

    // QMap[CHANNEL, QList[ [VOLTAGE, [MEAN, STDEV]]]
    QMap<int, QList<QPair<double,QPair<double,double>>>> sortedLists;
    int chID;
    int max_num_channels = 0;

    MSG_ONCE("AToolCalibrationOnlyFirstMPoint", 1,
             "AToolCalibration: only first MPoint (idx=0) is used for calculation",
             WARNING);

    // first collect data from selected mruns and write them into sortedLists
    for(int i = 0; i < selectedRuns().size(); i++)
    {
        MRun *run = selectedRuns().at(i);
        MPoint *mp = run->getPost(0); // first mpoint idx=0

        if(max_num_channels < run->getNoChannels(Signal::RAW))
            max_num_channels = run->getNoChannels(Signal::RAW);

        for (chID = 1; chID <= run->getNoChannels(Signal::RAW); chID++)
        {
            // skip channel if not selected
            if(!selectedChannelIds().contains(chID))
                continue;

            Signal signal = mp->getSignal(chID, selectedSignalType());

            QPair<double,double> mean_stdev = signal.calcRangeAverageStdev(xStart, xEnd);

            QPair<double,QPair<double,double>> pair;
            pair.first  = run->pmtGainVoltage(chID); // x
            pair.second = mean_stdev;                // y
            sortedLists[chID].append(pair);
        }
    }

    // get max value for each curve for normalization
    QMap<int,double> max_value;
    QMap<int,double> ref_value;

    if(checkBoxNormalize->isChecked())
    {
        for (chID = 1; chID <= max_num_channels; chID++)
        {
            max_value[chID] = 0.0;
            ref_value[chID] = 0.0;

            for(int i = 0; i < sortedLists[chID].size(); i++)
            {

             // save reference value at user-defined voltage
             if(std::abs(input_norm_ref->getValue() - sortedLists[chID].at(i).first) < 0.001)
                ref_value[chID] = sortedLists[chID].at(i).second.first;

             // get max value
             if(sortedLists[chID].at(i).second.first > max_value[chID])
                 max_value[chID] = sortedLists[chID].at(i).second.first;

             //qDebug() << "Ref voltage: " << input_norm_ref->getValue() << " / " << sortedLists[chID].at(i).first << "-" << sortedLists[chID].at(i).second << "Ref value: " << ref_value[chID];
            }

            // if reference voltage cannot be found normalize to max value
            if(ref_value[chID] == 0.0)
                ref_value[chID] = max_value[chID];
        }
    }

    // process all sortedLists elements  by channelID
    for (chID = 1; chID <= max_num_channels; chID++)
    {
        // skip channel if not selected
        if(!selectedChannelIds().contains(chID))
            continue;

        // set max row count
        if(calResultTableWidget->rowCount() != sortedLists[chID].size())
                calResultTableWidget->setRowCount(sortedLists[chID].size());

        xData.clear();
        yData.clear();
        sData.clear();

        BasePlotCurve *curve = new BasePlotCurve(QString("Calibration CH %0").arg(chID));

        // sort list by x values
        std::sort(sortedLists[chID].begin(), sortedLists[chID].end());

        int rh = 18; // rowheight

        // create data vectors from sorted list and show data in table
        for(int i = 0; i < sortedLists[chID].size(); i++)
        {
            x = sortedLists[chID].at(i).first;

            if(checkBoxNormalize->isChecked())
                y = sortedLists[chID].at(i).second.first / ref_value[chID];
            else
                y = sortedLists[chID].at(i).second.first;


            stdev = sortedLists[chID].at(i).second.second;

            xData.push_back(x);
            yData.push_back(y);
            sData.push_back(stdev);

            QString val;

            // 1) gain voltage
            val.sprintf("%g", x);
            item = new QTableWidgetItem(val);

            if(x == input_norm_ref->getValue())
                item->setBackgroundColor(Qt::cyan);

            //qDebug() << "gain voltage x: " << x;
            calResultTableWidget->setRowHeight(i,rh);
            calResultTableWidget->setItem(i,(chID-1)*3,item);

            // 2) average intensity over selected range
            val.sprintf("%g", y);
            item = new QTableWidgetItem(val);
            calResultTableWidget->setItem(i,(chID-1)*3+1,item);

            // 3) standard deviation over selected range
            val.sprintf("%g", stdev);
            item = new QTableWidgetItem(val);
            calResultTableWidget->setItem(i,(chID-1)*3+2,item);
        }

        // show data in plot

        // never change appearance
        curve->setFixedStyle(true);

        // curve style: no line
        curve->setStyle(BasePlotCurve::NoCurve);

        // data point symbol: cross
        QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
        s->setSize(10);
        s->setColor(curve->pen().color());
        curve->setSymbol(s);

        curve->setSamples(xData,yData);

        calPlot->registerCurve(curve);

        if(checkBoxShowFit->isChecked())
        {
            //calculate exponential fit
            //http://www.codewithc.com/c-program-for-linear-exponential-curve-fitting/
            //http://mathworld.wolfram.com/LeastSquaresFitting.html

            int N = 0;
            double a,b, sumx=0.0,sumy=0.0,sumxy=0.0,sumx2=0.0;

            a = 0.0;
            b = 0.0;

            for(int j = 0; j < xData.size(); j++)
            {
                 double x = xData.at(j);
                 double y = yData.at(j);

                 sumx    = sumx  + log(x);
                 sumx2   = sumx2 + log(x) * log(x);
                 sumy    = sumy  + log(y);
                 sumxy   = sumxy + log(x) * log(y);

                 N++;
            }

            // calculate parameters Y = aX+b
            a=((N*sumxy - sumx*sumy) / (N*sumx2 - sumx*sumx));
            b=((sumx2*sumy - sumx*sumxy) / (N*sumx2 - sumx*sumx));


            QVector<double> xFit;
            QVector<double> yFit;

            xFit = xData;


            double x_ref = exp(-b/a);


    //        for(double j= 0; j < 120; j++)
    //        {
    //            double val = 0.3+j*0.005;
    //            xFit.append(val);
    //        }

            for(int k=0; k < xFit.size(); k++)
            {
                double xval = log(xFit.at(k));
                yFit.append(exp(a*xval+b));

//                // normalize fit to reference voltage
//                if(std::abs(input_norm_ref->getValue() - xFit.at(k)) < 0.001)
//                   ref_value[chID] = yFit.last();
            }

//            for(int k=0; k < xFit.size(); k++)
//            {
//                yFit[k] = yFit[k] / ref_value[chID];
//            }



            BasePlotCurve* curve_fit = new BasePlotCurve(QString("CalFitTool CH %0").arg(chID));
            curve_fit->setSamples(xFit,yFit);

            //Qt::DashLine
            //Qt::DotLine
            //Qt::SolidLine

            curve_fit->setPen(* new QPen(Qt::blue,1,Qt::DashLine));

            calPlot->registerCurve(curve_fit);

            // add equation to plot
            QString equationText;

            if(checkBoxNormalize->isChecked())
                equationText = QString("CH %1: y = exp(%2 *ln(x) + %3) (N:%4 N:%5)").arg(chID).arg(a).arg(b).arg(input_norm_ref->getValue()).arg(x_ref);
            else
                equationText = QString("CH %1: y = exp(%2 *ln(x) + %3)").arg(chID).arg(a).arg(b);


            calPlot->appendText(equationText);

            // show current calibration curve
            if(checkBoxShowCurrentCal->isChecked())
            {
                BasePlotCurve *curve_cur_cal = getCurrentCalibrationCurve(chID);

                calPlot->registerCurve(curve_cur_cal);
            }
        }
    }
}


BasePlotCurve* AToolCalibration::getCurrentCalibrationCurve(int chID)
{
    LIISettings liisettings = cbliisettings->currentLIISettings();

    if(chID > liisettings.channels.size())
    {
        MSG_WARN(QString("AToolCalibration: Channel %0 not available for current LIISettings (%1)").arg(chID).arg(liisettings.name));

        BasePlotCurve* curve_fit = new BasePlotCurve(QString("Not available CH %0").arg(chID));
        return  curve_fit;
    }

    // create xData vector
    QVector<double> xData;

    for(int k=0; k <= 120; k++)
        xData.append(0.3 + k*0.005);


    double a = liisettings.channels.at(chID-1).pmt_gain_formula_A;
    double b = liisettings.channels.at(chID-1).pmt_gain_formula_B;
    double ref_voltage = liisettings.channels.at(chID-1).pmt_gain;

    // create curve
    BasePlotCurve* curve_fit = new BasePlotCurve(QString("Current calibration CH %0").arg(chID));
    curve_fit->setPen(* new QPen(Qt::green,0.5,Qt::DashLine));

    QVector<double> yFit;

    for(int k=0; k < xData.size(); k++)
    {
        //double xval = log(xData.at(k));
        //yFit.append(exp(a*xval+b));
        double xval = log(xData.at(k) / ref_voltage);
        yFit.append(exp(a*xval));
    }

//    // normalize
//    if(checkBoxNormalize->isChecked())
//    {
//        double max = *std::max_element(yFit.begin(), yFit.end());

//        //int ref_idx = xData.indexOf(input_norm_ref->getValue());
//        int ref_idx = xData.indexOf(ref_voltage);

//        double y_ref = yFit.at(ref_idx);

//        for(int k=0; k < xData.size(); k++)
//        {
//            if(ref_idx == -1)
//                yFit[k] /= max;
//            else
//                yFit[k] /= y_ref;
//        }
//    }

    curve_fit->setSamples(xData,yFit);
    return curve_fit;
}


void AToolCalibration::onRangeSelected(double xstart, double xend)
{

    xStart = xstart * 1E-9;
    xEnd = xend * 1E-9;

    processDataInterval();
}


void AToolCalibration::onClearPlotButtonReleased()
{
    calPlot->detachAllCurves();
    calPlot->plotTextLabelVisible(true);
}


void AToolCalibration::onAddPlotButtonReleased()
{
    processDataInterval();
}


void AToolCalibration::copyToClipboard()
{

    QList<QTableWidgetItem*> items =  calResultTableWidget->selectedItems();

    QString data;


    int minr = 1E10;
    int maxr = 0;
    int minc = 1E10;
    int maxc = 0;


    // first, get row/column index range
    for(int i = 0; i < items.size(); i++)
    {
        int r =  items.at(i)->row();
        int c =  items.at(i)->column();

        if( r > maxr ) maxr = r;
        if( c > maxc ) maxc = c;
        if( r < minr ) minr = r;
        if( c < minc ) minc = c;
    }

    //qDebug() << "onCopySelection " << minr << maxr << minc << maxc;

        // print data line per line to string
    for(int r = minr; r <= maxr; r++)
    {
        for(int c = minc; c <= maxc; c++)
        {
            data.append(calResultTableWidget->item(r,c)->text());

            // separate values in cols
            if(c<maxc)
                data.append("\t");
        }

        if(r < maxr)
            data.append("\n");
    }

    // qDebug() << "copied range: " << minr << minc << " to " << maxr << maxc;

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(data);
}


void AToolCalibration::onChManAddStateChanged(int state)
{
    // disable
}


void AToolCalibration::onGuiSettingsChanged()
{
    // re-select last liisettings selection
    cbliisettings->setCurrentText(Core::instance()->guiSettings->value("atool_calibration","liisettings","").toString());

    if(Core::instance()->guiSettings->hasEntry("atool_calibration", "plottype"))
    {
        switch(Core::instance()->guiSettings->value("atool_calibration", "plottype", 0).toUInt())
        {
        case 0: calPlot->setPlotType(BasePlotWidgetQwt::LINE_CROSSES); break;
        case 1: calPlot->setPlotType(BasePlotWidgetQwt::LINE); break;
        case 2: calPlot->setPlotType(BasePlotWidgetQwt::DOTS_SMALL); break;
        case 3: calPlot->setPlotType(BasePlotWidgetQwt::DOTS_MEDIUM); break;
        case 4: calPlot->setPlotType(BasePlotWidgetQwt::DOTS_LARGE); break;
        }
    }
}


void AToolCalibration::liisettingschanged(int index)
{
    // save selection to guisettings to
    Core::instance()->guiSettings->setValue("atool_calibration","liisettings",cbliisettings->currentText());
}


void AToolCalibration::onPlotTypeChanged(BasePlotWidgetQwt::PlotType type)
{
    switch(type)
    {
    case BasePlotWidgetQwt::LINE_CROSSES: Core::instance()->guiSettings->setValue("atool_calibration", "plottype", 0); break;
    case BasePlotWidgetQwt::LINE:         Core::instance()->guiSettings->setValue("atool_calibration", "plottype", 1); break;
    case BasePlotWidgetQwt::DOTS_SMALL:   Core::instance()->guiSettings->setValue("atool_calibration", "plottype", 2); break;
    case BasePlotWidgetQwt::DOTS_MEDIUM:  Core::instance()->guiSettings->setValue("atool_calibration", "plottype", 3); break;
    case BasePlotWidgetQwt::DOTS_LARGE:   Core::instance()->guiSettings->setValue("atool_calibration", "plottype", 4); break;
    }
}
