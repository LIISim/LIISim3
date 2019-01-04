#-------------------------------------------------
#
# Project created by QtCreator 2013-12-27T14:16:58
#
#-------------------------------------------------

CONFIG += c++11 # new version support

# -------------------------------------------------
# INCLUDE/EXCLUDE PICOSCOPE AND DATAACQUISITION-GUI
# ALWAYS ENABLE LIISIM FULL VERSION
# -------------------------------------------------

# Do not uncomment this unless you need to (and know what
# you do), as it breaks the switching function between lite
# and full version inside Qt Creator

# DEFINES += "LIISIM_FULL"

contains(DEFINES, LIISIM_FULL){
    message("LIISim3.pro: Full version will be build")
# Comment the following definitions out if you want to either
# exclude the NI DAQmx functionality ("LIISIM_NIDAQMX" only)
# or the data acquisition GUI ("LIISIM_PICOSCOPE") AND the
# NI DAQmx function.
# Note: You need to run qmake and rebuild (-> "Build" menu)
# in order to enable the changes, otherwise it might result
# in undefined behaviour and strange errors!
    DEFINES += "LIISIM_PICOSCOPE"
    DEFINES += "LIISIM_NIDAQMX"
}else{
    #TARGET = LIISim3
}
TARGET = LIISim3

# -------------------------------------------------
# EXTERNAL LIBRARY PATHS
# -------------------------------------------------

# external libraries path for LIISim3
LIISIM_EXTERNAL_LIB_PATH = $$PWD/externalLibraries

LIISIM_EXTERNAL_LIB_PATH_BOOST = boost

LIISIM_EXTERNAL_LIB_PATH_MATIO = matio

LIISIM_EXTERNAL_LIB_PATH_QWT = qwt

# see also picoscope.h and picoscopesettings.h
LIISIM_EXTERNAL_LIB_PATH_PICOSCOPE = picoscope6000

LIISIM_EXTERNAL_LIB_PATH_NIDAQMX = nidaqmx

#---------------
# LISIM3 SOURCES
#---------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

win32:RC_ICONS += resources/icons/program.ico


# -------------------------------
# ADD STYLESHEET FILES TO PROJECT
# -------------------------------

DISTFILES += \
    resources/style/style.qss \
    resources/style/ribbontabbarstyle.qss



