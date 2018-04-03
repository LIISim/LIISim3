#include "ft_runplot.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>

#include "core.h"
#include "../../signal/mrungroup.h"
#include "../../signal/processing/temperatureprocessingchain.h"
#include "../../signal/processing/plugins/temperaturecalculator.h"


FT_RunPlot::FT_RunPlot(QWidget *parent) : QWidget(parent), colorGenerator(ColorGenerator::DYNAMIC_RUN_COLOR),
    identifier_temperature("Temperature"),
    identifier_intensity_raw("Intensity (raw)"),
    identifier_intensity_abs("Intensity (abs)"),
    identifierGroup("ft_plot"),
    identifierFitStart("range_start"),
    identifierFitEnd("range_end")
{
    rangeSelected = false;

    xStart = 0.0;
    xEnd = 0.0;

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    plot = new SignalPlotWidgetQwt(this);
    plot->setPlotLabelText("No measurements selected. Please select a run from the treeview (left) to display signal data.");
    plot->plotTextLabelVisible(true);

    // Mode selection
    QWidget *widgetSelection = new QWidget(this);
    QHBoxLayout *layoutWidgetSelection = new QHBoxLayout;
    layoutWidgetSelection->setMargin(0);
    widgetSelection->setLayout(layoutWidgetSelection);

    comboboxMode = new QComboBox(this);
    comboboxMode->addItem(identifier_temperature);

#ifdef LIISIM_FULL
    comboboxMode->addItem(identifier_intensity_abs);
    comboboxMode->addItem(identifier_intensity_raw);
#endif

    layoutWidgetSelection->addWidget(new QLabel("Mode"));
    layoutWidgetSelection->addWidget(comboboxMode);

    // Show available channels
    layoutChannelSelection = new QHBoxLayout;
    layoutChannelSelection->setMargin(0);
    layoutWidgetSelection->addLayout(layoutChannelSelection);


    // show "select range in plot" button
    patRange = plot->addPlotAnalysisTool("select fit range in plot", "Select a range to perform a FitRun on.", false);

    QToolBar *tbRange = new QToolBar(this);
    tbRange->addAction(plot->toolActions().last()); // last added PlotAnalysisTool
    layoutWidgetSelection->addWidget(tbRange);

    // Range
    le_fitStart = new LabeledLineEdit("Fit Range: ", NumberLineEdit::DOUBLE);
    le_fitEnd = new LabeledLineEdit(" ns to ", NumberLineEdit::DOUBLE);
    QLabel* le_fitlabel = new QLabel(" ns");

    le_fitStart->setMaximumWidth(110);
    le_fitEnd->setMaximumWidth(80);

    connect(le_fitStart, SIGNAL(valueChanged()), SLOT(updateRangeFromLineEdit()));
    connect(le_fitEnd, SIGNAL(valueChanged()), SLOT(updateRangeFromLineEdit()));

    layoutWidgetSelection->addWidget(le_fitStart);
    layoutWidgetSelection->addWidget(le_fitEnd);
    layoutWidgetSelection->addWidget(le_fitlabel);

    // Toolbar: Plot tools (pan, avg, zoom,...)
    QWidget *widgetButtons = new QWidget(this);
    QHBoxLayout *layoutWidgetButtons = new QHBoxLayout;
    layoutWidgetButtons->setMargin(0);
    widgetButtons->setLayout(layoutWidgetButtons);

    // Plot Tools
    QToolBar *toolbarPlot = new QToolBar("Plot Tools");
    QList<QAction*> actions = plot->toolActions();
    actions.removeAt(1);
    actions.removeLast();
    toolbarPlot->addActions(actions);
    actions.first()->triggered(true);

    layoutWidgetButtons->addWidget(toolbarPlot);


    QHBoxLayout *layoutHeader = new QHBoxLayout;
    layoutHeader->addWidget(widgetSelection);
    layoutHeader->addWidget(widgetButtons);
    layoutHeader->setAlignment(widgetSelection, Qt::AlignLeft);
    layoutHeader->setAlignment(widgetButtons, Qt::AlignRight);

    mainLayout->addLayout(layoutHeader);

    mainLayout->addWidget(plot);

    connect(comboboxMode, SIGNAL(currentIndexChanged(int)), SLOT(onModeComboboxIndexChanged()));

    connect(patRange, SIGNAL(dataSelected(double,double)), SLOT(onRangeSelected(double,double)));
    connect(patRange, SIGNAL(hidden()), SLOT(onRangeHidden()));

    connect(Core::instance()->guiSettings, SIGNAL(settingsChanged()), SLOT(onGUISettingsChanged()));

    onGUISettingsChanged();
}


