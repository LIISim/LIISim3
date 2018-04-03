#include "tutorialviewer.h"

#include <QDebug>
#include <Core.h>

//#define SHOW_OBJECT_IDENTIFIER

TutorialViewer::TutorialViewer(QWidget *parent) : QWidget(parent)
{
    lightBox = new QLightBoxWidget(parent);

    currentIndex = 0;

    firstStart = false;

    // General text

    QString runTreeCaption = "Loaded measurement runs tree";
    QString runTreeText = "This box lists all loaded measurement runs, ordered in groups. \n"
                          "Right click either on the group or on the measurement run shows further options of creating groups or toggle colormaps.";

    QString runDetailsCaption = "Measurement run details";
    QString runDetailsText = "Details of the currently selected measurement run are shown in this box. \n"
                             "These values can be used as parameters for the processing steps and are \n"
                             "saved for every measurement run in a corresponding '<RunName>_settings.txt' file.";

    QString runPlotCaption = "Signal plot";
    QString runPlotText = "Visualizes the currently selected measurement run(s) from the left box.";

    QString memoryBox = "Shows you, how much memory is currently used by LIISim. \n"
                        "\"Check Memory\": Available system memory is calculated before signal processing.\n\nNote: "
                        "The program could crash when this option is disabled\n"
                        "and the amount of memory needed for signal processing\n"
                        "exceeds the memory limit of this machine!";

    QString notificationBox =  "Shows you if there are any pending warnings / errors.\n"
                               "A click opens the warning / error history and resets\nthe counter.";

    // --- Homescreen ---

     addView(new TutorialView("",
                             "Home",
                             QString("This is the Home screeen. Here you can find the log message "
                                     "window and various tools for \n"
                                     "import of signals, sessions and settings. \n"
                                     "This tutorial guides you through the main "
                                     "functionalities of this module."),
                             Qt::AlignCenter,
                             "Homescreen",
                              false,
                              false));

    addView(new TutorialView("HS_SESSION_BOX",
                             "Sessions box",
                             "This box can be used to load and save sessions. A session consists of the currently \n"
                             "loaded measurement runs, assigned groups and the processing chains including custom parameters.",
                             Qt::AlignCenter,
                             "Homescreen",
                             false,
                             true));

    addView(new TutorialView("HS_SETTINGS_BOX",
                             "Settings box",
                             "Using this box you can manually load and save the program settings, \n "
                             "e.g. all GUI settings, modeling settings, database directory, ... \n\n"
                             "\"Automatic signal loading\": data from the last session are automatically loaded \n"
                             "\"Save as default\": Instantly save all current program settings and session. \n"
                             "(This is automatically done when the program is closed.)",
                             Qt::AlignCenter,
                             "Homescreen",
                             false,
                             true));

    addView(new TutorialView("HS_FILES_BOX",
                             "Signal data box",
                             "Load and save measurement runs. \nThese buttons open the file import/export window.",
                             Qt::AlignCenter,
                             "Homescreen",
                             false,
                             true));

    addView(new TutorialView("HS_SETTINGS_WIDGET",
                             "General settings",
                             "Shows you some general program settings and the option to show all tutorials again.",
                             Qt::AlignCenter,
                             "Homescreen"));

    addView(new TutorialView("HS_LOG_WIDGET",
                             "Log message history",
                             "Shows you the history of all log messages since the last program start.",
                             Qt::AlignCenter,
                             "Homescreen",
                             true));

    addView(new TutorialView("HS_NOTIFICATION_CENTER",
                             "Notification center",
                             notificationBox,
                             Qt::AlignRight,
                             "Homescreen",
                             false,
                             true));

    addView(new TutorialView("HS_MEMORY_BOX",
                             "Memory usage box",
                             memoryBox,
                             Qt::AlignRight,
                             "Homescreen",
                             false,
                             true));

    // --- Database editor ---

    addView(new TutorialView("",
                            "Database Module",
                            QString("This module visualizes the currently loaded database files. \n\n"
                                    "Database files contain equations for the calculation of physical properties: \n\n"
                                    "- Material: Particle material (i.e. soot, silicon,...) \n"
                                    "- Gas: Surounding gas, which is used in the GasMixture files \n"
                                    "- Gas Mixtures: Individual compositions from the Gas files can be defined \n\n"
                                    "- LIISettings: These files contain information about the LII detection device \n"
                                    "  (bandpass wavelength, calibration, ND-filter transmissions ...)\n"),
                            Qt::AlignCenter,
                            "DatabaseWindow",
                             false,
                             false));

    addView(new TutorialView("DBE_CURRENT_PATH",
                             "Current database path",
                             "Shows the current path, where the database files are located.",
                             Qt::AlignRight,
                             "DatabaseWindow"));

    addView(new TutorialView("DBE_BTN_OPEN_FOLDER",
                             "Open database folder",
                             "This button opens the database folder in windows explorer.",
                             Qt::AlignRight,
                             "DatabaseWindow"));

    addView(new TutorialView("DBE_BTN_CHANGE_FOLDER",
                             "Change database folder",
                             "Select a different database folder with this button.",
                             Qt::AlignRight,
                             "DatabaseWindow"));

    addView(new TutorialView("DBE_BTN_SCAN_DIR",
                             "Database scan",
                             "When changes to the database files have been made it is necessary\n"
                             "to rescan the database folder and load all database files again.",
                             Qt::AlignRight,
                             "DatabaseWindow"));

    addView(new TutorialView("DBE_EDITOR_TABS",
                             "Database",
                             "Detailed information about the database files can be shown by \n"
                             "selecting the database file in the left box.",
                             Qt::AlignCenter,
                             "DatabaseWindow",
                             true));

//    addView(new TutorialView("DBE_NOTIFICATION_CENTER",
//                             "Notification center",
//                             notificationBox,
//                             Qt::AlignRight,
//                             "DatabaseWindow",
//                             false,
//                             true));

//    addView(new TutorialView("DBE_MEMORY_BOX",
//                             "Memory usage box",
//                             memoryBox,
//                             Qt::AlignRight,
//                             "DatabaseWindow",
//                             false,
//                             true));

    // --- Signal processing ---

    addView(new TutorialView("",
                            "Signal Processing Module",
                            QString("This module provides signal processing and temperature calculation tools. \n\n"
                                    "After processing, data can be viewed in the Analysis Tools or \n"
                                    "temperature traces can be used in the Fit Tools for heat-transfer modeling."),
                            Qt::AlignCenter,
                            "SP",
                            false,
                            false));

    addView(new TutorialView("SPE_FILES_BOX",
                             "Signal data box",
                             "Load and save measurement runs via this box.",
                             Qt::AlignCenter,
                             "SP",
                             false,
                             true));

    addView(new TutorialView("SPE_CALC_BOX",
                             "Calculation box",
                             "Gives you different options to calculate signal data: \n"
                             "- The buttons on the left start/cancel the calculations or reset the calculation status. \n\n"
                             "- \"Single/Group/All\": These options can be used to calculate only a single \n"
                             "   measurement runs or groups. \n\n"
                             "- \"Raw/Absolute/Temperature\": Selects which processing chains should be processed \n\n"
                             "- \"Material for Spectroscopic Model\": This Material file is used for temperature\n"
                             "   calculations. (uses E(m) values of this database file)",
                             Qt::AlignCenter,
                             "SP",
                             false,
                             true));

    addView(new TutorialView("SPE_SELECTION_BOX",
                             "Current selection box",
                             "Shows the currently selected measurement run name and number of available signals. \n"
                             "The spinbox indicates which signal is showed in the plots below.",
                             Qt::AlignCenter,
                             "SP",
                             false,
                             true));

    addView(new TutorialView("SPE_PLOT_OPTIONS_BOX",
                             "Plot options box",
                             "Gives you some options to alter how the plot is displayed.",
                             Qt::AlignCenter,
                             "SP",
                             false,
                             true));

    addView(new TutorialView("SPE_NOTIFICATION_CENTER",
                             "Notification center",
                             notificationBox,
                             Qt::AlignRight,
                             "SP",
                             false,
                             true));

    addView(new TutorialView("SPE_MEMORY_BOX",
                             "Memory usage box",
                             memoryBox,
                             Qt::AlignRight,
                             "SP",
                             false,
                             true));

    addView(new TutorialView("TUTORIAL_SE_RUN_TREE",
                             runTreeCaption,
                             runTreeText,
                             Qt::AlignCenter,
                             "SP"));


    addView(new TutorialView("TUTORIAL_SE_DETAILS",
                             runDetailsCaption,
                             runDetailsText,
                             Qt::AlignCenter | Qt::AlignBottom,
                             "SP"));

    addView(new TutorialView("SPE_PCHAINS",
                             "Processing chains",
                             "These are the three available processing chains for basic signal processing and temperature calculation.\n\n"
                             "They are executed in the order: RAW -> ABSOLUTE -> TEMPERATURE\n\n"
                             "The output of both, RAW/ABSOLUTE processing chain can be used as input for the TEMPERATURE processing chain. \n\n"
                             "In general, raw and absolute processing chain can be used independently to apply different processing steps \n"
                             "and compare the results.",
                             Qt::AlignRight,
                             "SP"));

      addView(new TutorialView("SPE_PCHAIN_RAW",
                             "Raw processing chain",
                             "RAW chain is usually used to visualize unprocessed (or minimally processed) data and later compare this to \n"
                             "the outcome of the ABSOLUTE processing chain. \n\n"
                             "This helps for example in cases, when the raw voltage data needs to be compared against the calibrated data.",
                             Qt::AlignRight | Qt::AlignTop,
                             "SP"));

    addView(new TutorialView("SPE_PCHAIN_ABS",
                             "Absolute processing chain",
                             "Absolute processing chain is used to apply any correction steps (baseline, calibration, ND-filter ...) \n\n"
                             "The output of this processing chain can then be used to calculate a temperature trace using the \n"
                             "TEMPERATURE processing chain, or can be compared against RAW data.",
                             Qt::AlignRight,
                             "SP"));

    addView(new TutorialView("SPE_PCHAIN_TEMP",
                             "Temperature processing chain",
                             "TEMPERATURE processing chain uses 'TemperatureCalculator' to calculate a temperature trace from \n"
                             "either RAW or ABSOLUTE data. After temperature calculation further processing steps can be added\n"
                             "(i.e., MovingAverage for smoothing).",
                             Qt::AlignRight | Qt::AlignBottom,
                             "SP"));

    addView(new TutorialView("SPE_ALL_PLOTS",
                             "Processing result visualization",
                             "These three plots can be used to visualize and compare each single processing step: \n\n"
                             "RAW processing chain (top)\n"
                             "ABSOLUTE processing chain (middle)\n"
                             "TEMPERATURE processing chain (bottom)\n\n"
                             "Plot tools (above) can be used to link plots, so that they always show the same section.",
                             Qt::AlignLeft,
                             "SP",
                             false,
                             false));

//    addView(new TutorialView("SPE_RAW_PLOT",
//                             "Raw signal plot",
//                             "Shows you the raw signal plot of the currently selected measurement run.",
//                             Qt::AlignLeft | Qt::AlignTop ,
//                             "SP"));

//    addView(new TutorialView("SPE_ABS_PLOT",
//                             "Absolute signal plot",
//                             "Shows you the absolute signal plot of the currently selected measurement run.",
//                             Qt::AlignLeft,
//                             "SP"));

//    addView(new TutorialView("SPE_TEMP_PLOT",
//                             "Temperature trace plot",
//                             "Shows you the temperature trace plot of the currently selected measurement run.",
//                             Qt::AlignLeft | Qt::AlignBottom,
//                             "SP"));


    // --- Analysis tools ---

    // Signal plot tools

    addView(new TutorialView("",
                            "AnalysisTool: Plotter",
                            QString("This module helps to compare signal traces of different measurement runs against each other.\n"
                                    "One or more measurement runs can be selected and then be compared in the same plot."),
                            Qt::AlignCenter,
                            "AT_SPT",
                            false,
                            false));

    addView(new TutorialView("AT_MRUN_TREEVIEW",
                             runTreeCaption,
                             runTreeText,
                             Qt::AlignCenter,
                             "AT_SPT"));

    addView(new TutorialView("AT_MRUN_DETAILS",
                             runDetailsCaption,
                             runDetailsText,
                             Qt::AlignCenter | Qt::AlignBottom,
                             "AT_SPT"));

    addView(new TutorialView("AT_SPT_PLOT",
                             runPlotCaption,
                             runPlotText,
                             Qt::AlignRight,
                             "AT_SPT",
                             true));

    // Temperature fit

    addView(new TutorialView("",
                            "AnalysisTool: Temperature Fit",
                            QString("This module can be used to analyze the temperature fits from the TemperatureCalculator:Spectrum. \n"
                                    "Single datapoints of a measurement run can be picked using a data cursor tool and the \n"
                                    "parameters for every fitting iteration is visualized in a table and multiple plots."),
                            Qt::AlignCenter,
                            "AT_TF",
                            false,
                            false));

    addView(new TutorialView("AT_MRUN_TREEVIEW",
                             runTreeCaption,
                             runTreeText,
                             Qt::AlignCenter,
                             "AT_TF"));

    addView(new TutorialView("AT_MRUN_DETAILS",
                             runDetailsCaption,
                             runDetailsText,
                             Qt::AlignCenter | Qt::AlignBottom,
                             "AT_TF"));

    addView(new TutorialView("AT_TF_PLOT",
                             runPlotCaption,
                             runPlotText + "\n"
                                           "Use the data cursor tool to pick a data point from the curve in this plot.",
                             Qt::AlignCenter,
                             "AT_TF"));


    addView(new TutorialView("AT_TF_FIT_PLOT",
                             "Fit Visualization (Spectrum)",
                             "The fitted spectrum and the data for all channels is shown for the selected data point.",
                             Qt::AlignRight | Qt::AlignBottom,
                             "AT_TF"));

    addView(new TutorialView("AT_TF_FIT_RESULT_TABLE",
                             "Fit Iteration Table",
                             "The iterations for the temperature fit are shown in this table. Click on an iteration to \n"
                             "visualize the specific spectrum in the plot above.",
                             Qt::AlignRight,
                             "AT_TF"));

    addView(new TutorialView("AT_TF_RESULT_TAB",
                             "Fit results (Trace/Iterations) / Chi-squared function (2D)\n\n",
                             "Fit Results:\n"
                             "  Final result: Summarizes the final fitting results for the complete temperature trace\n"
                             "  Iterations: For the selected data point detailed information about the sequence of iterations is shown\n\n"
                             "Chi-squared function (2D): the sequence of iterations is visualized on a 2D chi-square map with \n"
                             "                           the two fitting parameters on the x-/y-axis (Scaling factor C and Temperature)\n"
                             "  Red triangle: Initial fitting parameters\n"
                             "  Crosses: Iterations are connected in their chronological order",
                             Qt::AlignCenter | Qt::AlignTop,
                             "AT_TF"));


    // Parameter analysis

    addView(new TutorialView("",
                            "AnalysisTool: Parameter Analysis",
                            QString("This module can be used to compare experimental parameters of different measurement runs. \n"
                                    "Multiple curves can be created by selecting parameters for x-axis and y-axis, which is then visualized \n"
                                    "in a plot and table.\n\n"
                                    "Example: laser fluence (x-axis) vs. average temperature range (y-axis)"),
                            Qt::AlignCenter,
                            "AT_PA",
                            false,
                            false));

    addView(new TutorialView("AT_MRUN_TREEVIEW",
                             runTreeCaption,
                             runTreeText,
                             Qt::AlignCenter,
                             "AT_PA"));

    addView(new TutorialView("AT_MRUN_DETAILS",
                             runDetailsCaption,
                             runDetailsText,
                             Qt::AlignCenter | Qt::AlignBottom,
                             "AT_PA"));

    addView(new TutorialView("AT_PA_PLOT",
                             runPlotCaption,
                             runPlotText,
                             Qt::AlignCenter | Qt::AlignBottom,
                             "AT_PA"));

    addView(new TutorialView("AT_PA_CURVE_SETTINGS",
                             "Analysis curve settings",
                             "The settings to add a parameter analysis curve:\n\n"
                             "1) Select the x-axis/y-axis and parameter of interest\n"
                             "2) Link channel if the channels for x-axis/y-axis are the same\n"
                             "3) Add single curve or add all available channels (works only with linked channels)",
                             Qt::AlignCenter | Qt::AlignBottom,
                             "AT_PA"));

    addView(new TutorialView("AT_PA_PARAMETER_PLOT",
                             "Analysis curve plot",
                             "This plot shows all enabled parameter analysis curves. \n\n"
                             "You can toggle visability and change colors in the left bottom box.",
                             Qt::AlignRight | Qt::AlignBottom,
                             "AT_PA"));

    addView(new TutorialView("AT_PA_TABLE_CURVE",
                             "Analysis curves table",
                             "This table contains all parameter analysis curves.",
                             Qt::AlignCenter,
                             "AT_PA"));



    addView(new TutorialView("AT_PA_TABLE_DATA",
                             "Analysis curve data table",
                             "Shows the data from all enabled parameter analysis curves. Data can \nbe selected and copy-pasted into any spreadsheet software.",
                             Qt::AlignRight,
                             "AT_PA"));

    // --- Fit tools ---

    // Fit creator

    addView(new TutorialView("FC_RUN_TREEVIEW",
                             runTreeCaption,
                             runTreeText,
                             Qt::AlignCenter,
                             "FitCreator"));

    addView(new TutorialView("FC_RUN_DETAILS",
                             runDetailsCaption,
                             runDetailsText,
                             Qt::AlignCenter | Qt::AlignBottom,
                             "FitCreator"));

    addView(new TutorialView("FC_RUN_PLOT",
                             "Temperature plot",
                             "This plot visualizes the selected temperature traces and allows to select a range for the fit.\n\n"
                             "The PlotTool 'select fit range' allows selection via click/drag, but you can also enter\n"
                             "the range manually.\n\n"
                             "The selected range is used for any further processing within this tool.",
                             Qt::AlignCenter | Qt::AlignBottom,
                             "FitCreator"));

    addView(new TutorialView("FC_MODELING_SETTINGS",
                             "Modeling settings",
                             "The heat transfer model can be combined with physical properties from different database files.\n\n"
                             "Caution: We tested the heat transfer models only with the associated database files.",
                             Qt::AlignCenter,
                             "FitCreator"));

    addView(new TutorialView("FC_INIT_FIT_PARAMETERS",
                             "Initial fit parameters",
                             "These parameters are used as start parameters for the fit. If you want to keep a parameter \n"
                             "constant during all iterations, you can click the checkbox 'Fixed'.",
                             Qt::AlignCenter,
                             "FitCreator"));

    addView(new TutorialView("FC_NUMERIC_SETTINGS",
                             "Numeric settings",
                             "When the maximum number of iterations is reached, the fitting will stop and return the \n"
                             "result from the last iteration. Make sure to keep this number sufficiently high.\n\n"
                             "Various ordinary differential equation (ODE) solver of the Boost library are \n"
                             "implemented. Please see the user guide for further information.\n\n"
                             "ODE accuracy can be increased, but increases also processing time.\n\n"
                             "For signals with high gradients (especially for high laser fluences), \n"
                             "make sure to use sufficiently high accuracy and the right solver. \n"
                             "(We recommend 'Dormand-Prince 5')",
                             Qt::AlignCenter,
                             "FitCreator"));

    addView(new TutorialView("FC_SIMULATION_ONLY_SETTINGS",
                             "Simulation only settings",
                             "These settings are used for modeling when using the 'Simulate' button.",
                             Qt::AlignCenter,
                             "FitCreator"));

    addView(new TutorialView("FC_BTN_START_FIT",
                             "Fit start button",
                             "If clicked, starts a fit run with the current measurement run and settings.",
                             Qt::AlignCenter | Qt::AlignTop,
                             "FitCreator"));

    addView(new TutorialView("FC_BTN_SIMULATION",
                             "Fit simulation",
                             "If clicked, starts a fit simulation with the current settings.\n\n"
                             "This function can be used without loading any data.",
                             Qt::AlignCenter | Qt::AlignTop,
                             "FitCreator"));

    addView(new TutorialView("FC_FIT_LIST",
                             "Fit run list",
                             "This list contains all fit runs and simulations. \n"
                             "Visibility of data/model can be toggled and compared.",
                             Qt::AlignCenter | Qt::AlignTop,
                             "FitCreator"));


    addView(new TutorialView("FC_DATA_VISUALIZATION",
                             "Fit details",
                             "Shows a summary of the selected fit run/simulation.",
                             Qt::AlignLeft | Qt::AlignBottom,
                             "FitCreator"));

    addView(new TutorialView("FC_RESULT_VISUALIZATION",
                             "Fit result plot",
                             QString("Visualizes the enabled fit results, with the following plots:\n")
                             .append("\n- Temperature traces: Shows traces of the temperature overlayed with the modeled trace.")
                             .append("\n- Heat transfer rates (trace): Shows the simulated heat transfer rates for evaporation, conduction and radiation.")
                             .append("\n- Particle diameter (trace): Shows the temporal change of the particle diameter during the LII signal.\n\n"
                                     "The following plots show information about the progress of fit iterations:\n")
                             .append("\n- Particle diameter (iterations): Fit parameter changes of the particle diameter.")
                             .append("\n- Gas temperature (iterations): Fit parameter changes of the gas temperature.")
                             .append("\n- Fit error (iterations): Shows the fit error between model and data."),
                             Qt::AlignCenter | Qt::AlignTop,
                             "FitCreator"));


    connect(lightBox, SIGNAL(onLastView()), this, SLOT(showLast()));
    connect(lightBox, SIGNAL(onNextView()), this, SLOT(showNext()));
    connect(lightBox, SIGNAL(onCloseView()), this, SLOT(closeView()));
}