SOURCES += \
    core.cpp \
    main.cpp \
    calculations/constants.cpp \
    calculations/numeric.cpp \
    calculations/fit/fitdata.cpp \
    calculations/fit/fititerationresult.cpp \
    calculations/fit/fitrun.cpp \
    calculations/fit/simrun.cpp \
    calculations/heattransfermodel.cpp \
    calculations/models/htm_kock_soot.cpp \
    calculations/models/htm_mansmann.cpp \
    calculations/temperature.cpp \
    database/databasecontent.cpp \
    database/databasemanager.cpp \
    database/structure/gasmixture.cpp \
    database/structure/gasproperties.cpp \
    database/structure/liisettings.cpp \
    database/structure/material.cpp \
    database/structure/opticalproperty.cpp \
    database/structure/property.cpp \
    general/channel.cpp \
    general/filter.cpp \    
    gui/analysisTools/analysistools.cpp \
    gui/analysisTools/atoolbase.cpp \    
    gui/analysisTools/tools/signalplottool.cpp \
    gui/analysisTools/tools/sptplotitem.cpp \
    gui/databaseEditor/databaseeditor.cpp \
    gui/databaseEditor/dbeditorwidget.cpp \
    gui/databaseEditor/gaseditor.cpp \
    gui/databaseEditor/gasmixeditor.cpp \
    gui/databaseEditor/liisettingseditor.cpp \
    gui/databaseEditor/materialeditor.cpp \
    gui/dataItemViews/treeview/dataitemtreeview.cpp \
    gui/dataItemViews/treeview/mrungrouptreewidgetitem.cpp \
    gui/dataItemViews/treeview/mruntreewidgetitem.cpp \
    gui/dataItemViews/treeview/processingplugintreewidgetitem.cpp \
    gui/dataItemViews/treeview/vistoggabletwi.cpp \
    gui/fitTools/fitcreator.cpp \
    gui/fitTools/fittools.cpp \
    gui/fitTools/ft_fitparamtable.cpp \
    gui/fitTools/ft_modelingsettingstable.cpp \
    gui/fitTools/ft_numericparamtable.cpp \
    gui/homescreen.cpp \
    gui/masterwindow.cpp \
    gui/settingsWidgets/generalsettingswidget.cpp \    
    gui/signalEditor/exportdialog.cpp \
    gui/signalEditor/gridablesplitter.cpp \
    gui/signalEditor/importdialog.cpp \
    gui/signalEditor/memusagewidget.cpp \
    gui/signalEditor/sessionloaddialog.cpp \
    gui/signalEditor/sessionsavedialog.cpp \
    gui/signalEditor/signalprocessingeditor.cpp \
    gui/utils/checkboxinput.cpp \
    gui/utils/colormap.cpp \
    gui/utils/customQwtPlot/customlogscaleengine.cpp \
    gui/utils/customQwtPlot/customlogtransform.cpp \
    gui/utils/customQwtPlot/customplotmagnifier.cpp \
    gui/utils/customQwtPlot/customplotoptionswidget.cpp \
    gui/utils/customQwtPlot/customplotpicker.cpp \
    gui/utils/customQwtPlot/signalplotcurve.cpp \
    gui/utils/datatablewidget.cpp \
    gui/utils/helpmanager.cpp \
    gui/utils/labeledcombobox.cpp \
    gui/utils/labeledlineedit.cpp \
    gui/utils/liifiltercombobox.cpp \
    gui/utils/liisettingscombobox.cpp \
    gui/utils/materialcombobox.cpp \
    gui/utils/numberlineedit.cpp \
    gui/utils/plotanalysistool.cpp \
    gui/utils/plotavgtool.cpp \
    gui/utils/plotfittool.cpp \
    gui/utils/ribbontabbar.cpp \
    gui/utils/ribbontoolbox.cpp \
    gui/utils/signalplotwidgetqwt.cpp \
    io/iobase.cpp \
    io/iocsv.cpp \
    io/iocustom.cpp \
    io/iomatlab.cpp \
    io/ioxml.cpp \    
    io/signalfileinfo.cpp \
    io/signaliorequest.cpp \
    logging/consoleouthandler.cpp \
    logging/logfilehandler.cpp \
    logging/logmessagewidget.cpp \
    logging/msghandlerbase.cpp \
    logging/statusmessagewidget.cpp \
    models/dataitem.cpp \
    models/datamodel.cpp \
    settings/fitparameter.cpp \
    settings/fitsettings.cpp \
    settings/generalsettings.cpp \
    settings/guisettings.cpp \
    settings/iosettings.cpp \
    settings/modelingsettings.cpp \
    settings/mrunsettings.cpp \
    settings/numericsettings.cpp \
    settings/settingsbase.cpp \
    signal/memusagemonitor.cpp \
    signal/mpoint.cpp \
    signal/mrun.cpp \
    signal/mrungroup.cpp \
    signal/processing/pluginfactory.cpp \
    signal/processing/plugins/arithmetic.cpp \
    signal/processing/plugins/baseline.cpp \
    signal/processing/plugins/calibration.cpp \
    signal/processing/plugins/convolution.cpp \
    signal/processing/plugins/dummyplugin.cpp \
    signal/processing/plugins/filterplugin.cpp \
    signal/processing/plugins/getsignalsection.cpp \
    signal/processing/plugins/movingaverage.cpp \
    signal/processing/plugins/multisignalaverage.cpp \
    signal/processing/plugins/normalize.cpp \
    signal/processing/plugins/overwrite.cpp \
    signal/processing/plugins/signalarithmetic.cpp \
    signal/processing/plugins/simpledatareducer.cpp \
    signal/processing/plugins/simplepeakvalidator.cpp \
    signal/processing/plugins/swapchannels.cpp \
    signal/processing/plugins/temperaturecalculator.cpp \    
    signal/processing/plugins/xshiftsignals.cpp \
    signal/processing/ppstepbuffer.cpp \
    signal/processing/processingchain.cpp \
    signal/processing/processingplugin.cpp \
    signal/processing/processingpluginconnector.cpp \
    signal/processing/processingplugininput.cpp \
    signal/processing/processingplugininputlist.cpp \
    signal/processing/processingtask.cpp \
    signal/processing/temperatureprocessingchain.cpp \
    signal/signal.cpp \
    signal/signalmanager.cpp \
    signal/signalpair.cpp \
    models/dataitemobserverobject.cpp \
    models/dataitemobserverwidget.cpp \
    gui/analysisTools/tools/atoolcalibration.cpp \
    gui/utils/baseplotwidgetqwt.cpp \
    gui/dataItemViews/treeview/dataitemtreewidget.cpp \
    gui/utils/customQwtPlot/baseplotcurve.cpp\
    signal/streampoint.cpp \
    gui/dataAcquisition/da_referencewidget.cpp \
    signal/mruncalculationstatus.cpp \
    signal/processing/plugins/resolutionreducer.cpp \
    general/nativeeventfilter.cpp \
    gui/utils/doublelabel.cpp \
    gui/dataAcquisition/channelvisualcontrolwidget.cpp \
    gui/dataAcquisition/dataacquisitionplotwidgetqwt.cpp \
    database/structure/laserenergy.cpp \
    database/structure/laserenergyproperties.cpp \
    gui/databaseEditor/laserenergyeditor.cpp \
    gui/analysisTools/tools/atooltemperaturefit.cpp \
    signal/spectrum.cpp \
    database/structure/spectrumdbe.cpp \
    gui/databaseEditor/spectrumeditor.cpp \
    gui/utils/checkboxgroupinput.cpp \
    database/structure/transmissiondbe.cpp \
    gui/databaseEditor/transmissioneditor.cpp \
    gui/analysisTools/tools/atoolabscalibration.cpp \
    gui/utils/spectracombobox.cpp \
	general/singleinstanceguard.cpp \
    signal/processing/plugins/savitzkygolay.cpp \
    gui/dataAcquisition/da_userdefinedparameters.cpp \
    gui/utils/customQwtPlot/signalplotintervalcurve.cpp \
    gui/utils/baseplotspectrogramwidgetqwt.cpp \
    gui/utils/colormapspectrogram.cpp \
    gui/analysisTools/tools/parameteranalysis.cpp \
    signal/covmatrix.cpp \
    gui/utils/udpeditor.cpp \
    gui/signalEditor/importdialoghelper.cpp \
    gui/utils/extendedtablewidget.cpp \
    gui/utils/calculationtoolbox.cpp \
    gui/fitTools/ft_runplot.cpp \
    gui/fitTools/ft_resultvisualization.cpp \
    gui/fitTools/ft_fitlist.cpp \
    gui/fitTools/ft_datatable.cpp \
    gui/fitTools/ft_simruntreeitem.cpp \
    gui/fitTools/ft_plotcurve.cpp \
    gui/fitTools/ft_runlistfitdataitem.cpp \
    gui/fitTools/ft_fitruntreeitem.cpp \
    gui/utils/colorgenerator.cpp \
    gui/utils/visibilitybutton.cpp \
    gui/fitTools/ft_parametertable.cpp \
    gui/fitTools/ft_datavisualization.cpp \
    gui/fitTools/ft_simsettings.cpp \    
    gui/utils/mathml/equationlist.cpp \
    gui/databaseEditor/databasewindow.cpp \
    gui/utils/lightbox/qlightboxwidget.cpp \
    gui/utils/lightbox/tutorialview.cpp \
    gui/utils/lightbox/tutorialviewer.cpp \
    calculations/models/htm_melton.cpp \
	gui/utils/notificationmanager.cpp \
    gui/utils/notificationcenter.cpp \
    gui/utils/notificationwindow.cpp \
    gui/utils/notificationmessage.cpp \
    calculations/models/htm_liu.cpp \
    io/consistencycheck.cpp \
    gui/utils/aboutwindow.cpp \
    calculations/models/htm_menser.cpp \
    gui/utils/minimizablewidget.cpp \
    calculations/standarddeviation.cpp \
    gui/utils/plotzoomer.cpp \
    signal/processing/plugins/transfer.cpp \
    gui/utils/mrundetailswidget.cpp \
    gui/utils/exportoverwritedialog.cpp \
    gui/databaseEditor/dbelementnamedialog.cpp \
    gui/databaseEditor/tablerowelement.cpp \
    gui/databaseEditor/gasmixturerow.cpp \
    gui/databaseEditor/liisettingsnamedialog.cpp \
    gui/analysisTools/tools/measurementlist.cpp \
    calculations/models/htm_musikhin.cpp \
    gui/utils/flowlayout.cpp


