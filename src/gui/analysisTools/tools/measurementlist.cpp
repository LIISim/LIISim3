#include "measurementlist.h"

#include <limits>

#include <QHeaderView>
#include <QShortcut>
#include <QClipboard>

#include "core.h"

MeasurementList::MeasurementList(QWidget *parent) : AToolBase(parent)
{
    setObjectName("AT_ML");
    m_title = "Measurement List";
    m_iconLocation = Core::rootDir + "resources/icons/table.png";

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    QHBoxLayout *parameterLayout = new QHBoxLayout;
    parameterLayout->setContentsMargins(5, 10, 5, 5);
    parameterLayout->addWidget(new QLabel("Show:", this));

    _elements.push_back(new MLEName(this));
    _elements.push_back(new MLEDirectory(this));
    _elements.push_back(new MLEFiles(this));
    _elements.push_back(new MLELaserFluence(this));
    _elements.push_back(new MLEPmtGainCH1(this));
    _elements.push_back(new MLEPmtGainCH2(this));
    _elements.push_back(new MLEPmtGainCH3(this));
    _elements.push_back(new MLEPmtGainCH4(this));
    _elements.push_back(new MLELIISettings(this));
    _elements.push_back(new MLENDFilter(this));
    _elements.push_back(new MLEGroup(this));

    for(int i = 0; i < _elements.size(); i++)
        parameterLayout->addWidget(_elements.at(i)->_checkbox);
    parameterLayout->addStretch(-1);
    mainLayout->addLayout(parameterLayout);

    _table = new QTableWidget(this);

    QStringList headerLabels;
    for(int i = 0; i < _elements.size(); i++)
        if(_elements.at(i)->enabled())
            headerLabels << _elements.at(i)->_name;

    _table->setColumnCount(headerLabels.size());
    _table->setHorizontalHeaderLabels(headerLabels);

    for(int i = 0; i < headerLabels.size(); i++)
        _table->setColumnWidth(i, 200);

    mainLayout->addWidget(_table);

    for(int i = 0; i < _elements.size(); i++)
        connect(_elements.at(i)->_checkbox, SIGNAL(stateChanged(int)), SLOT(onCheckboxElementChanged()));

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(shortcut, SIGNAL(activated()), SLOT(copyToClipboard()));
}


void MeasurementList::handleSignalDataChanged()
{

}


void MeasurementList::handleCurrentRunChanged(MRun* run)
{

}


void MeasurementList::handleSelectedRunsChanged(const QList<MRun*>& runs)
{
    QStringList headerLabels;

    for(int i = 0; i < _elements.size(); i++)
        if(_elements.at(i)->enabled())
            headerLabels << _elements.at(i)->_name;

    _table->clear();

    _table->setColumnCount(headerLabels.size());
    _table->setRowCount(runs.size());
    _table->setHorizontalHeaderLabels(headerLabels);

    for(int i = 0; i < runs.size(); i++)
    {
        int column = 0;

        QTableWidgetItem *item;
        for(int j = 0; j < _elements.size(); j++)
        {
            if(_elements.at(j)->enabled())
            {
                item = new QTableWidgetItem(_elements.at(j)->getMeasurement(runs.at(i)));
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                _table->setItem(i, column, item);
                _table->setColumnWidth(column, _elements.at(j)->_columnWidth);
                column++;
            }
        }
    }
}


void MeasurementList::handleSelectedStypeChanged(Signal::SType stype)
{

}


void MeasurementList::handleSelectedChannelsChanged(const QList<int>& ch_ids)
{

}


QList<QAction*> MeasurementList::toolbarActions()
{
    return QList<QAction*>();
}


void MeasurementList::onToolActivation()
{
    handleSelectedRunsChanged(selectedRuns());
}


void MeasurementList::onCheckboxElementChanged()
{
    handleSelectedRunsChanged(selectedRuns());
}


void MeasurementList::copyToClipboard()
{
    QList<QTableWidgetItem*> items = _table->selectedItems();
    QString data;

    int minr = std::numeric_limits<int>::max();
    int maxr = 0;
    int minc = std::numeric_limits<int>::max();
    int maxc = 0;

    // first, get row/column index range
    for(int i = 0; i < items.size(); i++)
    {
        int r =  items.at(i)->row();
        int c =  items.at(i)->column();

        if( r > maxr ) maxr = r;
        if( c > maxc ) maxc = c;
        if( r < minr ) minr = r;
        if( c < minc ) minc = c;
    }

    // print data line per line to string
    for(int r = minr; r <= maxr; r++)
    {
        for(int c = minc; c <= maxc; c++)
        {
            data.append(_table->item(r,c)->text());
            // separate values in cols
            if(c < maxc)
                data.append("\t");
        }
        if(r < maxr)
            data.append("\n");
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->clear();
    clipboard->setText(data);
}
