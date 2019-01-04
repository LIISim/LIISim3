#ifndef MEASUREMENTLIST_H
#define MEASUREMENTLIST_H

#include <QTableWidget>

#include "../atoolbase.h"
#include "measurementlisthelper.h"

class MeasurementList : public AToolBase
{
    Q_OBJECT
public:
    MeasurementList(QWidget *parent = 0);

    virtual void handleSignalDataChanged();
    virtual void handleCurrentRunChanged(MRun* run);
    virtual void handleSelectedRunsChanged(const QList<MRun*>& runs);
    virtual void handleSelectedStypeChanged(Signal::SType stype);
    virtual void handleSelectedChannelsChanged(const QList<int> &ch_ids);

    virtual QList<QAction*> toolbarActions();

    virtual void onToolActivation();

private:
    QTableWidget *_table;

    QList<MLElementBase*> _elements;

public slots:
    void onCheckboxElementChanged();
    void copyToClipboard();

};

#endif // MEASUREMENTLIST_H