HEADERS  += \
    core.h \
    calculations/constants.h \
    calculations/numeric.h \
    calculations/fit/fitdata.h \
    calculations/fit/fititerationresult.h \
    calculations/fit/fitrun.h \
    calculations/fit/simrun.h \
    calculations/heattransfermodel.h \
    calculations/models/htm_kock_soot.h \
    calculations/models/htm_mansmann.h \
    calculations/temperature.h \
    database/databasecontent.h \
    database/databasemanager.h \
    database/structure/gasmixture.h \
    database/structure/gasproperties.h \
    database/structure/liisettings.h \
    database/structure/material.h \
    database/structure/opticalproperty.h \
    database/structure/property.h \
    general/channel.h \
    general/filter.h \
    general/LIISimException.h \
    general/LIISimMessageType.h \
    general/picoscopecommon.h \
    gui/analysisTools/analysistools.h \
    gui/analysisTools/atoolbase.h \    
    gui/analysisTools/tools/signalplottool.h \
    gui/analysisTools/tools/sptplotitem.h \
    gui/databaseEditor/databaseeditor.h \
    gui/databaseEditor/dbeditorwidget.h \
    gui/databaseEditor/gaseditor.h \
    gui/databaseEditor/gasmixeditor.h \
    gui/databaseEditor/liisettingseditor.h \
    gui/databaseEditor/materialeditor.h \
    gui/dataItemViews/treeview/dataitemtreeview.h \
    gui/dataItemViews/treeview/mrungrouptreewidgetitem.h \
    gui/dataItemViews/treeview/mruntreewidgetitem.h \
    gui/dataItemViews/treeview/processingplugintreewidgetitem.h \
    gui/dataItemViews/treeview/vistoggabletwi.h \
    gui/fitTools/fitcreator.h \
    gui/fitTools/fittools.h \
    gui/fitTools/ft_fitparamtable.h \
    gui/fitTools/ft_modelingsettingstable.h \
    gui/fitTools/ft_numericparamtable.h \
    gui/homescreen.h \
    gui/masterwindow.h \
    gui/settingsWidgets/generalsettingswidget.h \    
    gui/signalEditor/exportdialog.h \
    gui/signalEditor/gridablesplitter.h \
    gui/signalEditor/importdialog.h \
    gui/signalEditor/memusagewidget.h \
    gui/signalEditor/sessionloaddialog.h \
    gui/signalEditor/sessionsavedialog.h \
    gui/signalEditor/signalprocessingeditor.h \
    gui/utils/checkboxinput.h \
    gui/utils/colormap.h \
    gui/utils/customQwtPlot/customlogscaleengine.h \
    gui/utils/customQwtPlot/customlogtransform.h \
    gui/utils/customQwtPlot/customplotmagnifier.h \
    gui/utils/customQwtPlot/customplotoptionswidget.h \
    gui/utils/customQwtPlot/customplotpicker.h \
    gui/utils/customQwtPlot/signalplotcurve.h \
    gui/utils/datatablewidget.h \
    gui/utils/helpmanager.h \
    gui/utils/labeledcombobox.h \
    gui/utils/labeledlineedit.h \
    gui/utils/liifiltercombobox.h \
    gui/utils/liisettingscombobox.h \
    gui/utils/materialcombobox.h \
    gui/utils/numberlineedit.h \
    gui/utils/plotanalysistool.h \
    gui/utils/plotavgtool.h \
    gui/utils/plotfittool.h \
    gui/utils/plugininputfield.h \
    gui/utils/ribbontabbar.h \
    gui/utils/ribbontoolbox.h \
    gui/utils/signalplotwidgetqwt.h \
    io/iobase.h \
    io/iocsv.h \
    io/iocustom.h \
    io/iomatlab.h \
    io/ioxml.h \    
    io/signalfileinfo.h \
    io/signaliorequest.h \
    logging/consoleouthandler.h \
    logging/logfilehandler.h \
    logging/logmessagewidget.h \
    logging/msghandlerbase.h \
    logging/statusmessagewidget.h \
    models/dataitem.h \
    models/datamodel.h \
    settings/fitparameter.h \
    settings/fitsettings.h \
    settings/generalsettings.h \
    settings/guisettings.h \
    settings/iosettings.h \
    settings/modelingsettings.h \
    settings/mrunsettings.h \
    settings/numericsettings.h \
    settings/settingsbase.h \
    signal/memusagemonitor.h \
    signal/mpoint.h \
    signal/mrun.h \
    signal/mrungroup.h \
    signal/processing/pluginfactory.h \
    signal/processing/plugins/arithmetic.h \
    signal/processing/plugins/baseline.h \
    signal/processing/plugins/calibration.h \
    signal/processing/plugins/convolution.h \
    signal/processing/plugins/dummyplugin.h \
    signal/processing/plugins/filterplugin.h \
    signal/processing/plugins/getsignalsection.h \
    signal/processing/plugins/movingaverage.h \
    signal/processing/plugins/multisignalaverage.h \
    signal/processing/plugins/normalize.h \
    signal/processing/plugins/overwrite.h \
    signal/processing/plugins/signalarithmetic.h \
    signal/processing/plugins/simpledatareducer.h \
    signal/processing/plugins/simplepeakvalidator.h \
    signal/processing/plugins/swapchannels.h \
    signal/processing/plugins/temperaturecalculator.h \
    signal/processing/plugins/xshiftsignals.h \
    signal/processing/ppstepbuffer.h \
    signal/processing/processingchain.h \
    signal/processing/processingplugin.h \
    signal/processing/processingpluginconnector.h \
    signal/processing/processingplugininput.h \
    signal/processing/processingplugininputlist.h \
    signal/processing/processingtask.h \
    signal/processing/temperatureprocessingchain.h \
    signal/signal.h \
    signal/signalmanager.h \
    signal/signalpair.h \
    models/dataitemobserverobject.h \
    models/dataitemobserverwidget.h \
    gui/utils/baseplotwidgetqwt.h \
    gui/analysisTools/tools/atoolcalibration.h \
    gui/dataItemViews/treeview/dataitemtreewidget.h \
    gui/utils/customQwtPlot/baseplotcurve.h\
    signal/streampoint.h \
    gui/dataAcquisition/da_referencewidget.h \
    signal/mruncalculationstatus.h \
    signal/processing/plugins/resolutionreducer.h \
    general/nativeeventfilter.h \
    gui/utils/doublelabel.h \
    gui/dataAcquisition/channelvisualcontrolwidget.h \
    gui/dataAcquisition/dataacquisitionplotwidgetqwt.h \
    database/structure/laserenergy.h \
    database/structure/laserenergyproperties.h \
    gui/databaseEditor/laserenergyeditor.h \
    gui/analysisTools/tools/atooltemperaturefit.h \
    signal/spectrum.h \
    database/structure/spectrumdbe.h \
    gui/databaseEditor/spectrumeditor.h \
    gui/utils/checkboxgroupinput.h \
    database/structure/transmissiondbe.h \
    gui/databaseEditor/transmissioneditor.h \
    gui/analysisTools/tools/atoolabscalibration.h \
    gui/utils/spectracombobox.h \
    general/singleinstanceguard.h \
    signal/processing/plugins/savitzkygolay.h \
    gui/dataAcquisition/da_userdefinedparameters.h \
    gui/utils/customQwtPlot/signalplotintervalcurve.h \
    gui/utils/baseplotspectrogramwidgetqwt.h \
    gui/utils/colormapspectrogram.h \
    gui/analysisTools/tools/parameteranalysis.h \
    signal/covmatrix.h \
    gui/utils/udpeditor.h \
    gui/signalEditor/importdialoghelper.h \
    gui/utils/extendedtablewidget.h \
    gui/utils/calculationtoolbox.h \
    gui/fitTools/ft_runplot.h \
    gui/fitTools/ft_resultvisualization.h \
    gui/fitTools/ft_fitlist.h \
    gui/fitTools/ft_datatable.h \
    gui/fitTools/ft_simruntreeitem.h \
    gui/fitTools/ft_plotcurve.h \
    gui/fitTools/ft_runlistfitdataitem.h \
    gui/fitTools/ft_fitruntreeitem.h \
    gui/utils/colorgenerator.h \
    gui/utils/visibilitybutton.h \
    gui/fitTools/ft_parametertable.h \
    gui/fitTools/ft_datavisualization.h \
    gui/fitTools/ft_simsettings.h \
    gui/utils/mathml/equationlist.h \
    gui/databaseEditor/databasewindow.h \
    gui/utils/lightbox/qlightboxwidget.h \
    gui/utils/lightbox/tutorialview.h \
    gui/utils/lightbox/tutorialviewer.h \
    calculations/models/htm_melton.h \
	gui/utils/notificationmanager.h \
    gui/utils/notificationcenter.h \
    gui/utils/notificationwindow.h \
    gui/utils/notificationmessage.h \
    calculations/models/htm_liu.h \
    io/consistencycheck.h \
    gui/utils/aboutwindow.h \
    general/profiler.h \
    calculations/models/htm_menser.h \
    gui/utils/minimizablewidget.h \
    signal/tempcalcmetadata.h \
    calculations/standarddeviation.h \
    gui/utils/plotzoomer.h \
    signal/processing/plugins/transfer.h \
    gui/utils/mrundetailswidget.h \
    gui/utils/exportoverwritedialog.h \
    gui/databaseEditor/dbelementnamedialog.h \
    gui/databaseEditor/tablerowelement.h \
    gui/databaseEditor/gasmixturerow.h \
    gui/databaseEditor/liisettingsnamedialog.h \
    gui/analysisTools/tools/measurementlist.h \
    gui/analysisTools/tools/measurementlisthelper.h \
    calculations/models/htm_musikhin.h \
    gui/utils/flowlayout.h

