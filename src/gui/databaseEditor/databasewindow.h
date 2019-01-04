#ifndef DATABASEWINDOW_H
#define DATABASEWINDOW_H

#include <QWidget>
#include <QToolBar>
#include <QSplitter>

#include "databaseeditor.h"

class DatabaseWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DatabaseWindow(QWidget *parent = 0);

    QToolBar* ribbonToolbar(){return m_ribbonToolbar;}

    DatabaseEditor *getDatabaseEditor()
    {
        return dbEdit;
    }

private:
    DatabaseEditor* dbEdit;

    QToolBar* m_ribbonToolbar;

    QGridLayout* grid;
    QSplitter* splith0;
    QSplitter* splitv1r;

signals:

public slots:
};

#endif // DATABASEWINDOW_H
