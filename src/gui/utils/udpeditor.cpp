#include "udpeditor.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

UDPEditorElement::UDPEditorElement(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setMargin(0);

    lineEditIdentifier = new QLineEdit(this);
    lineEditIdentifier->setMinimumWidth(135);
    lineEditIdentifier->setToolTip("Parameter Identifier");
    lineEditIdentifier->setStyleSheet("QLineEdit { margin: 0px }");
    lineEditValue = new QLineEdit(this);
    lineEditValue->setMinimumWidth(135);
    lineEditValue->setToolTip("Parameter Value");
    lineEditValue->setStyleSheet("QLineEdit { margin: 0px }");
    buttonRemove = new QPushButton("X", this);
    buttonRemove->setToolTip("Remove Parameter");
    buttonRemove->setMaximumWidth(20);
    buttonRemove->setMaximumHeight(20);

    mainLayout->addWidget(lineEditIdentifier);
    mainLayout->addWidget(lineEditValue);
    mainLayout->addWidget(buttonRemove);

    setLayout(mainLayout);

    connect(buttonRemove, SIGNAL(clicked(bool)), SLOT(onButtonRemoveClicked()));
}


void UDPEditorElement::onButtonRemoveClicked()
{
    emit removeRequest(this);
}


//---


UDPEditor::UDPEditor(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("User Defined Parameters Editor");

    setMinimumWidth(350);
    setMinimumHeight(400);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    QHBoxLayout *lowerLayout = new QHBoxLayout();

    QWidget *elementsWidget = new QWidget();
    elementsLayout = new QVBoxLayout();
    elementsLayout->setMargin(1);
    elementsLayout->setSizeConstraint(QLayout::SetFixedSize);
    elementsWidget->setLayout(elementsLayout);
    elementsWidget->setMinimumSize(100, 200);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(elementsWidget);
    scrollArea->setStyleSheet("QScrollArea { border: 0px }");

    buttonAddElement = new QPushButton("Add Parameter");
    buttonOk = new QPushButton("OK", this);
    buttonCancel = new QPushButton("Cancel", this);

    QHBoxLayout *buttonAddLayout = new QHBoxLayout();
    buttonAddLayout->addWidget(buttonAddElement);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buttonOk);
    buttonLayout->addWidget(buttonCancel);

    lowerLayout->addLayout(buttonAddLayout);
    lowerLayout->addLayout(buttonLayout);
    lowerLayout->setAlignment(buttonAddLayout, Qt::AlignLeft);
    lowerLayout->setAlignment(buttonLayout, Qt::AlignRight);

    //mainLayout->addLayout(elementsLayout);
    //mainLayout->addWidget(elementsWidget);
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(lowerLayout);
    //mainLayout->setAlignment(elementsWidget, Qt::AlignTop);
    this->setLayout(mainLayout);

    connect(buttonAddElement, SIGNAL(clicked(bool)), SLOT(onButtonAddParameterClicked()));
    connect(buttonOk, SIGNAL(clicked(bool)), SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(reject()));
}


void UDPEditor::setMRun(MRun *mrun)
{
    currentMRun = mrun;
}


void UDPEditor::onElementRemoveRequest(UDPEditorElement *element)
{
    int index = elementsList.indexOf(element);
    if(index != -1)
    {
        delete elementsList.at(index);
        elementsList.removeAt(index);
    }
}


void UDPEditor::onButtonAddParameterClicked()
{
    UDPEditorElement *element = new UDPEditorElement(this);
    elementsList.push_back(element);
    elementsLayout->addWidget(element);
    connect(element, SIGNAL(removeRequest(UDPEditorElement*)), SLOT(onElementRemoveRequest(UDPEditorElement*)));
}


int UDPEditor::exec()
{
    setWindowTitle(QString(currentMRun->name).append(" - User Defined Parameters Editor"));

    while(!elementsList.isEmpty())
    {
        delete elementsList.last();
        elementsList.removeLast();
    }

    for(auto e : currentMRun->userDefinedParameters.keys())
    {
        UDPEditorElement *element = new UDPEditorElement(this);
        element->lineEditIdentifier->setText(e);
        element->lineEditValue->setText(currentMRun->userDefinedParameters.value(e).toString());
        elementsList.push_back(element);
        elementsLayout->addWidget(element);
        connect(element, SIGNAL(removeRequest(UDPEditorElement*)), SLOT(onElementRemoveRequest(UDPEditorElement*)));
    }

    return QDialog::exec();
}


void UDPEditor::accept()
{
    QMap<QString, QVariant> parameters;

    for(int i = 0; i < elementsList.size(); i++)
    {
        if(!elementsList.at(i)->lineEditIdentifier->text().isEmpty())
        {
            parameters.insert(elementsList.at(i)->lineEditIdentifier->text(), elementsList.at(i)->lineEditValue->text());
        }
    }

    currentMRun->userDefinedParameters = parameters;
    currentMRun->setUDPChanged();

    QDialog::accept();
}





