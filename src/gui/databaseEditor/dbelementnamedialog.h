#ifndef DBELEMENTNAMEDIALOG_H
#define DBELEMENTNAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QStatusBar>
#include <typeinfo>
#include <QBoxLayout>

class DBElementNameDialog : public QDialog
{
    Q_OBJECT
public:
    DBElementNameDialog(const std::type_info &type, QWidget *parent = 0);

    void setDefault(const QString &name);

    QString getName();
    QString getUniqueFilename(QString dir, QString extension);

protected:
    QString getFilename();

    QLineEdit *editName;
    QStatusBar *statusBar;

    QVBoxLayout *layout;

    const std::type_info &_type;

public slots:
    virtual void accept();

};

#endif // DBELEMENTNAMEDIALOG_H
