#ifndef EXPORTOVERWRITEDIALOG_H
#define EXPORTOVERWRITEDIALOG_H

#include <QWidget>
#include <QFileInfo>

class ExportOverwriteDialog
{
public:
    ExportOverwriteDialog(QFileInfo &finfo, QWidget *parent = 0);

    int show();

private:
    QFileInfo &_finfo;
    QWidget *_parent;
};

#endif // EXPORTOVERWRITEDIALOG_H
