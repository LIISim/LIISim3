#include "dbeditorwidget.h"
#include "../../database/databasemanager.h"
#include <QFile>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

DbEditorWidget::DbEditorWidget( QWidget *parent) : QWidget(parent), _unsavedChanges(false)
{
    currentIndex = 0;
    dbm = NULL;
    db_data = NULL;
    defaultFileName = "";
    defaultName = "";

    lay_main = new QHBoxLayout;
    setLayout(lay_main);

    // LEFT BOX
    lay_left_box = new QVBoxLayout;
    lay_main->addLayout(lay_left_box);

    listView = new QListView;
    listView->setMaximumWidth(140);
    lay_left_box->addWidget(listView);

    listModel = new QStringListModel(this);

    listView->setModel(listModel);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(listView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(onSelectionChanged(QItemSelection)));

    lay_list_buttons = new QHBoxLayout;
    lay_left_box->addLayout(lay_list_buttons);
    butAdd = new QPushButton(tr("Add"));
    butRemove = new QPushButton(tr("Remove"));
    butAdd->setMaximumWidth(60);
    butRemove->setMaximumWidth(60);
    lay_list_buttons->addWidget(butAdd);
    lay_list_buttons->addWidget(butRemove);

    connect(butAdd,SIGNAL(released()),this,SLOT(onAddItemToList()));
    connect(butRemove,SIGNAL(released()),this,SLOT(onRemoveCurrentSelectionFromList()));

    // RIGHT BOX
    lay_right_box = new QVBoxLayout;
    lay_main->addLayout(lay_right_box);
    lay_props = new QGridLayout;
    lay_right_box->addLayout(lay_props);

    lbName = new QLabel("Name: ");
    leName = new QLineEdit();

    lbFname = new QLabel("File: ");
    leFname = new QLineEdit;

    lbDescr = new QLabel("Description: ");
    leDescr = new QLineEdit();


    bOpen = new QPushButton(tr("Open file"));
    //bOpen->setMaximumWidth(100);

    labelUnsavedChanges = new QLabel("Unsaved changes", this);
    labelUnsavedChanges->setStyleSheet("QLabel { color : red }");
    labelUnsavedChanges->setVisible(false);

    bApply = new QPushButton(tr("Apply changes"));
    bApply->setMaximumWidth(100);
    bApply->setEnabled(false);

    lay_props->addWidget(lbName,0,0);
    lay_props->addWidget(leName,0,1);
    lay_props->addWidget(lbFname,0,2, 1,1,Qt::AlignRight);
    lay_props->addWidget(leFname,0,3);
    lay_props->addWidget(bOpen,0,4);

    lay_props->addWidget(lbDescr,1,0, 1, 1);
    lay_props->addWidget(leDescr,1,1, 1, 3);

    QHBoxLayout *layoutSaveChanges = new QHBoxLayout;
    layoutSaveChanges->addWidget(labelUnsavedChanges);
    layoutSaveChanges->addWidget(bApply);

    //lay_props->addWidget(labelUnsavedChanges, 1, 4);
    //lay_props->addWidget(bApply,1,5);

    lay_props->addLayout(layoutSaveChanges, 1, 4);


    //lay_right_box->addWidget(bOpen,0,Qt::AlignRight);
    //lay_right_box->addWidget(bApply,0,Qt::AlignRight);

    connect(bOpen,SIGNAL(released()),this,SLOT(onOpenFile()));
    connect(bApply,SIGNAL(released()),this,SLOT(onApplyChanges()));
    connect(leName,SIGNAL(textEdited(QString)),this,SLOT(onValueEdited(QString)));
    connect(leFname,SIGNAL(textEdited(QString)),this,SLOT(onValueEdited(QString)));
    connect(leDescr,SIGNAL(textEdited(QString)),this,SLOT(onValueEdited(QString)));
}


void DbEditorWidget::initData()
{
    QStringList s;
    listModel->setStringList(s);

    int no = db_data->size();
    for(int i = 0; i < no; i++)
    {
        listModel->insertRow(listModel->rowCount());
        QModelIndex idx = listModel->index(listModel->rowCount()-1,0);
        listModel->setData(idx,db_data->at(i)->name);
    }

    // reselect current index
    if(no>0)
    {
        QModelIndex idx = listModel->index(currentIndex,0);
        listView->setCurrentIndex(idx);
    }
}


void DbEditorWidget::setDatabaseManager(DatabaseManager *dbm)
{
    this->dbm = dbm;
}


bool DbEditorWidget::hasUnsavedChanges()
{
    return _unsavedChanges;
}


void DbEditorWidget::onSelectionChanged(const QItemSelection &selection)
{
    currentIndex = selection.indexes().first().row();
    bApply->setEnabled(false);
    labelUnsavedChanges->setVisible(false);

    leName->setText(db_data->at(currentIndex)->name);
    leFname->setText(db_data->at(currentIndex)->filename);
    leDescr->setText(db_data->at(currentIndex)->description);
}


void DbEditorWidget::onValueEdited(const QString &)
{
    bApply->setEnabled(true);
    labelUnsavedChanges->setVisible(true);
    _unsavedChanges = true;
}


/**
 * @brief DbEditorWidget::onOpenFile opens file with external application
 */
void DbEditorWidget::onOpenFile()
{
    QUrl url = QUrl::fromLocalFile(dbm->getDataBasePath()
                                   + db_data->at(currentIndex)->filename);
    QDesktopServices::openUrl(url);
}


void DbEditorWidget::onApplyChanges()
{
    bApply->setEnabled(false);
    labelUnsavedChanges->setVisible(false);
    _unsavedChanges = false;
}


void DbEditorWidget::onAddItemToList()
{
    butAdd->setEnabled(false);
}


void DbEditorWidget::onRemoveCurrentSelectionFromList()
{
    if(db_data == NULL) return;

    butRemove->setEnabled(false);

    if(db_data->size() <= 0)
    {
        butRemove->setEnabled(true);
        return;
    }

    QString fname = db_data->at(currentIndex)->filename;

    dbm->removeContentFromDB(db_data->at(currentIndex));

    currentIndex --;
    if(currentIndex<=0)currentIndex = 0;

    initData();

   /* QModelIndex idx = listModel->index(currentIndex,0);
    listView->setCurrentIndex(idx);*/
    MSG_STATUS("removed: " + fname);
    butRemove->setEnabled(true);
}


/**
 * @brief get valid default file name
 * @return
 * @details checks if the default filenames database subdirectory exists and
 * creates it if necessary. To avoid overwriting other files with the default name
 * the filename will be extended (with an index) until the given file is nonexistent.
 */
 QString  DbEditorWidget::getValidDefaultFileName()
{
    QString fname = dbm->getDataBasePath() + defaultFileName;

    // make sure that the file's database subdirectory exists
    createDirForFile(fname);

    QString validName = fname + ".txt";

    int i = 1;
    while(QFile(validName).exists())
    {
        validName = fname + QString::number(i) + ".txt";
        i++;
    }

    validName.remove(dbm->getDataBasePath());
    return validName;
}


/**
 * @brief creates a database subdirectory for a given File name (if it does not already exist)
 * @param fname
 */
void DbEditorWidget::createDirForFile(const QString &fname)
{
    // strip down file name:
    QString dirName = fname;
    while(!dirName.endsWith('/') && !dirName.isEmpty())
    {
        dirName.remove(dirName.size()-1,1);
    }

    if(!QDir(dirName).exists())
    {
        QDir().mkdir(dirName);
        qDebug() << "DbEditor: created " << dirName;
    }
}

