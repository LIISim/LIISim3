#ifndef SAVITZKYGOLAY_H
#define SAVITZKYGOLAY_H

#include "../processingplugin.h"

/**
 * @brief The SavitzkyGolay class https://en.wikipedia.org/wiki/Savitzky%E2%80%93Golay_filter
 */
class SavitzkyGolay : public ProcessingPlugin
{
    Q_OBJECT

    public:

        explicit SavitzkyGolay(ProcessingChain *parentChain);

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


};

#endif // SAVITZKYGOLAY_H
