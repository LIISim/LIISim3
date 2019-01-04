#include "aboutwindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include "core.h"

#include <qwt_global.h>
#include <boost/version.hpp>
#include <matio.h>
#include <zlib.h>

AboutWindow::AboutWindow(QString title) : QWidget(0)
{
    setWindowTitle("About LIISim");

    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::ApplicationModal);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    QVBoxLayout *mainLayout_left = new QVBoxLayout;
    QVBoxLayout *mainLayout_right = new QVBoxLayout;

    QString infoText = "";    
    infoText.append(QString("<h2><b>%0</b></h2>").arg(title));
    infoText.append(QString("Build date: %1").arg(__DATE__));
    infoText.append("<br><br>");
    infoText.append("<b>If you make use of this software in your research, please cite:</b>");
    infoText.append("<table style=\"padding: 0px; margin: 5px; background: #c9d7e9; \">");
    infoText.append("<tr>");
    infoText.append("<td style=\"padding: 5px; margin: 5px;\">");
    infoText.append("R. Mansmann, T. Terheiden, P. Schmidt, J. Menser, T. Dreier,");
    infoText.append(" T. Endres and C. Schulz, ");
    infoText.append("\"LIISim: a modular signal processing toolbox for laser-induced incandescence measurements\"");
    infoText.append(" Appl. Phys. B <b>124</b>(4), 69 (2018). <div style=\"white-space: nowrap;\">DOI&nbsp;10.1007/s00340-018-6934-9</div>");
    infoText.append("</tr></td></table>");

    infoText.append("View at publisher: <a href='https://doi.org/10.1007/s00340-018-6934-9'>DOI 10.1007/s00340-018-6934-9</a>");

    infoText.append("<br><br><br>");
    infoText.append("<b>Documentation:</b>");
    infoText.append("<br>");
    infoText.append("<a href='http://www.liisim.com'>www.liisim.com</a>");

    infoText.append("<br><br>");

    infoText.append("<b>Source code:</b>");
    infoText.append("<br>");
    infoText.append("<a href='https://www.github.com/LIISim/LIISim3'>www.github.com/LIISim/LIISim3</a>");

    infoText.append("<br><br>");

    infoText.append("<b>Authors:</b><br> ");
    infoText.append("Raphael Mansmann<br>");
    infoText.append("Tobias Terheiden<br>");
    infoText.append("Philip Schmidt<br>");

    infoText.append("<br><b>Contact:</b><br>");
    infoText.append("<a href='mailto:raphael.mansmann@uni-due.de'>raphael.mansmann@uni-due.de</a><br>");
    infoText.append("<a href='https://www.uni-due.de/ivg/rf/'>www.uni-due.de/ivg/rf/</a>");

    infoText.append("<br><br><br>");
    infoText.append("<b>LIISim is still work-in-progress. If you encounter any errors,<br>");
    infoText.append("please open an issue <a href='https://github.com/LIISim/LIISim3/issues'>here</a></b>.");

    infoText.append("<br>");

    QLabel *text = new QLabel(infoText, this);
    text->setTextFormat(Qt::RichText);
    text->setScaledContents(true);
    text->setWordWrap(true);
    text->setMinimumWidth(400);
    mainLayout_left->addWidget(text);

    checkboxShow = new QCheckBox("Don't show this window again");
    checkboxShow->setChecked(!Core::instance()->guiSettings->value("aboutWindow", "showAtStart", true).toBool());
    mainLayout_left->addWidget(checkboxShow);
    mainLayout_left->setAlignment(checkboxShow, Qt::AlignLeft);

    QString infoText2 = "";

    //adapted from: https://www.gnu.org/licenses/gpl-howto.de.html
    infoText2.append("<br><br><br><b>License:</b>");
    infoText2.append("<br>LIISim is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.");
    infoText2.append("<br><br>LIISim is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.");
    infoText2.append("<br><br>You should have received a copy of the GNU General Public License along with this program; if not, see <a href='http://gnu.org/licenses'>gnu.org/licenses</a>.");

    infoText2.append("<br><br><br><b>LIISim is based in part on the work of:</b>");
    infoText2.append(QString("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Qt Project</b> (Version %0) - <a href='http://www.qt.io'>qt.io</a> - <a href='http://doc.qt.io/qt-5/gpl.html'>View License</a>").arg(QT_VERSION_STR));
    infoText2.append(QString("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Qwt Library</b> (Version %0) - <a href='http://qwt.sourceforge.net/'>qwt.sourceforge.net</a> - <a href='local:resources/licenses/qwt_6.1.2/LICENSE_1_0.txt'>View License</a>").arg(QWT_VERSION_STR));
    infoText2.append(QString("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Boost Library</b> (Version %0.%1.%2) - <a href='http://www.boost.org'>boost.org</a> - <a href='local:resources/licenses/boost_1.55.0/LICENSE_1_0.txt'>View License</a>").arg(BOOST_VERSION / 100000).arg(BOOST_VERSION / 100 % 1000).arg(BOOST_VERSION % 100));
    int major, minor, release;
    Mat_GetLibraryVersion(&major, &minor, &release);
    infoText2.append(QString("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Matio Library</b> (Version %0.%1.%2) - <a href='https://github.com/tbeu/matio'>github.com/tbeu/matio</a> - <a href='local:/resources/licenses/matio/COPYING'>View License</a>").arg(major).arg(minor).arg(release));
    infoText2.append(QString("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>zlib</b> (Version %0) - <a href='http://zlib.net'>zlib.net</a> - <a href='http://zlib.net/zlib_license.html'>View License</a>").arg(ZLIB_VERSION));
    infoText2.append("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Qwt MathML Renderer</b> - <a href='https://www.github.com/uwerat/qwt-mml-dev'>github.com/uwerat/qwt-mml-dev</a>");
    infoText2.append("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>QLightBoxWidget</b> - <a href='https://www.github.com/dimkanovikov/QLightBoxWidget'>github.com/dimkanovikov/QLightBoxWidget</a> - <a href='local:/resources/licenses/qlightboxwidget/LICENSE'>View License</a>");
    infoText2.append("<br><br>&nbsp;&nbsp;&nbsp;&nbsp;<b>FatCow Icons</b> - <a href='http://www.fatcow.com/free-icons'>fatcow.com/free-icons</a> - <a href='http://creativecommons.org/licenses/by/3.0/us/'>View License</a>");

    infoText2.append("<br>");

    QLabel *text2 = new QLabel(infoText2, this);
    text2->setTextFormat(Qt::RichText);
    text2->setScaledContents(true);
    text2->setWordWrap(true);
    text2->setMinimumWidth(425);
    text2->setIndent(20);
    mainLayout_right->addWidget(text2);

    mainLayout->addLayout(mainLayout_left);
    mainLayout->addLayout(mainLayout_right);

    mainLayout->setAlignment(mainLayout_left, Qt::AlignTop);
    mainLayout->setAlignment(mainLayout_right, Qt::AlignTop);
    setLayout(mainLayout);

    connect(text, SIGNAL(linkActivated(QString)), SLOT(onLinkActivated(QString)));
    connect(text2, SIGNAL(linkActivated(QString)), SLOT(onLinkActivated(QString)));

