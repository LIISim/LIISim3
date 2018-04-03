#ifndef SESSIONLOADDIALOG_H
#define SESSIONLOADDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>

/**
 * @brief The SessionLoadDialog class provides a dialog window
 * for loading .xml session files.
 */
class SessionLoadDialog : public QDialog
{
    Q_OBJECT
public:
    SessionLoadDialog(QWidget* parent = 0);


private:

    // ui elements
    QLabel* labelInfo;
    QLabel* labelFname;
    QPushButton* buttonSelectFile;

    QPushButton* buttonCancel;
    QPushButton* buttonLoad;

    QButtonGroup* radioButtons;
    QRadioButton* radioButtonClear;
    QRadioButton* radioButtonAddIgnore;
    QRadioButton* radioButtonAddOverwrite;

    void setDataMode(int mode);
    int dataMode();

public slots:

    virtual void done(int r);

private slots:

    void onButtonSelectReleased();
    void onButtonCancelReleased();
    void onButtonLoadReleased();


};

#endif // SESSIONLOADDIALOG_H
