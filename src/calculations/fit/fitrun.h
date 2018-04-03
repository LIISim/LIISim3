#ifndef FITRUN_H
#define FITRUN_H

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include "../../settings/fitsettings.h"
#include "../../settings/numericsettings.h"
#include "../../settings/modelingsettings.h"

#include "fitdata.h"

/**
 * @brief The FitRun class
 */
class FitRun : public QObject
{
    Q_OBJECT
public:
    /** @brief The FitMode enum  Temperature (Planck) or Particle size   */
    enum FitMode {TEMP, TEMP_CAL, PSIZE};

    explicit FitRun(FitMode mode = FitMode::TEMP, QObject *parent = 0);
    ~FitRun();

    inline QString name(){return m_name;}


    static QString modeToString(FitMode mode);

    inline FitMode mode(){return m_mode;}


    inline NumericSettings* numericSettings(){return m_numericSettings;}
    inline FitSettings* fitSettings(){return m_fitSettings;}
    inline QDateTime creationDate(){return m_creationDate;}
    inline bool canceled(){return m_canceled;}

    void setFitSettings(FitSettings* fs);
    void setNumericSettings(NumericSettings* ns);
    void setModelingSettings(ModelingSettings* ms);
    ModelingSettings* modelingSettings(int i = -1);

    /**
    * @brief id unique id of FitRun
    * @return id of this item
    */
    inline int id() const {return m_id;}


    QString toString();

    void writeToXML(QXmlStreamWriter &w);
    void readFromXml(QXmlStreamReader &r);

    void setFitData(QList<FitData> & data);

    int calculateProgressStepCount();
    int updatedProgressStepCount(int iterations_passed);

    void fitAll();


    /** @brief returns the FitData count of this run
        (number of fitted signals) **/
    inline int count(){return m_fitData.size();}

    FitData& operator[](int idx);
    FitData at(int idx);

    void setSection(double begin, double end);

    bool sectionSet();
    double sectionBegin();
    double sectionEnd();

private:

    FitMode m_mode;
    QList<FitData> m_fitData;
    QList<ModelingSettings*> m_modelingSettingsList;

    QString m_name;
    QDateTime m_creationDate;

    ModelingSettings* m_initModelingSettings;
    NumericSettings* m_numericSettings;
    FitSettings* m_fitSettings;

    bool m_canceled;

    /** @brief global counter for id generation */
    static int m_id_count;

    /** @brief unique fitrun id */
    int m_id;
    int m_finishedfits;
    int m_max_iteration_count;

    QString generateName();

    void fitAsync(int i);

    bool m_sectionSet;
    double m_sectionBegin;
    double m_sectionEnd;

signals:
    void fitStarted();
    void fitFinished();
    void asyncFitFinished(int i);
    void modified();

public slots:

private slots:
    void onAsyncFitFinished(int i);
};

#endif // FITRUN_H