#ifdef LIISIM_FULL
    //if(!Core::instance()->guiSettings->value("FullVersionWarning", "Shown", false).toBool())
    {
        Core::instance()->guiSettings->setValue("FullVersionWarning", "Shown", true);
        QMessageBox fvw;
        fvw.setText("<br>This is the <b>FULL version of LIISim</b>. There is <b>no support</b> for the 'Data Acquisition'-module, the additional plugins and the 'Calibration' and 'Absolute Calibration' analysis tools.");
        fvw.exec();
    }
#endif
}


AboutWindow::~AboutWindow()
{
    Core::instance()->guiSettings->setValue("aboutWindow", "showAtStart", !checkboxShow->isChecked());
}


QSize AboutWindow::sizeHint() const
{
    return QSize(850, 450);
}


void AboutWindow::onLinkActivated(const QString &link)
{
    if(link.startsWith("local:"))
    {
        QWidget *widgetLicenseBrowser = new QWidget;
        widgetLicenseBrowser->setWindowFlags(Qt::WindowStaysOnTopHint);
        widgetLicenseBrowser->setWindowTitle("License");
        widgetLicenseBrowser->setLayout(new QHBoxLayout);
        widgetLicenseBrowser->layout()->setMargin(2);
        QTextBrowser *textBrowser = new QTextBrowser(widgetLicenseBrowser);
        textBrowser->setMinimumSize(500, 600);
        widgetLicenseBrowser->layout()->addWidget(textBrowser);
        widgetLicenseBrowser->setAttribute(Qt::WA_DeleteOnClose);

        QString licenseText = "";

        QString fileString = Core::rootDir;
        QString link2 = link;
        fileString.append(link2.remove("local:"));

        QFile file(fileString);
        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            licenseText = stream.readAll();
        }
        textBrowser->setText(licenseText);
        widgetLicenseBrowser->show();
    }
    else
    {
        QDesktopServices::openUrl(QUrl(link));
    }
}

