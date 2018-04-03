#ifndef UDPEDITOR_H
#define UDPEDITOR_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include "../../signal/mrun.h"

class UDPEditorElement : public QWidget
{
    Q_OBJECT
public:
    UDPEditorElement(QWidget *parent = 0);

    QLineEdit *lineEditIdentifier;
    QLineEdit *lineEditValue;
    QPushButton *buttonRemove;
signals:
    void removeRequest(UDPEditorElement *element);
private slots:
    void onButtonRemoveClicked();
};

class UDPEditor : public QDialog
{
    Q_OBJECT
public:
    UDPEditor(QWidget *parent = 0);

    void setMRun(MRun *mrun);

private:
    MRun *currentMRun;

    QVBoxLayout *elementsLayout;

    QPushButton *buttonAddElement;
    QPushButton *buttonOk;
    QPushButton *buttonCancel;

    QList<UDPEditorElement*> elementsList;

private slots:
    void onElementRemoveRequest(UDPEditorElement *element);

public slots:
    void onButtonAddParameterClicked();
    virtual int exec();
    virtual void accept();

};

#endif // UDPEDITOR_H
