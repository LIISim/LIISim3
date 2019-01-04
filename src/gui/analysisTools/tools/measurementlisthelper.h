#ifndef MEASUREMENTLISTHELPER_H
#define MEASUREMENTLISTHELPER_H

#include <QCheckBox>
#include <QFileInfo>

#include "signal/mrun.h"
#include "signal/mrungroup.h"

#define MEASUREMENTLISTHELPER_GAIN_COLUMN_SIZE 100

class MLElementBase
{
public:
    MLElementBase(QString name, QWidget *parent = 0)
    {
        _name = name;
        _checkbox = new QCheckBox(name, parent);
        _checkbox->setChecked(true);
        _columnWidth = 200;
    }

    virtual QString getMeasurement(MRun *run) = 0;

    bool enabled()
    {
        return _checkbox->isChecked();
    }

    QString _name;
    QCheckBox *_checkbox;
    int _columnWidth;
};


class MLEName : public MLElementBase
{
public:
    MLEName(QWidget *parent = 0) : MLElementBase("Run Name", parent)
    {

    }

    QString getMeasurement(MRun *run)
    {
        return run->name;
    }
};


class MLEDirectory : public MLElementBase
{
public:
    MLEDirectory(QWidget *parent = 0) : MLElementBase("Directory", parent)
    {

    }

    QString getMeasurement(MRun *run)
    {
        QString dir;
        if(!run->importRequest().flist.isEmpty())
        {
            QFileInfo fi(run->importRequest().flist.first().filename);
            dir = fi.absolutePath();
        }
        return dir;
    }
};


class MLEFiles : public MLElementBase
{
public:
    MLEFiles(QWidget *parent = 0) : MLElementBase("File(s)", parent)
    {
        _checkbox->setChecked(false);
    }

    QString getMeasurement(MRun *run)
    {
        QString files;
        for(int i = 0; i < run->importRequest().flist.size(); i++)
        {
            if(!files.isEmpty())
                files.append(" \n");
            files.append(run->importRequest().flist.at(i).filename.split("/").last());
        }
        return files;
    }
};


class MLELaserFluence : public MLElementBase
{
public:
    MLELaserFluence(QWidget *parent = 0) : MLElementBase("Laser Fluence", parent)
    {
        _columnWidth = 100;
    }

    QString getMeasurement(MRun *run)
    {
        return QString::number(run->laserFluence());
    }
};


class MLEPmtGainCH1 : public MLElementBase
{
public:
    MLEPmtGainCH1(QWidget *parent = 0) : MLElementBase("PMT Gain (Ch 1)", parent)
    {
        _columnWidth = MEASUREMENTLISTHELPER_GAIN_COLUMN_SIZE;
    }

    QString getMeasurement(MRun *run)
    {
        QString pmtgain;
        if(run->channelIDs(Signal::RAW).contains(1))
            pmtgain = QString::number(run->pmtGainVoltage(1));
        return pmtgain;
    }
};


class MLEPmtGainCH2 : public MLElementBase
{
public:
    MLEPmtGainCH2(QWidget *parent = 0) : MLElementBase("PMT Gain (Ch 2)", parent)
    {
        _columnWidth = MEASUREMENTLISTHELPER_GAIN_COLUMN_SIZE;
    }

    QString getMeasurement(MRun *run)
    {
        QString pmtgain;
        if(run->channelIDs(Signal::RAW).contains(2))
            pmtgain = QString::number(run->pmtGainVoltage(2));
        return pmtgain;
    }
};


class MLEPmtGainCH3 : public MLElementBase
{
public:
    MLEPmtGainCH3(QWidget *parent = 0) : MLElementBase("PMT Gain (Ch 3)", parent)
    {
        _columnWidth = MEASUREMENTLISTHELPER_GAIN_COLUMN_SIZE;
    }

    QString getMeasurement(MRun *run)
    {
        QString pmtgain;
        if(run->channelIDs(Signal::RAW).contains(3))
            pmtgain = QString::number(run->pmtGainVoltage(3));
        return pmtgain;
    }
};


class MLEPmtGainCH4 : public MLElementBase
{
public:
    MLEPmtGainCH4(QWidget *parent = 0) : MLElementBase("PMT Gain (Ch 4)", parent)
    {
        _columnWidth = MEASUREMENTLISTHELPER_GAIN_COLUMN_SIZE;
    }

    QString getMeasurement(MRun *run)
    {
        QString pmtgain;
        if(run->channelIDs(Signal::RAW).contains(4))
            pmtgain = QString::number(run->pmtGainVoltage(4));
        return pmtgain;
    }
};


class MLELIISettings : public MLElementBase
{
public:
    MLELIISettings(QWidget *parent = 0) : MLElementBase("LII Settings", parent)
    {

    }

    QString getMeasurement(MRun *run)
    {
        return run->liiSettings().name;
    }
};


class MLENDFilter : public MLElementBase
{
public:
    MLENDFilter(QWidget *parent = 0) : MLElementBase("ND-Filter", parent)
    {

    }

    QString getMeasurement(MRun *run)
    {
        return run->filter().identifier;
    }
};


class MLEGroup : public MLElementBase
{
public:
    MLEGroup(QWidget *parent = 0) : MLElementBase("Group", parent)
    {

    }

    QString getMeasurement(MRun *run)
    {
        return run->group()->title();
    }
};

#endif // MEASUREMENTLISTHELPER_H
