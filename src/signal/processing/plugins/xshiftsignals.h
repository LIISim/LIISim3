#ifndef XSHIFTSIGNALS_H
#define XSHIFTSIGNALS_H

#include "../../mrun.h"
#include "../../signal.h"
#include "../processingplugin.h"
#include <QList>


/**
 * @brief The XShiftSignals class
 * @ingroup ProcessingPlugin-Implementations
 */
class XShiftSignals : public ProcessingPlugin
{
     Q_OBJECT

public:
    explicit XShiftSignals( ProcessingChain *parentChain);

    static QString descriptionFileName;
    static QString iconFileName;
    static QString pluginName;

    static QList<Signal::SType> supportedSignalTypes;

    // implementations, overrides of virtual base class functions
    QString getName();
    bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
    void setFromInputs();
    void reset();

    QString getParameterPreview();

private:

    // mpIdx -> channel -> Signal
    std::map<int, std::vector<Signal>> shiftSignals;

};

#endif // XSHIFTSIGNALS_H
