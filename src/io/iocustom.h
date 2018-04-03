#ifndef IOCUSTOM_H
#define IOCUSTOM_H

#include "iobase.h"

/**
 * @brief The IOcustom class
 */
class IOcustom : public IOBase
{
    Q_OBJECT
public:
    explicit IOcustom(QObject *parent = 0);

    // implementation of abstract IOBase methods
    void exportImplementation(const SignalIORequest & rq);

    void checkFiles();

protected:

    // implementation of abstract IOBase methods
    void setupImport();
    void importStep(MRun* mrun, SignalFileInfoList  fileInfos);


private:

    // private helpers
    void scanMainDirectory();
    void scanDirectory(SignalIORequest irq);

    int loadFromCustomFile(MRun* mRun, SignalFileInfo fi);

    QString getFnamePatternRegExp(QString var);

};

#endif // IOCUSTOM_H