FT_RunPlot::~FT_RunPlot()
{
    GuiSettings* gs = Core::instance()->guiSettings;
    gs->setValue(identifierGroup, identifierFitStart, le_fitStart->getValue());
    gs->setValue(identifierGroup, identifierFitEnd, le_fitEnd->getValue());
}


QList<int> FT_RunPlot::getSelectedChannel()
{
    if(comboboxMode->currentText() == identifier_temperature)
        return activeTempChannel;
    else if(comboboxMode->currentText() == identifier_intensity_abs)
        return activeAbsChannel;
    else if(comboboxMode->currentText() == identifier_intensity_raw)
        return activeRawChannel;

    return QList<int>();
}


bool FT_RunPlot::rangeValid()
{
    return rangeSelected;
}


double FT_RunPlot::getRangeStart()
{
    return xStart;
}


double FT_RunPlot::getRangeEnd()
{
    return xEnd;
}


Signal::SType FT_RunPlot::getSelectedSource()
{
    if(comboboxMode->currentText() == identifier_temperature)
        return Signal::TEMPERATURE;
    else if(comboboxMode->currentText() == identifier_intensity_abs)
        return Signal::ABS;
    else if(comboboxMode->currentText() == identifier_intensity_raw)
        return Signal::RAW;
}


void FT_RunPlot::update(QList<MRun*> runList)
{
    for(MRun *run : selectedMRuns)
        run->disconnect(this);

    selectedMRuns = runList;

    for(MRun *run : selectedMRuns)
        connect(run, SIGNAL(dataChanged(int,QVariant)), SLOT(onRunDataChanged(int,QVariant)));

    plot->detachAllCurves();

    if(comboboxMode->currentText() == identifier_temperature)
    {
        QList<int> channel;

        for(MRun *run : runList)
        {
            TemperatureProcessingChain *tpchain = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));

            for(int i = 0; i < tpchain->temperatureCalculatorCont(); i++)
            {
                TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(tpchain->getPlug(i));
                if(tc && !channel.contains(tc->temperatureChannelID()))
                    channel << tc->temperatureChannelID();
            }
        }

        std::sort(channel.begin(), channel.end());

        while(!channelCheckboxes.isEmpty())
        {
            QCheckBox *checkbox = channelCheckboxes.first();
            channelCheckboxes.removeFirst();
            layoutChannelSelection->removeWidget(checkbox);
            delete checkbox;
        }

        for(int channelNo : channel)
        {
            QCheckBox *checkbox = new QCheckBox(QString("T%0 ").arg(channelNo));
            checkbox->setProperty("chID", channelNo);
            if(activeTempChannel.contains(channelNo))
                checkbox->setChecked(true);
            connect(checkbox, SIGNAL(stateChanged(int)), SLOT(onChannelCheckboxStateChanged()));
            channelCheckboxes << checkbox;
            layoutChannelSelection->addWidget(checkbox);
        }

        int maxChannel = 0;
        for(int chNo : channel)
            if(maxChannel < chNo)
                maxChannel = chNo;
        colorGenerator.setParameter(runList.size(), maxChannel);

        updatePlot();
    }
    else if(comboboxMode->currentText() == identifier_intensity_abs || comboboxMode->currentText() == identifier_intensity_raw)
    {
        int maxChannelCount = 0;

        if(comboboxMode->currentText() == identifier_intensity_abs)
        {
            for(MRun *run : runList)
                if(maxChannelCount < run->getNoChannels(Signal::ABS))
                    maxChannelCount = run->getNoChannels(Signal::ABS);
        }
        else if(comboboxMode->currentText() == identifier_intensity_raw)
        {
            for(MRun *run : runList)
                if(maxChannelCount < run->getNoChannels(Signal::RAW))
                    maxChannelCount = run->getNoChannels(Signal::RAW);
        }

        while(!channelCheckboxes.isEmpty())
        {
            QCheckBox *checkbox = channelCheckboxes.first();
            channelCheckboxes.removeFirst();
            layoutChannelSelection->removeWidget(checkbox);
            delete checkbox;
        }

        for(int i = 1; i <= maxChannelCount; i++)
        {
            QCheckBox *checkbox = new QCheckBox(QString("Ch %0").arg(i));
            checkbox->setProperty("chID", i);
            if(comboboxMode->currentText() == identifier_intensity_abs)
                if(activeAbsChannel.contains(i))
                    checkbox->setChecked(true);
            if(comboboxMode->currentText() == identifier_intensity_raw)
                if(activeRawChannel.contains(i))
                    checkbox->setChecked(true);
            connect(checkbox, SIGNAL(stateChanged(int)), SLOT(onChannelCheckboxStateChanged()));
            channelCheckboxes << checkbox;
            layoutChannelSelection->addWidget(checkbox);
        }

        colorGenerator.setParameter(runList.size(), maxChannelCount);

        updatePlot();
    }
}


