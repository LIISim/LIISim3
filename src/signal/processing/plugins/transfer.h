#ifndef TRANSFER_H
#define TRANSFER_H

#include "../processingplugin.h"
#include <QList>

class MRun;

class Transfer : public ProcessingPlugin
{
    Q_OBJECT

public:
    explicit Transfer(ProcessingChain *parentChain);

    static QString pluginName;

    static QList<Signal::SType> supportedSignalTypes;

    QString getName();

    bool processSignalImplementation(const Signal &in, Signal &out, int mpIdx);
    void setFromInputs();

    QString getParameterPreview();

private:
    Signal::SType destinationSType;


};

#endif // TRANSFER_H
