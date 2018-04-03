#ifndef DA_USERDEFINEDPARAMETERS_H
#define DA_USERDEFINEDPARAMETERS_H

#include <QWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

class UserDefinedParameterWidget : public QWidget
{
    Q_OBJECT

public:
    UserDefinedParameterWidget(QWidget *parent = 0);

    bool isValid();
    QString getIdentifier();
    QString getValue();

    void setIdentifier(QString identifier);
    void setValue(QString value);

    QToolButton *buttonRemove;

private:
    QHBoxLayout *layout;
    QCheckBox *checkbox;
    QLineEdit *lineEditIdentifier;
    QLineEdit *lineEditValue;

signals:
    void removeClicked(UserDefinedParameterWidget *widget);
    void valuesChanged();

private slots:
    void onButtonRemoveClicked();
    void onChanges();
};


class UserDefinedParameterMasterWidget : public QWidget
{
    Q_OBJECT
public:
    UserDefinedParameterMasterWidget(QWidget *parent = 0);

    QHBoxLayout *layout;
    QToolButton *buttonAdd;
    QToolButton *buttonClear;
};

#endif // DA_USERDEFINEDPARAMETERS_H
