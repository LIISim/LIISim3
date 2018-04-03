#include "fitrun.h"

#include <QtConcurrent/qtconcurrentrun.h>
#include <QCoreApplication>

#include "../../core.h"
#include "../../calculations/numeric.h"

int FitRun::m_id_count = 0;

FitRun::FitRun(FitMode mode, QObject *parent) : QObject(parent)
{
    m_mode = mode;

    m_id = m_id_count++;
    m_finishedfits = 0;

    m_name = generateName();
    m_creationDate = QDateTime::currentDateTime();

    m_sectionSet = false;
    m_sectionBegin = 0.0f;
    m_sectionEnd = 0.0f;

    // same initial modelingSettings for all fitdata
    // these initial modelingSettings are then duplicated and processed individually
    // (T_gas is located in heat transfer model)
    m_initModelingSettings = new ModelingSettings(this);
    m_initModelingSettings->copyFrom(Core::instance()->modelingSettings);

    m_numericSettings = new NumericSettings(this);
    m_fitSettings = new FitSettings(this);
    m_canceled = false;


    Core::instance()->dataModel()->registerFitRun(this);
    connect(this,SIGNAL(asyncFitFinished(int)),this,SLOT(onAsyncFitFinished(int)),Qt::QueuedConnection);
}


FitRun::~FitRun()
{
   //  Core::instance()->dataModel()->unregisterFitrun(m_id);
}


void FitRun::setFitSettings(FitSettings *fs)
{
    if(m_fitSettings)
        delete m_fitSettings;
    m_fitSettings = fs;
    m_fitSettings->setParent(this);
    emit modified();
}


void FitRun::setNumericSettings(NumericSettings *ns)
{
    if(m_numericSettings)
        delete m_numericSettings;
    m_numericSettings = ns;
    m_numericSettings->setParent(this);
    emit modified();
}


/**
 * @brief FitRun::setModelingSettings INITIAL modelingSettings: these are copied for each fitData[i]
 * @param ms
 */
void FitRun::setModelingSettings(ModelingSettings *ms)
{
    if(m_initModelingSettings)
        delete m_initModelingSettings;
    m_initModelingSettings = ms;
    m_initModelingSettings->setParent(this);
    emit modified();
}


/**
 * @brief FitRun::modelingSettings returns ModelingSettings of FitData[i]
 * @param i fitData index
 */
ModelingSettings* FitRun::modelingSettings(int i)
{
    //qDebug() << "FitRun: ms: " << i;
    if(i == -1)
        return m_initModelingSettings;
    else
        return m_modelingSettingsList.at(i);
}


/**
 * @brief FitRun::setFitData adds FitData to list and copies modelingSettings
 * @param data
 */
void FitRun::setFitData(QList<FitData> &data)
{
    m_fitData = data;
    for(int i = 0; i < m_fitData.size(); i++)
    {
        m_fitData[i].setFitRun(this);

        // copy modelingSettings for this FitData
        m_modelingSettingsList.append(new ModelingSettings(this));
        m_modelingSettingsList[i]->copyFrom(m_initModelingSettings);
    }

    emit modified();
}


int FitRun::calculateProgressStepCount()
{
    // progress is underestimated, because only max number of iterations is known
    // and cannot be determined in advance

    // steps is calculated by number of ODE calls (Numeric::runge_kutta)
    // steps =  * number of fitruns
    //          * number if iterations
    //          * (number of enabled fitparameters +  additional call)
    //          + m_fitData.size() * 6 (for plotting results)

    return m_max_iteration_count
           * (m_fitSettings->getNoEnabledFitParameters() + 1)
           + m_fitData.size() * 6;
}

int FitRun::updatedProgressStepCount(int iterations_passed)
{
    // this function calculates updated progress step count
    // based on passed iterations
    m_max_iteration_count = m_max_iteration_count
                        - (m_numericSettings->iterations() - iterations_passed);
    //qDebug() << "FitRun: updated progress: " << iterations_passed << m_max_iteration_count << calculateProgressStepCount();
    return calculateProgressStepCount();
}


void FitRun::fitAll()
{
    emit fitStarted();

    // ------------------
    //  init progress bar
    // ------------------

    // max iterations = fitDataNumber * maxIterationsPerFitData
    m_max_iteration_count = m_fitData.size() * m_numericSettings->iterations();
    int steps = calculateProgressStepCount();
    Core::instance()->initProgressBar(steps);

    Numeric::canceled = false;
    m_finishedfits = 0;

    for(int i = 0; i < m_fitData.size(); i++)
        //fitAsync(i);
        QtConcurrent::run(this, &FitRun::fitAsync, i);
}


void FitRun::fitAsync(int i)
{
    auto ptr = m_modelingSettingsList[i]->heatTransferModel();

    QString htm_addr;
    htm_addr.sprintf("%p %i\n", ptr, *ptr); // QString("%0 %1").arg(ptr).arg(*ptr);

    qDebug() << "FitRun:fitAsync: " << i
             << m_modelingSettingsList[i]->heatTransferModel()->name
             << m_modelingSettingsList[i]->processPressure()             
             << " - " << m_fitData.size()
             << typeid(m_modelingSettingsList[i]->heatTransferModel()).name()
             << htm_addr;

    Numeric::levmar(FitRun::PSIZE,
                    &m_fitData[i],
                    m_modelingSettingsList[i],
                    m_fitSettings,
                    m_numericSettings);


    emit asyncFitFinished(i);
}


