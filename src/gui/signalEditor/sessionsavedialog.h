#ifndef SESSIONSAVEDIALOG_H
#define SESSIONSAVEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include "../dataItemViews/treeview/dataitemtreeview.h"


/**
 * @brief The SessionSaveDialog class provides a dialog window
 * for saving .xml session files. It allows the user to specify
 * a selection of runs/groups.
 */
class SessionSaveDialog : public QDialog
{
    Q_OBJECT
public:
    SessionSaveDialog(QWidget *parent = 0);

private:

    // ui elements
    QLabel* labelInfo;
    QLineEdit* lineEditFilename;
    QPushButton* buttonSelectFile;

    DataItemTreeView* dataTree;
    QCheckBox* checkboxRelative;
    QPushButton* buttonCancel;
    QPushButton* buttonSave;

    QList<QVariant> getCheckedRunIDs();

    bool fileSelected;

public slots:

    virtual void done(int r);

private slots:


    void onButtonSelectReleased();
    void onButtonCancelReleased();
    void onButtonSaveReleased();


};

#endif // SESSIONSAVEDIALOG_H