void FT_RunPlot::updatePlot()
{
    plot->detachAllCurves();

    plot->plotTextLabelVisible(selectedMRuns.isEmpty());
    if(selectedMRuns.isEmpty())
        plot->setPlotLabelText("No run selected. Please select a run from the measurement run list to display signal data.");

    if(comboboxMode->currentText() == identifier_temperature)
    {
        int runCount = 1;

        for(MRun *run : selectedMRuns)
        {
            TemperatureProcessingChain *tpchain = dynamic_cast<TemperatureProcessingChain*>(run->getProcessingChain(Signal::TEMPERATURE));
            MPoint *mp = run->getPost(run->data(2).toInt());

            for(int i = 0; i < tpchain->temperatureCalculatorCont(); i++)
            {
                TemperatureCalculator* tc = dynamic_cast<TemperatureCalculator*>(tpchain->getPlug(i));

                if(tc && activeTempChannel.contains(tc->temperatureChannelID()))
                {   
                    Signal signal = mp->getSignal(tc->temperatureChannelID(), Signal::TEMPERATURE);
                    QString label = QString("T%0 (%1)").arg(tc->temperatureChannelID()).arg(run->getName());
                    if(signal.data.isEmpty())
                        label.append(" (empty)");

                    QColor color = run->data(1).value<QColor>();

                    // same as abs/raw
                    //plot->setCurrentColor(run->group()->colorMap()->generateChannelColor(tc->temperatureChannelID(), tpchain->temperatureCalculatorCont(), color));

                    // new
                    //plot->setCurrentColor(colorGenerator.getColor(runCount, tc->temperatureChannelID()));

                    // same as signal processing editor
                    plot->setCurrentColor(plot->getTemperatureColor(tc->temperatureChannelID()));

                    plot->addSignal(signal, label, false);
                }
            }

            if(activeTempChannel.isEmpty())
            {
                plot->plotTextLabelVisible(true);
                plot->setPlotLabelText("No channel selected. Please select a channel above to display signal data");
            }
            runCount++;
        }
    }
    else if(comboboxMode->currentText() == identifier_intensity_abs || comboboxMode->currentText() == identifier_intensity_raw)
    {
        int runCount = 1;

        for(MRun *run : selectedMRuns)
        {
            MPoint *mp = run->getPost(run->data(2).toInt());

            if(comboboxMode->currentText() == identifier_intensity_abs)
            {
                for(int channel : activeAbsChannel)
                {
                    Signal signal = mp->getSignal(channel, Signal::ABS);
                    QString label = QString("%0 Channel %1").arg(run->name).arg(channel);
                    if(signal.data.isEmpty())
                        label.append(" (empty)");

                    QColor color = run->data(1).value<QColor>();
                    plot->setCurrentColor(run->group()->colorMap()->generateChannelColor(channel, run->getNoChannels(Signal::ABS), color));

                    //plot->setCurrentColor(colorGenerator.getColor(runCount, channel));
                    plot->addSignal(signal, label, false);
                }

                if(activeAbsChannel.isEmpty())
                {
                    plot->plotTextLabelVisible(true);
                    plot->setPlotLabelText("No channel selected. Please select a channel above to display signal data");
                }
            }
            else if(comboboxMode->currentText() == identifier_intensity_raw)
            {
                for(int channel : activeRawChannel)
                {
                    Signal signal = mp->getSignal(channel, Signal::RAW);
                    QString label = QString("%0 Channel %1").arg(run->name).arg(channel);
                    if(signal.data.isEmpty())
                        label.append(" (empty)");

                    QColor color = run->data(1).value<QColor>();
                    plot->setCurrentColor(run->group()->colorMap()->generateChannelColor(channel, run->getNoChannels(Signal::RAW), color));

                    //plot->setCurrentColor(colorGenerator.getColor(runCount, channel));
                    plot->addSignal(signal, label, false);
                }

                if(activeRawChannel.isEmpty())
                {
                    plot->plotTextLabelVisible(true);
                    plot->setPlotLabelText("No channel selected. Please select a channel above to display signal data");
                }
            }
            runCount++;
        }
    }
}


