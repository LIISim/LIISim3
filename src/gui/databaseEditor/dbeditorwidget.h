#ifndef DBEDITORWIDGET_H
#define DBEDITORWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QHBoxLayout>
#include <QListView>
#include <QStringListModel>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "../../core.h"
#include "../../database/databasecontent.h"

class DatabaseManager;

/**
 * @brief Base class for database item editors
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class DbEditorWidget : public QWidget
{
    Q_OBJECT

    QHBoxLayout* lay_main;

protected:
    QString defaultFileName;
    QString defaultName;
    int currentIndex;
    DatabaseManager* dbm;
    QList<DatabaseContent*>* db_data;


    // GUI ELEMENTS
    QGridLayout* lay_props;
    QVBoxLayout* lay_right_box;

    QVBoxLayout* lay_left_box;
    QHBoxLayout* lay_list_buttons;
    QStringListModel* listModel;
    QPushButton* butAdd;
    QPushButton* butRemove;

    QListView* listView;

    QLabel* lbName;
    QLineEdit* leName;

    QLabel* lbFname;
    QLineEdit* leFname;

    QLabel* lbDescr;
    QLineEdit* leDescr;

    QPushButton* bOpen;
    QPushButton* bApply;

    void createDirForFile(const QString & fname);
    QString getValidDefaultFileName();

public:
    virtual void setDatabaseManager(DatabaseManager *dbm);


    explicit DbEditorWidget(QWidget *parent = 0);


signals:


public slots:

    virtual void onSelectionChanged(const QItemSelection& selection);
    virtual void initData();
    virtual void onValueEdited(const QString &);
    virtual void onOpenFile();
    virtual void onApplyChanges();
    virtual void onAddItemToList();
    virtual void onRemoveCurrentSelectionFromList();

};

#endif // DBEDITORWIDGET_H
