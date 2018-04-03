#include "atoolabscalibration.h"

#include "../../core.h"

#include "../../utils/customQwtPlot/baseplotcurve.h"
#include <qwt_symbol.h>


#include <QHeaderView>
#include <QClipboard>
#include <QShortcut>
#include <QApplication>


AToolAbsCalibration::AToolAbsCalibration(QWidget *parent) : SignalPlotTool(parent)
{
    m_title = "Absolute calibration";
    m_iconLocation = Core::rootDir + "resources/icons/lightbulb.png";

    xStart  = 0.0;
    xEnd    = 0.0;

    plotter->addPlotAnalyzer("calibration","select data range for calibration",true, false);

    connect(plotter,SIGNAL(rangeSelected(double,double)),this,SLOT(onRangeSelected(double,double)));


    // keep track on MRun selection changed from SignalPlotTool
    connect(this,SIGNAL(mRunSelectionUpdated()),this,SLOT(onMRunSelectionUpdated()));


   connect(Core::instance()->guiSettings,SIGNAL(settingsChanged()),SLOT(onGuiSettingsChanged()));


   /** TODO:
    *
    * Program crashes if Signal::SType is chaned to TEMPERATURE
    * Implement AToolBase method to define allowed STypes
    *
    *
    */




    /******************
     *  TOP LEFT BOX
     */

    // add left vertically splitted box (two plots)
    QSplitter* verticalSplitterLeft = new QSplitter(Qt::Vertical);
    verticalSplitterLeft->setContentsMargins(5,5,2,0);
    verticalSplitterLeft->setHandleWidth(20);

    // remove plotter from horizontal splitter
    // and add it to vertical splitter (left box)

    plotterWidget->setParent(0);
    verticalSplitterLeft->addWidget(plotterWidget);


    /******************
     *  BOTTOM LEFT BOX
     */

    calResultTableWidget = new QTableWidget();
    calResultTableWidget->setColumnCount(7);
    calResultTableWidget->setColumnWidth(0,60);
    calResultTableWidget->setColumnWidth(1,80);
    calResultTableWidget->setColumnWidth(2,100);
    calResultTableWidget->setColumnWidth(3,120);
    calResultTableWidget->setColumnWidth(4,120);
    calResultTableWidget->setColumnWidth(5,120);
    calResultTableWidget->setColumnWidth(6,120);

    QStringList hHeaderListCal;
    hHeaderListCal << "Channel" << "Average" << "Integral" << "Integral/Bandwidth" << "Center wavelength" << "Calibration (integral)" << "Calibration (center)";
    calResultTableWidget->setHorizontalHeaderLabels(hHeaderListCal);

    // shortcut
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(copyToClipboard()));


    // add widget to general layout
    verticalSplitterLeft->addWidget(calResultTableWidget);


    /******************
     *  RIGHT BOX
     */

    // add right vertically splitted box
    QSplitter* verticalSplitterRight = new QSplitter(Qt::Vertical);
    verticalSplitterRight->setContentsMargins(5,5,2,0);
    verticalSplitterRight->setHandleWidth(20);


    /******************
     *  TOP RIGHT BOX
     */

    // add visualization plot
    calPlot  = new BasePlotWidgetQwt;
    calPlot->setZoomMode(BasePlotWidgetQwt::PLOT_PAN);
    calPlot->setPlotAxisTitles("Wavelength / nm", "Intensity / -");
    calPlot->setMaxLegendColumns(1);
    calPlot->setDataTableToolName(m_title);
    calPlot->setPlotTitle("Calibration Visualization");
    calPlot->setPlotLabelText("Sensitivity calibration:\n(1) Select a measurement run\n(2) Use the \"calibration\" option from the plotter to select a range for calibration");
    calPlot->plotTextLabelVisible(true);

    verticalSplitterRight->addWidget(calPlot);

    //connect(calPlot, SIGNAL(plotTypeChanged(BasePlotWidgetQwt::PlotType)), SLOT(onPlotTypeChanged(BasePlotWidgetQwt::PlotType)));

    /******************
     *  BOTTOM RIGHT BOX
     */

    // split bottom right box into two
    QSplitter *bottomRightSplitter = new QSplitter(Qt::Vertical);

    QGridLayout *settingsRightLayout = new QGridLayout;
    settingsRightLayout->setMargin(0);

    QWidget *settingsRightLayoutDummy = new QWidget();
    settingsRightLayoutDummy->setLayout(settingsRightLayout);


    // buttons (top)
    QPushButton * clearPlotButton = new QPushButton("clear plot");
    clearPlotButton->setMaximumWidth(60);

    connect(clearPlotButton,
            SIGNAL(released()),
            SLOT(onClearPlotButtonReleased()));



    // add buttons to layout below plot
    QHBoxLayout *infoAndButtonLayout = new QHBoxLayout;

    infoAndButtonLayout->addWidget(clearPlotButton);
    infoAndButtonLayout->addStretch(-1);

    settingsRightLayout->addLayout(infoAndButtonLayout,0,0);

    /******************
     *  BOTTOM RIGHT SETTINGS
     */


    calRightSettingsTableWidget = new QTableWidget();

    int rh = 20; // rowheight

    calRightSettingsTableWidget->setRowCount(2);
    calRightSettingsTableWidget->setColumnCount(2);
    calRightSettingsTableWidget->setColumnWidth(0,200);
    calRightSettingsTableWidget->setColumnWidth(1,220);
    calRightSettingsTableWidget->setShowGrid(false);

    calRightSettingsTableWidget->horizontalHeader()->setVisible(false);
    calRightSettingsTableWidget->verticalHeader()->setVisible(false);
    calRightSettingsTableWidget->setStyleSheet("QTableWidget {background-color: transparent;}");
    calRightSettingsTableWidget->setFrameStyle(QFrame::NoFrame);


    int rowID = 0;

    // COMBOBOX: reference spectrum


    calRightSettingsTableWidget->setRowHeight(rowID,rh);
    calRightSettingsTableWidget->setItem(rowID,0, new QTableWidgetItem("Select reference spectrum: "));

    cbSpectra = new SpectraComboBox;
    cbSpectra->setCurrentText(Core::instance()->guiSettings->value("atool_abs_calibration","ref_spectrum","").toString());
    connect(cbSpectra,SIGNAL(currentIndexChanged(int)),SLOT(refSpectrumChanged(int)));

    calRightSettingsTableWidget->setCellWidget(rowID,1,cbSpectra);

    rowID++;


    // layout

    settingsRightLayout->addWidget(calRightSettingsTableWidget);

    bottomRightSplitter->addWidget(settingsRightLayoutDummy);



    /******************
     *  SPECTRA BOX
     */

    calSpectraTableWidget = new QTableWidget();

    rh = 20; // rowheight

    calSpectraTableWidget->setRowCount(8);
    calSpectraTableWidget->setColumnCount(4);
    calSpectraTableWidget->setColumnWidth(0,100);
    calSpectraTableWidget->setColumnWidth(1,100);
    calSpectraTableWidget->setColumnWidth(2,100);
    calSpectraTableWidget->setColumnWidth(3,100);
    calSpectraTableWidget->setShowGrid(false);

    QStringList hHeaderList;
    hHeaderList << "Reference" << "Dichroic" << "ND" << "other";
    calSpectraTableWidget->setHorizontalHeaderLabels(hHeaderList);
    calSpectraTableWidget->verticalHeader()->setVisible(false);


    // TODO: insert automatic listing of database entries with checkbox for overlay



    bottomRightSplitter->addWidget(calSpectraTableWidget);


    // change bottom right splitter height

    int w1 = 100; // height of buttons and settings (right)
    int w2 = bottomRightSplitter->height()-w1;

    QList<int> wlist;
    wlist << w1 << w2;
    bottomRightSplitter->setSizes(wlist);

     // add widget to general layout

    verticalSplitterRight->addWidget(bottomRightSplitter);

    // define vertical heights (general)
    w1 = 250; // height of calPlot
    w2 = verticalSplitterRight->height()-w1;

    wlist.clear();
    wlist << w1 << w2;

    verticalSplitterRight->setSizes(wlist);

    // change main horizontal splitter size

    horizontalSplitter->addWidget(verticalSplitterLeft);
    horizontalSplitter->addWidget(verticalSplitterRight);

    w1 = 300;
    w2 = horizontalSplitter->width()-w1;

    wlist.clear();
    wlist << w1 << w2;
    horizontalSplitter->setSizes(wlist);

    plotterToolbar->addActions(toolbarActions());
}


