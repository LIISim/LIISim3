#ifndef BASELINE_H
#define BASELINE_H

#include "../processingplugin.h"
#include <QList>

/**
 * @brief The Baseline class
 * @ingroup ProcessingPlugin-Implementations
 */
class Baseline : public ProcessingPlugin
{
     Q_OBJECT

    public:
        explicit Baseline(ProcessingChain *parentChain);

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
        void reset();

        QString getParameterPreview();

    private:

        QString operation;
        double startAverage;
        double endAverage;

        /// @brief offset value for each channel
        QList<double> offset;

        /// @brief flag indicating that the average signal has been calculated
        bool offsetAvailable;

    private slots:
        void onOperationChanged();
};

#endif // BASELINE_H
