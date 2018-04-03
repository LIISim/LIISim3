#include "extendedtablewidget.h"

#include <QShortcut>
#include <QClipboard>
#include <QApplication>

#include "core.h"

ExtendedTableWidget::ExtendedTableWidget(QWidget *parent) : QTableWidget(parent)
{
    mEditable = true;

    allRowHeight = 18;

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(shortcut, SIGNAL(activated()), SLOT(copyToClipboard()));
}

ExtendedTableWidget::~ExtendedTableWidget()
{

}


void ExtendedTableWidget::setData(const QStringList &headerLabels, const QList<QList<QString>> &data)
{
    clearContents();

    if(data.isEmpty())
    {
        setColumnCount(0);
        setRowCount(0);
        return;
    }

    int columnCount = 0;

    if(headerLabels.size() < data.size())
        columnCount = data.size();
    else
        columnCount = headerLabels.size();

    int rowCount = 0;

    for(int i = 0; i < data.size(); i++)
        if(rowCount < data.at(i).size())
            rowCount = data.at(i).size();

    setColumnCount(columnCount);
    setRowCount(rowCount);
    setHorizontalHeaderLabels(headerLabels);

    for(int i = 0; i < rowCount; i++)
        setRowHeight(i, allRowHeight);

    for(int i = 0; i < data.size(); i++)
    {
        for(int j = 0; j < data.at(i).size(); j++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(data.at(i).at(j));
            if(!mEditable)
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            setItem(j, i, item);
        }
    }
}


void ExtendedTableWidget::setAllRowHeight(int height)
{
    allRowHeight = height;
}


void ExtendedTableWidget::setEditable(bool editable)
{
    mEditable = editable;
}


void ExtendedTableWidget::copyToClipboard()
{
    QList<QTableWidgetItem*> items = selectedItems();

    QString data;

    int minr = 1E10;
    int maxr = 0;
    int minc = 1E10;
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
            data.append(item(r,c)->text());
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
