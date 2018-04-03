#ifndef GETSIGNALSECTION_H
#define GETSIGNALSECTION_H

#include "../../mrun.h"
#include "../processingplugin.h"
#include <QList>

/**
 * @brief The GetSignalSection class
 * @ingroup ProcessingPlugin-Implementations
 */
class GetSignalSection : public ProcessingPlugin
{    
    Q_OBJECT

    public:
        explicit GetSignalSection( ProcessingChain *parentChain);

        static QString descriptionFileName;
        static QString iconFileName;
        static QString pluginName;

        static QList<Signal::SType> supportedSignalTypes;

        // implementations, overrides of virtual base class functions
        QString getName();
        bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
        void setFromInputs();

        QString getParameterPreview();

    private:

        /** @brief defines start and end of selected signal */
        bool get_decay;
        bool reset_time;

        double signal_start;
        double signal_end;
};

#endif // GETSIGNALSECTION_H