# include PicoScope/DataAcquisition Code
# if LIISIM_PICOSCOPE has been defined (see top of this file)
contains(DEFINES, LIISIM_PICOSCOPE){

    HEADERS += \
    gui/dataAcquisition/dataacquisitionwindow.h \
    settings/picoscopesettings.h \
    io/picoscope.h \
    io/laserenergyposition.h \
    gui/dataAcquisition/da_laserenergysettingswidget.h \
    gui/dataAcquisition/picoscopesettingswidget.h \
    gui/dataAcquisition/da_runsettingswidget.h \
    gui/dataAcquisition/da_triggerdialog.h \
    gui/dataAcquisition/da_exportsettingswidget.h

    SOURCES += \
    gui/dataAcquisition/dataacquisitionwindow.cpp \
    settings/picoscopesettings.cpp \
    io/picoscope.cpp \
    io/laserenergyposition.cpp \
    gui/dataAcquisition/da_laserenergysettingswidget.cpp \
    gui/dataAcquisition/picoscopesettingswidget.cpp \
    gui/dataAcquisition/da_runsettingswidget.cpp \
    gui/dataAcquisition/da_triggerdialog.cpp \
    gui/dataAcquisition/da_exportsettingswidget.cpp

}

contains(DEFINES, LIISIM_NIDAQMX){
    HEADERS += io/devicemanager.h \
    io/daqdevice.h \
    gui/dataAcquisition/da_blocksequence.h \
    gui/deviceManager/devicemanagerwidget.h \
    io/daqio.h

    SOURCES += io/devicemanager.cpp \
    io/daqdevice.cpp \
    gui/deviceManager/devicemanagerwidget.cpp \
    gui/dataAcquisition/da_blocksequence.cpp \
    io/daqio.cpp
}


