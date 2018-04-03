#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>
#include <QCheckBox>

class AboutWindow : public QWidget
{
    Q_OBJECT
public:
    AboutWindow(QString title);
    ~AboutWindow();

    virtual QSize sizeHint() const;

private:
    QCheckBox *checkboxShow;

private slots:
    void onLinkActivated(const QString &link);

};

#endif // ABOUTWINDOW_H
