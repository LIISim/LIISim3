#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "../processingplugin.h"
#include <QList>
class MRun;

/**
 * @brief The Convolution class
 * @ingroup ProcessingPlugin-Implementations
 */
class Convolution : public ProcessingPlugin
{
    Q_OBJECT

    public:
        explicit Convolution(ProcessingChain *parentChain);

        /** @brief name of the description .hml file
          * @dtails the description .html file shoud be located
          * in the resources/pluginData/ directory (see PluginManager )*/
        static QString descriptionFileName;

        /** @brief name of icon file (for PluginManager)
          * @dtails the icon image file shoud be located
          * in the resources/pluginData/ directory (see PluginManager )*/
        static  QString iconFileName;
        static  QString pluginName;

        static QList<Signal::SType> supportedSignalTypes;

        // implementations, overrides of virtual base class functions
        QString getName();
        bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
        void setFromInputs();

        QString getParameterPreview();

    private:

        QString chId;
        double calcvalue;
};

#endif // CONVOLUTION_H
