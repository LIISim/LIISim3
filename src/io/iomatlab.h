#ifndef IOMATLAB_H
#define IOMATLAB_H

#include "iobase.h"

#include "matio.h"

/**
 * @brief The IOmatlab class enables the import and export of the MRun data structure
 * to the Matalb (.mat) file format. Therefore it utilizes the MatIO C-Library (can
 * be found in 'io/matio/', also see http://sourceforge.net/projects/matio/).
 *
 * Structure of .mat output file:
 * The output file contains structured array containing the following fields:
 *
 * file.mat :
 * - measurement_run :  measurement run data struct
 *   -- name :          name of measurement run
 *   -- description :   description of measurement run
 *   -- nChannels :     number of channels
 *   -- nSignals :      number of measurement points
 *
 *   -- raw_signal (nSignals x nChannels): data struct containing  raw signals
 *      --- signal_type :   type of signal
 *      --- channel_id  :   channel id of signal
 *      --- size        :   number of datapoints
 *      --- dt          :   delta time in seconds
 *      --- start_time  :   start time in seconts
 *      --- data        :   vector of datapoints
 *
 *  -- abs_signal (nSignals x nChannels): data struct containing  abolute signals
 *      --- signal_type :   type of signal
 *      --- channel_id  :   channel id of signal
 *      --- size        :   number of datapoints
 *      --- dt          :   delta time in seconds
 *      --- start_time  :   start time in seconts
 *      --- data        :   vector of datapoints
 *
 *  -- temperature_signal (nSignals x nChannels): data struct containing  temperature signals
 *      --- signal_type :   type of signal
 *      --- channel_id  :   channel id of signal
 *      --- size        :   number of datapoints
 *      --- dt          :   delta time in seconds
 *      --- start_time  :   start time in seconts
 *      --- data        :   vector of datapoints
 *
 */
class IOmatlab : public IOBase
{
    Q_OBJECT
public:
    explicit IOmatlab(QObject *parent = 0);

    // implementation of abstract IOBase methods
    void exportImplementation(const SignalIORequest & rq);

    void checkFiles();

protected:

    // implementation of abstract IOBase methods

    void setupImport();

    void importStep(MRun* mrun, SignalFileInfoList  fileInfos);

signals:
    void fileExists(QFileInfo &finfo, int &ret);

public slots:


private:
    /// @brief message prefix
    QString msgprfx;

    bool saveRun(const QString & dirpath, MRun* run);

    matvar_t* getMRunVar(MRun* run);

    matvar_t* getSignalDataMatrix(MRun* run, Signal::SType stype);

    matvar_t* getSignalFieldVar(const Signal & s, int fieldIndex);

    matvar_t* getSettingsStruct(MRun* run);

    matvar_t* getSignalTypeStruct(MRun *run, Signal::SType stype, bool unprocessed, bool processed, bool stdev);
    matvar_t* getSignalStruct(MRun *run, Signal::SType stype, bool processed, int signalID, int channelID, bool stdev);
    matvar_t* getTempSignalStruct(MRun *run, int signalID, int channelID, bool processed, bool stdev);
};

#endif // IOMATLAB_H
