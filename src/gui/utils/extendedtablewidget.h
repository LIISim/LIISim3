#ifndef EXTENDEDTABLEWIDGET_H
#define EXTENDEDTABLEWIDGET_H

#include <QTableWidget>
#include <QStringList>

class ExtendedTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    ExtendedTableWidget(QWidget *parent = 0);
    ~ExtendedTableWidget();

    void setData(const QStringList &headerLabels, const QList<QList<QString>> &data);

    void setAllRowHeight(int height);

    void setEditable(bool editable);

private:
    int allRowHeight;
    bool mEditable;

private slots:
    void copyToClipboard();
};

#endif // EXTENDEDTABLEWIDGET_H