# ---------------------------------
# ADD EXTERNAL LIBRARIES TO PROJECT
# ---------------------------------



exists("$$LIISIM_EXTERNAL_LIB_PATH") {
    message("OK: LIISim3.pro external library path (" $$LIISIM_EXTERNAL_LIB_PATH ")")

}else{
    message("ERROR: LIISim3.pro: cannot find external library path:" $$LIISIM_EXTERNAL_LIB_PATH)
}



# -------------
#  Qwt textengine from Qwt 6.1.2 (used in EquationList class)
# -------------

MMLPATH = $$LIISIM_EXTERNAL_LIB_PATH/mathml

exists("$$MMLPATH/") {
    message("OK: LIISim3.pro MML library")
    QT += xml

    HEADERS += \
        externalLibraries/mathml/qwt_mathml_text_engine.h \
        externalLibraries/mathml/qwt_mml_document.h

    SOURCES += \
        externalLibraries/mathml/qwt_mathml_text_engine.cpp \
        externalLibraries/mathml/qwt_mml_document.cpp
}


# -------------
# BOOST LIBRARY
# -------------

BOOSTPATH = $$LIISIM_EXTERNAL_LIB_PATH/$$LIISIM_EXTERNAL_LIB_PATH_BOOST

exists("$$BOOSTPATH/libs/") {
    INCLUDEPATH += $$BOOSTPATH/
    LIBS += -L$$BOOSTPATH/libs/
    message("OK: LIISim3.pro boost library")
}else{
    message("ERROR: LIISim3.pro: cannot find boost library (should be at $$BOOSTPATH/libs/) ")
}


