

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
 
   
   