void TutorialViewer::setFirstStart()
{
    if(!firstStart)
    {
        TutorialView *view = new TutorialView("WELCOME_SCREEN",
                                 "Welcome to LIISim",
                                 QString("Since this seems to be your first start of LIISim, we would like to")
                                 .append("\nshow you a little around. This should give you a short introduction")
                                 .append("\ninto all elements of LIISim so you can begin to work right away.")
                                 .append("\n")
                                 .append("\nIf you would like to open these tutorials at a later time, use the")
                                 .append("\n'Help' button in the upper right corner.\n"),
                                 Qt::AlignCenter,
                                 "Homescreen");

        _views.value(view->moduleTabName)->prepend(view);

        firstStart = true;
    }
}


void TutorialViewer::addView(TutorialView *view)
{
    if(!_views.contains(view->moduleTabName))
        _views.insert(view->moduleTabName, new QList<TutorialView*>);
    _views.value(view->moduleTabName)->append(view);
}


void TutorialViewer::showView()
{
    if(currentIndex >= currentViews->size())
    {
        currentIndex = 0;
        closeView();
        return;
    }

    if(!currentViews->isEmpty())
    {
        if(currentIndex == 0)
        {
            // hide back button
            lightBox->setButtonBackVisible(false);
        }
        else
            lightBox->setButtonBackVisible(true);

        TutorialView* view = currentViews->at(currentIndex);

        lightBox->setTitle(view->caption);

        // DEBUGGING: #define SHOW_OBJECT_IDENTIFIER to see the object name in GUI
#ifdef SHOW_OBJECT_IDENTIFIER
        lightBox->setDescription(view->objName + "\n" + view->text + "\n");
#else
        lightBox->setDescription(view->text + "\n");
#endif
        lightBox->setOrientation(view->align);

        if(view->topBarItem)
            lightBox->setHighlightArea(getTopBarViewRect(view->objName));
        else
            lightBox->setHighlightArea(getViewRect(view->objName, view->moduleTabName));

        lightBox->setTextColorBlack(view->textColorBlack);
        lightBox->show();
    }
}


