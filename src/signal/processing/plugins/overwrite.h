#ifndef OVERWRITE_H
#define OVERWRITE_H

#include "../processingplugin.h"
#include <QList>
class MRun;


/**
 * @brief The Overwrite class
 * @ingroup ProcessingPlugin-Implementations
 */
class Overwrite : public ProcessingPlugin
{
    Q_OBJECT

    public:
        explicit Overwrite(ProcessingChain *parentChain);

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

        QString source;
        QString chId_A;
        QString chId_B;
};

#endif // OVERWRITE_H
