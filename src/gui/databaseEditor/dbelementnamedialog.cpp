#include "dbelementnamedialog.h"

#include <QLabel>
#include <QPushButton>

#include "core.h"
#include "database/databasecontent.h"

DBElementNameDialog::DBElementNameDialog(const std::type_info &type, QWidget *parent) : QDialog(parent), _type(type)
{
    setModal(true);

    layout = new QVBoxLayout();
    layout->setMargin(0);

    QHBoxLayout *layoutEdit = new QHBoxLayout();
    layoutEdit->setContentsMargins(10, 10, 10, 0);
    QLabel *labelEdit = new QLabel("Name:", this);
    editName = new QLineEdit(this);
    layoutEdit->addWidget(labelEdit);
    layoutEdit->addWidget(editName);

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->setContentsMargins(10, 5, 10, 0);
    QPushButton *buttonOK = new QPushButton("OK", this);
    QPushButton *buttonCancel = new QPushButton("Cancel", this);
    layoutButtons->addStretch(-1);
    layoutButtons->addWidget(buttonOK);
    layoutButtons->addWidget(buttonCancel);

    layout->addLayout(layoutEdit);
    layout->addLayout(layoutButtons);

    setLayout(layout);

    connect(buttonOK, SIGNAL(clicked(bool)), SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(reject()));

    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    QSize size = sizeHint();
    size.setWidth(380);
    resize(size);
}


void DBElementNameDialog::setDefault(const QString &name)
{
    editName->setText(name);
    editName->setSelection(0, name.size());
}


QString DBElementNameDialog::getName()
{
    return editName->text();
}


QString DBElementNameDialog::getFilename()
{
    QString filename = editName->text();
    QRegExp regExp("[^a-zA-Z0-9\-]+", Qt::CaseInsensitive);
    filename.replace(regExp, "_");
    return filename;
}


QString DBElementNameDialog::getUniqueFilename(QString dir, QString extension)
{
    QString file = getFilename() + extension;
    QString location = dir + file;

    int run = 1;

    while(QFile(location).exists())
    {
        file = QString(getFilename() + "_(%0)" + extension).arg(run);
        location = dir + file;
        run++;

        if(run == 100)
            return "";
    }

    qDebug() << "DBElementNameDialog::getUniqueFilename" << file;

    return file;
}


void DBElementNameDialog::accept()
{
    QList<DatabaseContent*> *dbList = nullptr;
    if(_type == typeid(GasProperties))
        dbList = Core::instance()->getDatabaseManager()->getGases();
    else if(_type == typeid(Material))
        dbList = Core::instance()->getDatabaseManager()->getMaterials();
    else if(_type == typeid(GasMixture))
        dbList = Core::instance()->getDatabaseManager()->getGasMixtures();
    else if(_type == typeid(LIISettings))
        dbList = Core::instance()->getDatabaseManager()->getLIISettings();
    else
    {
        qDebug() << "DBElementNameDialog: Database type not defined, rejecting...";
        QDialog::reject();
        return;
    }

    for(int i = 0; i < dbList->size(); i++)
    {
        if(dbList->at(i)->name == editName->text())
        {
            statusBar->showMessage("An entry with this name already exists, please choose another name", 10000);
            return;
        }
    }

    QDialog::accept();
}