void AToolAbsCalibration::onMRunSelectionUpdated()
{
    // don't update automatically if "manually add" checkbox is checked
    //if(!checkBoxManAdd->isChecked())
        processDataInterval();
}


void AToolAbsCalibration::processDataInterval()
{
    //qDebug() << "AToolAbsCalibration: processDataInterval: " << xStart << " " <<xEnd;

    if(xEnd <= xStart)
        return;

    // delete gain calibration text
    calPlot->clearText();
    calPlot->plotTextLabelVisible(false);


    // only for ABS and RAW signals (see TODO)
    Signal::SType stype = selectedSignalType();

    if(stype == Signal::TEMPERATURE)
        stype = Signal::RAW;

    // get current run/mpoint data
    MRun *run = currentMRun;
    MPoint *mp = currentMPoint;

    int chID;
    int numCh = run->getNoChannels(stype);

    QVector<double> xData, yData;
    QVector<double> xDataC, yDataC;
    QVector<double> xDataI, yDataI;

    double avg;
    double intensity;

    // temporary channel properties
    int wavelength;
    int bandwidth;
    int b2; // rounded half bandwidth


    // set boundaries for calResultTableWidget
    QTableWidgetItem * item;
    calResultTableWidget->setRowCount(numCh);
    QString val;
    int rh = 18; // rowheight

    // plot reference spectrum

    BasePlotCurve *ref_spec_curve = new BasePlotCurve(QString("Reference spectrum"));
    ref_spec_curve->setSamples(refSpectrum.xData, refSpectrum.yData);

    calPlot->registerCurve(ref_spec_curve);

    // visualize bandpass bandwidth

    for (chID = 1; chID <= numCh; chID++)
    {

        xDataC.clear();
        yDataC.clear();

        wavelength = currentMRun->liiSettings().channels.at(chID-1).wavelength;
        bandwidth = currentMRun->liiSettings().channels.at(chID-1).bandwidth;
        b2 = currentMRun->liiSettings().channels[chID-1].getHalfBandwidth();

        for(int k = - b2 ; k <= b2; k++)
        {
            intensity = refSpectrum.getYofX(wavelength+k);

            xDataC.append(wavelength + k);
            yDataC.append(intensity);
        }

        BasePlotCurve *bcurve = new BasePlotCurve(QString("%0 nm(%1 nm)")
                                                  .arg(wavelength)
                                                  .arg(bandwidth)
                                                  );
        bcurve->setSamples(xDataC,yDataC);
        bcurve->setBaseline(0.0);
        bcurve->setBrush(QBrush(Qt::blue,Qt::Dense6Pattern));

        calPlot->registerCurve(bcurve);

        double integral = refSpectrum.integrate(wavelength-b2, wavelength+b2);

        double centerValue = refSpectrum.getYofX(wavelength);
        double calValue = integral / (b2 * 2);

        xDataI.append(wavelength);
        yDataI.append(calValue);

        // get average value from signal
        Signal signal = mp->getSignal(chID, stype);
        avg = signal.calcRangeAverage(xStart, xEnd);

        xData.append(wavelength);
        yData.append(calValue/avg);

        // show data in calResultTableWidget
        calResultTableWidget->setRowHeight(chID-1, rh);

        val.sprintf("%i",wavelength);
        calResultTableWidget->setItem(chID-1, 0,new QTableWidgetItem(val));

        val.sprintf("%g",avg);
        calResultTableWidget->setItem(chID-1, 1,new QTableWidgetItem(val));

        val.sprintf("%g",integral);
        calResultTableWidget->setItem(chID-1, 2,new QTableWidgetItem(val));

        val.sprintf("%g",calValue);
        calResultTableWidget->setItem(chID-1, 3,new QTableWidgetItem(val));

        val.sprintf("%g",centerValue);
        calResultTableWidget->setItem(chID-1, 4,new QTableWidgetItem(val));

        val.sprintf("%g",calValue/avg);
        calResultTableWidget->setItem(chID-1, 5,new QTableWidgetItem(val));

        val.sprintf("%g",centerValue/avg);
        calResultTableWidget->setItem(chID-1, 6,new QTableWidgetItem(val));
    }



    // plot symbols for intensity of integral
    BasePlotCurve *bcurve_int = new BasePlotCurve(QString("%0 - Integral approximation")
                                              .arg(QChar(0x222B))
                                              );

    // never change appearance
    bcurve_int->setFixedStyle(true);

    // curve style: no line
    bcurve_int->setPen(QPen(Qt::red, 1, Qt::DashLine));
    bcurve_int->setStyle(BasePlotCurve::NoCurve);

    // data point symbol: diamond
    QwtSymbol* s2 = new QwtSymbol(QwtSymbol::Diamond);
    s2->setSize(5);
    s2->setColor(Qt::red);
    bcurve_int->setSymbol(s2);

    // legend attributes
    bcurve_int->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
    bcurve_int->setLegendIconSize(QSize(8,8));

    bcurve_int->setSamples(xDataI,yDataI);

    calPlot->registerCurve(bcurve_int);



    // process and plot data

    BasePlotCurve *curve = new BasePlotCurve(QString("Data (%0 to %1 ns)").arg(xStart*1E9).arg(xEnd*1E9));

    // never change appearance
    curve->setFixedStyle(true);

    // curve style: no line
    curve->setStyle(BasePlotCurve::NoCurve);

    // data point symbol: cross
    QwtSymbol* s = new QwtSymbol(QwtSymbol::Cross);
    s->setSize(10);
    s->setColor(curve->pen().color());
    curve->setSymbol(s);

    curve->setSamples(xData, yData);


    // front level display
    //calPlot->registerCurve(curve);

}


