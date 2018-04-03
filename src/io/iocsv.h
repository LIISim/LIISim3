#ifndef IOCSV_H
#define IOCSV_H

#include "iobase.h"

/**
 * @brief The IOcsv class implements the IOBase class for
 * the CSV and CSV_SCAN io-types.
 */
class IOcsv : public IOBase
{
    Q_OBJECT

public:
    explicit IOcsv(QObject *parent = 0);

    // implementation of abstract IOBase methods
    void exportImplementation(const SignalIORequest & rq);

    void checkFiles();

protected:

    // implementation of abstract IOBase methods
    void setupImport();
    void importStep(MRun* mrun, SignalFileInfoList  fileInfos);

private:

    // private helpers
    void scanDirectory();
    int loadFile(MRun* mRun, const SignalFileInfo & fi);

    void writeToCSV(QString fname,MRun* m, Signal::SType, int ChID);

    void writeStdevToCSV(QString fname, MRun *run, Signal::SType, int channelID);

};

#endif // IOCSV_H
