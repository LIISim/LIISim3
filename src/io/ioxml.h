#ifndef IOXML_H
#define IOXML_H

#include "iobase.h"

#include <QVector>
#include <QList>
#include <QMap>

class ProcessingPluginConnector;

/**
 * @brief The IOxml class extends IOBase and defines the Import/Export of
 * the complete program state within a XML-Session-File
 * @ingroup IO
 * This includes import-information of all loaded mruns as well as
 * custom names,groups and processingchains. The Signaldata will
 * not be stored within the XML-File. Instead only references
 * to the datafiles will be stored (SignalIORequest, SignalFileInfo)
 */
class IOxml : public IOBase
{
    Q_OBJECT
public:
    explicit IOxml(QObject *parent = 0);


    static void loadInitSession();
    static void saveInitSession();

    // implementation of abstract IOBase methods
    void exportImplementation(const SignalIORequest & rq);

    void checkFiles();

protected:

    // implementation of abstract IOBase methods

    void setupImport();

    void importStep(MRun* mrun, SignalFileInfoList  fileInfos);

private:

    int mProgressCounter;
    bool mRelativePaths;
    QString mXMLfname;
    QList<ProcessingPluginConnector*> initGlobalPPCs;

    QMap<int,ProcessingPluginConnector*> readPlugConnectorsMap;

    // private helpers

    void writeGroup(QXmlStreamWriter& w, MRunGroup* g, QList<MRun *>& checkedRuns);
    void readGroup(QXmlStreamReader& r);

    void writeMRun(QXmlStreamWriter& w, MRun* m);
    void readMRun(QXmlStreamReader& r, MRunGroup *parentGroup);

    void writeProcessingChain(QXmlStreamWriter& w, ProcessingChain* p);
    void readProcessingChain(QXmlStreamReader& r, MRun *parentRun);

    void readProcessingPlugin(QXmlStreamReader& r, ProcessingChain *parentChain);

    QList<ProcessingPluginConnector*> findGlobalPPCs();
    void deleteAllProcessingSteps();
    void deleteAllRuns();

signals:

    void m_MrunLoadingStepFinished(MRun* mrun);
    void m_importSetupFinished();

private slots:

    void m_onImportSetupFinished();
    void m_onMrunLoadingStepFinished(MRun* mrun);


};

#endif // IOXML_H