void AToolAbsCalibration::handleSelectedRunsChanged(QList<MRun *> &runs)
{
    if(runs.size() <= 0)
    {
        updateMRunSelection();

        // emit signal for child classes
        emit mRunSelectionUpdated();
        return;
    }

    // TODO: get signal idx from treeview

    qDebug() << "AToolAbsCalibration: only first MPoint is used for calculation";

    MRun *run = selectedRuns().last();
    MPoint *mp = run->getPost(0); // first mpoint idx=0

    // save for later use
    currentMRun = run;
    currentMPoint = mp;

    updateMRunSelection();

    // emit signal for child classes
    emit mRunSelectionUpdated();
}


void AToolAbsCalibration::onToolActivation()
{
    SignalPlotTool::onToolActivation();

    handleSelectedRunsChanged(selectedRuns());
}


void AToolAbsCalibration::onRangeSelected(double xstart, double xend)
{
    xStart = xstart * 1E-9;
    xEnd = xend * 1E-9;

    qDebug() << "Range changed: " << xStart << " " << xEnd;

    processDataInterval();
}


void AToolAbsCalibration::refSpectrumChanged(int index)
{
    Core::instance()->guiSettings->setValue("atool_abs_calibration","ref_spectrum",cbSpectra->currentText());
    refSpectrum = cbSpectra->currentSpectrum().spectrum;

    // recalculate data interval
    processDataInterval();
}



void AToolAbsCalibration::onGuiSettingsChanged()
{
    // re-select last reference spectrum selection
    cbSpectra->setCurrentText(Core::instance()->guiSettings->value("atool_abs_calibration","ref_spectrum","").toString());
    refSpectrumChanged(0);
}


void AToolAbsCalibration::copyToClipboard()
{
    QList<QTableWidgetItem*> items =   calResultTableWidget->selectedItems();

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

    // print data line per line to string
    for(int r = minr; r <= maxr; r++)
    {
        for(int c = minc; c <= maxc; c++)
        {
            data.append(calResultTableWidget->item(r,c)->text());

            // separate values in cols
            if(c < maxc)
                data.append("\t");
        }

        if(r < maxr)
            data.append("\n");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(data);
}


/*******
 * BUTTONS
 */

void AToolAbsCalibration::onClearPlotButtonReleased()
{
   calPlot->detachAllCurves();
   calPlot->plotTextLabelVisible(true);
   //processDataInterval();
}
