#include "exportoverwritedialog.h"

#include <QMessageBox>
#include <QFileDialog>

ExportOverwriteDialog::ExportOverwriteDialog(QFileInfo &finfo, QWidget *parent) : _finfo(finfo), _parent(parent)
{

}


int ExportOverwriteDialog::show()
{
    while(true)
    {
        QMessageBox msgBox;
        msgBox.setText(QString("The file %0 already exists in the selected directory.\nDo you want to overwrite the file or choose another name?").arg(_finfo.fileName()));

        QPushButton *buttonOverwrite = msgBox.addButton("Overwrite", QMessageBox::ActionRole);
        QPushButton *buttonChooseFilename = msgBox.addButton("Choose Filename", QMessageBox::ActionRole);
        QPushButton *buttonCancel = msgBox.addButton(QMessageBox::Cancel);

        msgBox.exec();

        if((QPushButton*)msgBox.clickedButton() == buttonOverwrite)
        {
            return 1;
        }
        else if((QPushButton*)msgBox.clickedButton() == buttonChooseFilename)
        {
            QFileDialog fd(_parent, "Save matlab export as...", _finfo.absolutePath(), "*.mat");
            fd.selectFile(_finfo.fileName());
            fd.setFileMode(QFileDialog::AnyFile);
            if(fd.exec())
            {
                _finfo.setFile(fd.selectedFiles().first());
                if(!_finfo.exists())
                {
                    if(!_finfo.suffix().contains("mat", Qt::CaseInsensitive))
                        _finfo.setFile(_finfo.filePath() + ".mat");
                    return 0;
                }
            }
        }
        else if((QPushButton*)msgBox.clickedButton() == buttonCancel)
        {
            return -1;
        }
    }
}