void FitRun::onAsyncFitFinished(int i)
{
    m_finishedfits++;

    //  update progress bar based on passed iterations
    int steps = updatedProgressStepCount(m_fitData.at(i).iterationResCount());
    Core::instance()->updateMaxProgressBarSteps(steps);

    if(m_finishedfits == m_fitData.size())
    {
        m_canceled = Numeric::canceled;
        Numeric::canceled = false;

        emit modified();
        emit fitFinished();

        Core::instance()->finishProgressBar();
    }

    int max_it = m_fitData[i].iterationResCount() - 1;

    qDebug() << "FitRun::onAsyncFitFinished: "
            << m_fitData[i].mrun()->getName() << max_it
            << m_fitData[i].iterationResult(max_it).at(0)
            << m_fitData[i].iterationResult(max_it).at(1)
            << m_fitData[i].iterationResult(max_it).at(2)
            << m_fitData[i].iterationResult(max_it).at(3)
            << m_fitData[i].iterationResult(max_it).at(4)
            << m_fitData[i].iterationResult(max_it).at(5)
            << m_fitData[i].iterationResult(max_it).at(6)
            << m_fitData[i].iterationResult(max_it).at(7);
}


/**
 * @brief FitRun::operator []
 * @param idx
 * @return FitData Reference
 */
FitData& FitRun::operator[](int idx)
{
    return m_fitData[idx];
}


/**
 * @brief FitRun::at
 * @param idx
 * @return FitData
 */
FitData FitRun::at(int idx)
{
    return m_fitData[idx];
}


void FitRun::setSection(double begin, double end)
{
    m_sectionSet = true;
    m_sectionBegin = begin;
    m_sectionEnd = end;
}


bool FitRun::sectionSet()
{
    return m_sectionSet;
}


double FitRun::sectionBegin()
{
    return m_sectionBegin;
}


double FitRun::sectionEnd()
{
    return m_sectionEnd;
}


QString FitRun::toString()
{
    QString res = QString("name: %0:").arg(m_name);
    res.append("\ncreation date: "+m_creationDate.toString("dd.MM.yyyy hh:mm:ss.zzz"));
    res.append("\n"+m_fitSettings->toString());
    res.append("\n"+m_initModelingSettings->toString());
    res.append("\n"+m_numericSettings->toString());
    for(int i = 0; i < m_fitData.size(); i++)
        res.append("\n  "+m_fitData[i].toString());
    return res;
}


void FitRun::writeToXML(QXmlStreamWriter &w)
{
    w.writeStartElement("FitRun");
    w.writeAttribute("name",m_name);
    w.writeAttribute("creationDate",m_creationDate.toString("dd.MM.yyyy hh:mm:ss.zzz"));

    m_initModelingSettings->writeToXML(w);
    m_fitSettings->writeToXML(w);
    m_numericSettings->writeToXML(w);

    for(int i = 0; i < m_fitData.size(); i++)
        m_fitData[i].writeToXML(w);

    w.writeEndElement(); // FitRun
}


void FitRun::readFromXml(QXmlStreamReader &r)
{
    if(r.tokenType() != QXmlStreamReader::StartElement &&  r.name() == "FitRun")
        return;

    QXmlStreamAttributes a = r.attributes();
    m_name = a.value("name").toString();

    m_creationDate = QDateTime::fromString(a.value("creationDate").toString(),"dd.MM.yyyy hh:mm:ss.zzz");
    while( !(r.name() == "FitRun" &&  r.tokenType() == QXmlStreamReader::EndElement))
    {
        if(r.tokenType() == QXmlStreamReader::StartElement)
        {
            m_initModelingSettings->readFromXML(r);
            m_fitSettings->readFromXML(r);
            m_initModelingSettings->readFromXML(r);
            if(r.name() == "FitData")
            {
                FitData fdat;
                bool success = fdat.readFromXml(r);
                if(success)
                    m_fitData << fdat;
            }
        }
        r.readNext();
    }
    qDebug() << "FitRun::readFromXml";
    qDebug() << toString();
    emit modified();
}


/**
 * @brief FitRun::generateName generates unique default name
 * @return
 */
QString FitRun::generateName()
{
    bool unique = false;
    QList<FitRun*> others = Core::instance()->dataModel()->fitRuns();
    int count = others.size()+1;
    QString name = QString("FitRun %0").arg(count);
    while(!unique)
    {
        unique = true;
        for(int i = 0; i < others.size(); i++)
        {
            if(others[i]->name() == name)
            {
                name = QString("FitRun %0").arg(count++);
                unique = false;
            }
        }
    }
    return name;
}


/**
 * @brief FitRun::modeToString converts enum type to string
 * @param mode
 * @return
 */
QString FitRun::modeToString(FitMode mode)
{
    QString res = "none";

    switch(mode)
    {
        case FitMode::TEMP:
            res = "Temperature";
            break;

        case FitMode::PSIZE:
            res = "Particle size";
            break;
    }
    return res;
}
