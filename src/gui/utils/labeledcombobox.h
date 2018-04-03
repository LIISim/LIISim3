#ifndef LABELEDCOMBOBOX_H
#define LABELEDCOMBOBOX_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>

#include "plugininputfield.h"

class DatabaseContent;

/**
 * @brief Widget managing a Combobox and a description Label
 * @details can be used to manage a list of DatabaseContent and provides a
 * slot to update the Combobox on Database changes. This widget also implements
 * the PluginInputField interface
 * @ingroup GUI-Utilities
 */
class LabeledComboBox : public PluginInputField
{
    Q_OBJECT

private:

    bool useDataBaseContent;
    bool initDbContent;
    QList<DatabaseContent*>* dbc;

    void init(bool hStretch);

protected:

    QLabel* description;

public:    
    explicit LabeledComboBox(QWidget *parent = 0);
    LabeledComboBox(QString descr,bool hStretch = true,QWidget *parent = 0);

    void setDatabaseContent(QList<DatabaseContent*>* d);

    DatabaseContent* getSelectedDbContent();

    int getCurrentIndex();
    QString getCurrentText();
    void setCurrentIndex(int i);
    void setCurrentItem(DatabaseContent* dbi);
    void setCurrentItem(int dbc_ident);
    void addStringItem(QString itemname);

    // implement the PluginInputField interface
    void init();
    void setPluginParamValue(QVariant value);
    void setPluginParamMinValue(QVariant minValue);
    void setPluginParamMaxValue(QVariant maxValue);
    void setPluginLabelText(QString text);
    void setPluginLabelList(QList<QString> labels);
    void setPluginValueList(QList<QVariant> values);
    QVariant getPluginParamValue() ;

    void clearAll();

    QComboBox* choices;
    QHBoxLayout* layMainH;

signals:
    void currentIndexChanged(int);

public slots:
    void slot_onDBcontentChanged();

protected slots:
    virtual void onCurrentIndexChanged(int idx);
};

#endif // LABELEDCOMBOBOX_H
