#include "da_userdefinedparameters.h"

UserDefinedParameterWidget::UserDefinedParameterWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout();

    checkbox = new QCheckBox(this);
    checkbox->setChecked(true);
    checkbox->setToolTip("Check if parameter should be included in next MRun");
    lineEditIdentifier = new QLineEdit(this);
    lineEditIdentifier->setToolTip("Parameter name");
    lineEditIdentifier->setText("Name");
    lineEditValue = new QLineEdit(this);
    lineEditValue->setToolTip("Parameter value");
    lineEditValue->setText("Value");
    buttonRemove = new QToolButton(this);
    buttonRemove->setText("X");
    buttonRemove->setToolTip("Remove parameter");
    buttonRemove->setMaximumHeight(17);
    buttonRemove->setMaximumWidth(14);

    layout->addWidget(checkbox);
    layout->addWidget(lineEditIdentifier);
    layout->addWidget(lineEditValue);
    layout->addWidget(buttonRemove);
    layout->setMargin(0);

    setLayout(layout);

    connect(buttonRemove, SIGNAL(clicked(bool)), SLOT(onButtonRemoveClicked()));

    connect(lineEditIdentifier, SIGNAL(textEdited(QString)), SLOT(onChanges()));
    connect(lineEditValue, SIGNAL(textEdited(QString)), SLOT(onChanges()));
}

bool UserDefinedParameterWidget::isValid()
{
    return checkbox->isChecked() && (!lineEditIdentifier->text().isEmpty() || !lineEditValue->text().isEmpty());
}

QString UserDefinedParameterWidget::getIdentifier()
{
    return lineEditIdentifier->text();
}

QString UserDefinedParameterWidget::getValue()
{
    return lineEditValue->text();
}


void UserDefinedParameterWidget::setIdentifier(QString identifier)
{
    lineEditIdentifier->blockSignals(true);
    lineEditIdentifier->setText(identifier);
    lineEditIdentifier->blockSignals(false);
}


void UserDefinedParameterWidget::setValue(QString value)
{
    lineEditValue->blockSignals(true);
    lineEditValue->setText(value);
    lineEditValue->blockSignals(false);
}


void UserDefinedParameterWidget::onButtonRemoveClicked()
{
    emit removeClicked(this);
}

void UserDefinedParameterWidget::onChanges()
{
    emit valuesChanged();
}

//---

UserDefinedParameterMasterWidget::UserDefinedParameterMasterWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout();
    buttonAdd = new QToolButton(this);
    buttonAdd->setText("Add");
    buttonClear = new QToolButton(this);
    buttonClear->setText("Clear");
    buttonClear->setToolTip("Remove all user defined parameters");
    layout->addWidget(new QLabel("", this));
    layout->addWidget(buttonAdd);
    layout->addWidget(buttonClear);
    layout->setMargin(0);
    this->setLayout(layout);
}