void TutorialViewer::showTutorial(QString moduleName)
{
    if(!_views.contains(moduleName))
        return;

    currentViews = _views.value(moduleName);

    if(currentViews->isEmpty())
        return;

    currentIndex = 0;

    // hide back button
    lightBox->setButtonBackVisible(false);

    showView();
}


void TutorialViewer::showLast()
{
    if(currentIndex < 1)
        return;

    currentIndex--;
    showView();
    lightBox->repaint();
}


void TutorialViewer::showNext()
{
    currentIndex++;
    showView();
    lightBox->repaint();
}


void TutorialViewer::closeView()
{
   lightBox->hide();
}


QRect TutorialViewer::getViewRect(QString objName, QString moduleName)
{
    if(objName == "WELCOME_SCREEN" && moduleName == "Homescreen")
        return QRect();

    if(objName == "")
        return QRect();

    int x1, y1, w, h;
    int margin = 3; // margin arround the boxes in pixel

    QWidget* module = parent()->findChild<QWidget*>(moduleName);

    if(module == nullptr)
    {
        qDebug() << "TutorialViwer::getViewRect(): error - module not found: " + moduleName;
        return QRect();
    }

    QWidget* child = module->findChild<QWidget *>(objName, Qt::FindChildrenRecursively);

    if(child == nullptr)
    {
        qDebug() << "TutorialViwer::getViewRect(): error - child object not found: " + objName;
        return QRect();
    }

    // get widget geometry
    QRect rect = child->geometry();

    w = rect.width() + 2*margin;
    h = rect.height() + 2*margin;

    // get global coords
    QPoint pos = child->mapToGlobal(rect.topLeft());

    x1 = pos.x() - rect.x() - parentWidget()->geometry().x() - margin;
    y1 = pos.y() - rect.y() - parentWidget()->geometry().y() - margin;

//    qDebug() << " ---- " + objName + " -----";
//    qDebug() << pos;
//    qDebug() << "Window: " << parentWidget()->geometry();
//    qDebug() << x1 << y1 << w << h;

    return QRect(x1,y1,w,h);
}

QRect TutorialViewer::getTopBarViewRect(QString objName)
{
    int x1, y1;

    QWidget* topBar = parent()->findChild<QWidget*>("MasterTopBar");

    if(topBar == nullptr)
    {
        qDebug() << "TutorialViewer::getTopBarViewRect(): error - could not find MasterTopBar";
        return QRect();
    }

    QWidget* child = topBar->findChild<QWidget*>(objName);

    if(child == nullptr)
    {
        qDebug() << "TutorialViewer::getTopBarViewRect(): error - could not find top bar element: " + objName;
        return QRect();
    }

    // get widget geometry
    QRect rect = child->geometry();

    // get global coords
    QPoint pos = child->mapToGlobal(rect.topLeft());

    x1 = pos.x() - rect.x() - parentWidget()->geometry().x();
    y1 = pos.y() - rect.y() - parentWidget()->geometry().y();

    return QRect(x1, y1, rect.width(), rect.height());
}



