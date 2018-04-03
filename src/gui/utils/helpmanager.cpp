#include "helpmanager.h"
#include <QFile>
#include <QMessageBox>

#include "../../core.h"

HelpManager::HelpManager(){}

/**
 * @brief HelpManager::getHelpFileName
 * @param tag
 * @return filename of html file for given tag
 */
QString HelpManager::getHelpFileName(QString tag)
{
    QString helpRootDir = Core::rootDir + "resources/help/";

    QString res ="";
    if(tag == "importCSV")
        res = helpRootDir+"signalImportExport/csv_import.html";
    else if(tag == "importCSVauto")
        res = helpRootDir+"signalImportExport/csv_import_auto.html";
    else if(tag == "importMAT")
        res = helpRootDir+"signalImportExport/mat_import.html";
    else if(tag == "importDAT")
        res = helpRootDir+"signalImportExport/dat_import.html";
    else if(tag == "importCustom")
        res = helpRootDir+"signalImportExport/custom_import.html";

    return res;
}


/**
  * @brief HelpManager::getHelpHTML
  * @param tag
  * @return String containging html content for given tag
  */
 QString HelpManager::getHelpHTML(QString tag)
{
    QString fname = getHelpFileName(tag);
    QString html = "";

    QFile f(fname);
    if(f.exists())
    {
        f.open(QFile::ReadOnly|QFile::Text);
        html = f.readAll();
        f.close();
    }
    else
    {
        html = "HelpManager: cannot find help file: "+fname;
    }

    return html;
}


 /**
 * @brief HelpManager::showMessageBox: displays html content for tag in a message box window
 * @param tag
 */
void HelpManager::showMessageBox(QString tag)
{
    QMessageBox msgBox;
    msgBox.setModal(false);
    msgBox.setWindowTitle("Help");
    QString html = getHelpHTML(tag);
    msgBox.setText(html);
   // msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);
    int ret = msgBox.exec();
}
