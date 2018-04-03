#ifndef TEMPERATURECALCULATOR_H
#define TEMPERATURECALCULATOR_H

#include "../processingplugin.h"

class Core;
class MRun;
class ProcessingChain;

/**
 * @brief The TemperatureCalculator class allows to Calculate temperature signals
 * form given signal data.
 * @ingroup ProcessingPlugin-Implementations
 * @details Therefore this plugins needs acces to the LIISim Core and the
 * MRun the plugin's procesing chain is apllied to. Those properties make this plugin
 * unique and not suitable for general signal processing. But using the ProcessingPlugin
 * architecture allows to perform calculations parallelized among other signal processing
 * tasks.  An instance of this plugin is managed by the TemperatureView Class, we do not
 * allow the user to create instances of this plugin manually!
 */
class TemperatureCalculator : public ProcessingPlugin
{
    Q_OBJECT

    public:

        explicit TemperatureCalculator(ProcessingChain *parentChain);
        virtual ~TemperatureCalculator();

        /** @brief name of the description .hml file
          * @dtails the description .html file shoud be located
          * in the resources/pluginData/ directory (see PluginManager )*/
        static QString descriptionFileName;

        /** @brief name of icon file (for PluginManager)
          * @dtails the icon image file shoud be located
          * in the resources/pluginData/ directory (see PluginManager )*/
        static  QString iconFileName;


        /** @brief name of processing plugin */
        static  QString pluginName;


        /** @brief a list of signal types the plugin supports */
        static QList<Signal::SType> supportedSignalTypes;

        // implementations, overrides of virtual base class functions
        QString getName();
        bool processSignalImplementation(const Signal & in, Signal & out, int mpIdx);
        void setFromInputs();

        void reset();
        void setMRun(MRun* mrun);

        // moved to Plugin: inline bool processingError() { return mProcessingError; }

        QString getParameterPreview();

        void onAddedToPchain();

        int temperatureChannelID();
        void setTemperatureChannelID(int tchid);

        static QList<int> temperatureChannelIDs();
        static int generateTemperatureChannelID();

        Signal::SType getInputSignalType();

    private:

        QString method;
        int chId1;
        int chId2;
        int chId_copySignals;

        QString sourceEm;

        bool bpIntegration;
        bool weighting;

        int iter;
        double startT;
        double startC;
        bool autoStartC;

        int iter_Test;
        double startT_Test;
        double startC_Test;

        QList<bool> *activeChannels;

        int m_tempChannelID;

        QString selected_material;
        Signal::SType inputSignalType;
        QString inputType;

        bool msaMode;
        bool msaSearched;
        Signal msaTemp;

        /** @brief mProcessingError tracks errors for each temperature channel */
        // moved to Plugin: bool mProcessingError;

        ProcessingChain* m_sourcePchain;

        /** @brief counter map temperature channel IDs
            key: temperature channel id
            value: number of TemperatureCalculators using the channel id*/
        static QMap<int,int> globalTemperatureChannelIDmap;

    signals:

    public slots:

    private slots:

        void onMethodChanged();
        void onMaterialChanged();
        void onMRunChanged();
        void onLIISettingsChanged();
        void updateMaterialBox();
        void updateMRunMetadata();
};

#endif // TEMPERATURECALCULATOR_H