# -------------
# MATIO LIBRARY
# -------------

MATIO_PATH = $$LIISIM_EXTERNAL_LIB_PATH/$$LIISIM_EXTERNAL_LIB_PATH_MATIO


exists("$$MATIO_PATH/"){

    INCLUDEPATH += $$MATIO_PATH/include/

    !win32-g++: !linux-g++:{
        !contains(QMAKE_TARGET.arch, x86_64){
            COMPINFO = "/x86/"
        }else{
            COMPINFO = "/x64/"
        }
    }
    exists("$$MATIO_PATH$$COMPINFO"){
        #message("OK: MATIO directory for current compiler: ($$MATIO_PATH$$COMPINFO)")

        LIBS += -L$$MATIO_PATH$$COMPINFO -llibmatio
        LIBS += -L$$MATIO_PATH$$COMPINFO -lzlibwapi
    }

    message("OK: LIISim3.pro MATIO library")
}else{
    message("ERROR: LIISim3.pro: cannot find MATIO library (should be at $$MATIO_PATH/libs/) ")
}



# -------------
# QWT LIBRARY
# -------------

QWTPATH = $$LIISIM_EXTERNAL_LIB_PATH/$$LIISIM_EXTERNAL_LIB_PATH_QWT

exists("$$QWTPATH"){
    message("OK: qwt directory for Qt $$QT_VERSION")

    INCLUDEPATH += $$QWTPATH/include

    COMPINFO
    win32-g++:{
        COMPINFO = "/minGW/x86"
    }
    !win32-g++: !linux-g++:{
        !contains(QMAKE_TARGET.arch, x86_64){
            message("INFO: x86 build")
            COMPINFO = "/msvc/x86"
        }else{
            message("INFO: x64 build")
            COMPINFO = "/msvc/x64"
        }
    }
    exists("$$QWTPATH$$COMPINFO"){
        message("OK: qwt directory for current compiler: ($$QWTPATH$$COMPINFO)")

        CONFIG(debug, debug|release){
            LIBS += -L$$QWTPATH$$COMPINFO/  -lqwtd  # for debugging
        }

        CONFIG(release, debug|release){
            LIBS += -L$$QWTPATH$$COMPINFO/  -lqwt  # for release mode
        }

    }else{
        message("ERROR: cannot find qwt directory for current compiler. Please check your "external libraries" folder: $$QWTPATH$$COMPINFO")
    }
}else{
    message("ERROR: Cannot find qwt library for qt version $$QT_VERSION! Please check your "external libraries" folder: $$QWTPATH")
}

