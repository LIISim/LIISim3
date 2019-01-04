### 3.0.7
                        
##### Features
* DatabaseEditor: Properties can now be directly edited within the GUI and all supported property types can be selected from a dropdown menu
* AnalysisTools - MeasurementList: new analysis tool for visualization of all loaded MRuns and parameters in a table, supports copy/paste functionalities to clipboard
* AnalysisTools - TemperatureFit: customized Planck curves can be added for comparison to the 'Fit visualization' plot 

* AnalysisTools - TemperatureFit: added plot tools and auto-scaling to 'Fit visualization' plot 
* AnalysisTools - TemperatureFit: added plot tools for 'Result' plot
* AnalysisTools - ParameterAnalysis: new parameter types 'min/max of range' and 'time at min/max signal value'
* AnalysisTools - ParameterAnalysis: plot type of 'Parameter Visualization' plot can now be changed by right click menu
* Export: IOCSV: temperature traces can now be exported, export options added
                                                                                        

##### Bug fixes
* Code fixes for support of MSVC2017 compiler
* AnalysisTools - TemperatureFit: fixed iteration counter in spectrum plot
* AnalysisTools - TemperatureFit: auto scaling modified to prevent the plot to clip into the legend
* AnalysisTools - ParameterAnalysis: shows now ND-transmission for each channel instead of filter identifier
* DataTableWidget: x-values are now shown in their correct unit (previously only time unit)
* Export: IOMatlab: handling for default ND-filter added (vector in MATLAB files could be empty)
* Export: IOCSV: removed false error message about 'dt'
* Import: ImportDialog: now shows name of LIISettings filename in error message if file was not found
* IOCustom/ImportDialogHelper: fixed import window not closing after successful signal import
* NotificationWindow: fixed auto scrolling bug when new log messages are appended
* MRunSettings: fix for "settings not found" problem occured in some cases
                                  

##### Other changes

* IOMatlab, SignalManager, ExportOverwriteDialog: dialog window is opened if export filename already exists (overwrite or choose another name)
* DatabaseEditor: reminder for 'unsaved changes' added
* DatabaseEditor: reworked error handling
* DatabaseEditor: added generation of unique filenames, dialog is now instanciated with db type to check for name duplicates
* DatabaseEditor, MasterWindow: dialog window opens at exit if database editor contains unsaved changes
* MRunDetails: settings changes are now color-marked, warning at exit added if settings are not saved
* Signal: added functions to calculate min and max value of signal range
* Signal: added functions to calculate time at max/min value
* AnalysisTools - TemperatureFit: moved 'clear plot' button from 'Fit' plot toolbar to planck curve settings



### 3.0.6

##### Libraries
* *Qt framework* updated to 5.10.1
* *Matio Library* updated to 1.5.11
* *zlib* version 1.2.11 added for MATLAB file compression support
                
                                                               
##### Features 
* Export: MATLAB export reworked, new structure with support of parallel export for 
   unprocessed and processed data, standard deviation, temperature calculator 
   metadata, file compression 
* Plugin:Transfer: new plugin for transfering processed signals from raw to 
   absolute processing chain
                
                
##### Bug fixes
* Fixed scaling problems with high DPI displays (http://doc.qt.io/qt-5/highdpi.html)
* Plugin:Baseline: offset from LIISettings was only applied for first channel
* Plugin:MultiSignalAverage: when plugin is added, 'end signal'-field is now set to the highest signal number
* FitCreator: FitRun is removed when associated MRun is removed
* AnalysisTools - TemperatureFit: fixed crash when selecting a datapoint without a valid temperature signal calculated
* AnalysisTools - TemperatureFit: when 'clear plot' button of FitVisualization plot was pressed, additional legend items in FitResults are added
* AnalysisTools - TemperatureFit: for Fit Results plot, 'Show' combobox items were swapped
* AnalysisTools - ParameterAnalysis: multi-signal average signals are only shown once for temperature traces
* SignalPlot: channel description in plot legend showed 'ch' for temperature signals 
* SignalPlot: standard deviation envelope was displayed even if channel was deactivated
* SignalPlot: channel selection is now preserved while the checkboxes are displayed and other MRuns are selected
* SignalPlot: channel checkboxes label changed to 'T#' when signal type is temperature.


##### Other changes
* ExportDialog: selected export type is now saved/restored across restarts
* FitSettings: revamped due to scaling issues on high DPI displays 
* GeneralSettings: default session file name unified to 'iniSession.xml' 
* GeneralSettings: button added to reset splitter states 
* GuiSettings: added functions to load/save/reset splitter states
* SignalPlot: Channel checkbox bahavior improved: channel checkboxes state is preserved if other MRun is selected and if checkboxes are hidden, all channels are shown again.
* MRunDetails: table layout improved
* SignalManager: changed some status messages, so they are constantly displayed during signal processing
* AnalysisTools - ParameterAnalysis: changed data table resize method, resizing is now possible after table update
 
   
   