void FT_RunPlot::onModeComboboxIndexChanged()
{
    update(selectedMRuns);
}


void FT_RunPlot::onChannelCheckboxStateChanged()
{
    QCheckBox *checkbox = dynamic_cast<QCheckBox*>(QObject::sender());
    int channel = checkbox->property("chID").toInt();

    if(comboboxMode->currentText() == identifier_temperature)
    {
        if(checkbox->isChecked())
        {
            if(!activeTempChannel.contains(channel))
                activeTempChannel << channel;
        }
        else
        {
            if(activeTempChannel.contains(channel))
                activeTempChannel.removeAll(channel);
        }
    }
    else if(comboboxMode->currentText() == identifier_intensity_abs)
    {
        if(checkbox->isChecked())
        {
            if(!activeAbsChannel.contains(channel))
                activeAbsChannel << channel;
        }
        else
        {
            if(activeAbsChannel.contains(channel))
                activeAbsChannel.removeAll(channel);
        }
    }
    else if(comboboxMode->currentText() == identifier_intensity_raw)
    {
        if(checkbox->isChecked())
        {
            if(!activeRawChannel.contains(channel))
                activeRawChannel << channel;
        }
        else
        {
            if(activeRawChannel.contains(channel))
                activeRawChannel.removeAll(channel);
        }
    }

    updatePlot();
}


void FT_RunPlot::onRangeSelected(double start, double stop)
{
    rangeSelected = true;

    xStart = start * 1E-9;
    xEnd   = stop * 1E-9;

    // Update line edit - one decimal precision
    start = round(start * 10.0) / 10.0;
    stop = round(stop * 10.0) / 10.0;

    // update labeled line edits
    le_fitStart->setValue(start);
    le_fitEnd->setValue(stop);
}


void FT_RunPlot::onRangeHidden()
{
    rangeSelected = false;
    xStart = 0.0;
    xEnd   = 0.0;

    // update labeled line edits
    le_fitStart->setEmpty();
    le_fitEnd->setEmpty();
}


/**
 * @brief FT_RunPlot::updateRangeFromLineEdit when user changes line edit, marker is changed too
 */
void FT_RunPlot::updateRangeFromLineEdit()
{
    rangeSelected = true;

    xStart   = le_fitStart->getValue() * 1E-9;
    xEnd     = le_fitEnd->getValue() * 1E-9;

    patRange->setMarkers(xStart,xEnd);
}


void FT_RunPlot::onRunDataChanged(int pos, QVariant data)
{
    if(pos == 2) //data for signal count
        updatePlot();
}


void FT_RunPlot::onGUISettingsChanged()
{
    GuiSettings* gs = Core::instance()->guiSettings;
    le_fitStart->setValue(gs->value(identifierGroup, identifierFitStart, 0.0).toDouble());
    le_fitEnd->setValue(gs->value(identifierGroup, identifierFitEnd, 0.0).toDouble());

    if(le_fitStart->getValue() == 0.0 && le_fitEnd->getValue() == 0.0)
    {
        le_fitStart->setEmpty();
        le_fitEnd->setEmpty();
    }
    else
        updateRangeFromLineEdit();
}