# --------------------------------------------------
# Windows Process Status API (psaip)
# needs to be included explicitly when using minGW32
# --------------------------------------------------

win32-g++:{
    LIBS += -lpsapi
}
contains(DEFINES, LIISIM_FULL){
# --------------------------------------------------
# PicoScope library
# --------------------------------------------------

# include PicoScope library if LIISIM_PICOSCOPE has been defined (see top of this file)
contains(DEFINES, LIISIM_PICOSCOPE){

    PICOSCOPE_PATH = $$LIISIM_EXTERNAL_LIB_PATH/$$LIISIM_EXTERNAL_LIB_PATH_PICOSCOPE
    exists("$$PICOSCOPE_PATH") {
        message("OK: LIISim3.pro PicoScope directory")
        INCLUDEPATH += $$PICOSCOPE_PATH/include/
        DEPENDPATH += $$PICOSCOPE_PATH/include/

        win32-g++:{
            message("INFO: Using PicoScope x32 Library")
            LIBS += -L$$PICOSCOPE_PATH/x32/ -lps6000
        }

        !win32-g++: !linux-g++:{
            !contains(QMAKE_TARGET.arch, x86_64){
                message("INFO: Using PicoScope x32 Library")
                LIBS += -L$$PICOSCOPE_PATH/x32/ -lps6000
            }else{
                message("INFO: Using PicoScope x64 Library")
                LIBS += -L$$PICOSCOPE_PATH/x64/ -lps6000
            }
        }
    } else {
        message("LIISim3.pro ERROR: cannot find PicoScope library ")
    }
}

# --------------------------------------------------
# NI DAQmx library
# --------------------------------------------------

contains(DEFINES, LIISIM_NIDAQMX){
    NIDAQMX_PATH = $$LIISIM_EXTERNAL_LIB_PATH/$$LIISIM_EXTERNAL_LIB_PATH_NIDAQMX
    exists("$$NIDAQMX_PATH"){
        message("OK: LIISim3.pro NIDAQmx directory")
        INCLUDEPATH += $$NIDAQMX_PATH/include/
        DEPENDPATH += $$NIDAQMX_PATH/include/

        win32-g++:{
            message("INFO: Using NIDAQmx x32 Library")
            LIBS += -L$$NIDAQMX_PATH/lib32/msvc/ -lNIDAQmx
        }

        !win32-g++: !linux-g++:{
            !contains(QMAKE_TARGET.arch, x86_64){
                message("INFO: Using NIDAQmx x32 Library")
                LIBS += -L$$NIDAQMX_PATH/lib32/msvc/ -lNIDAQmx
            }else{
                message("INFO: Using NIDAQmx x64 Library")
                LIBS += -L$$NIDAQMX_PATH/lib64/msvc/ -lNIDAQmx
            }
        }

    }else{
        message("ERROR: LIISim3.pro: can not find NIDAQmx library")
    }
}

}

# http://www.tripleboot.org/?p=423
# Deploying Qt on XP and getting “not a valid Win32 application”
# This error will not occur for Qt with Visual Studio 2010, but it will when using Visual Studio 2012, 2013 or Visual Studio 2015.

#QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01
