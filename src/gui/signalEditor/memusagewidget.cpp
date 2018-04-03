#include "memusagewidget.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include "../../core.h"

MemUsageWidget::MemUsageWidget(QWidget *parent)
    : RibbonToolBox("MEMORY USAGE",parent)
{
    Core* core = Core::instance();
    GuiSettings* gs = core->guiSettings;

    info = new QLabel(" ");
    info->setToolTip("Physical memory used (in GB) after\n"
                     " last signal processing or data import");

    addWidget(info,0,0);

    bypass = new QCheckBox("Check Memory");
    bypass->setToolTip("Bypass the memory check before signal processing.\nNote:"
                       "The program could crash when this option is disabled\n"
                       "and the amount of memory needed for signal processing\n"
                       "exceeds the memory limit of this machine!");
    addWidget(bypass,1,0);

    connect(bypass,
            SIGNAL(stateChanged(int)),
            SLOT(onBypassStateChanged(int)));

    connect(gs,
            SIGNAL(settingsChanged()),
            SLOT(onGuiSettingsChanged()));

    MemUsageMonitor* m = Core::instance()->getSignalManager()->memUsageMonitor();

    connect(m,SIGNAL(infosUpdated()),
            SLOT(updateInfoText()));

}


MemUsageWidget::~MemUsageWidget()
{

}


void MemUsageWidget::onBypassStateChanged(int state)
{
    //qDebug() << " MemUsageWidget::onBypassStateChanged()" ;
    GuiSettings* gs = Core::instance()->guiSettings;

    gs->setValue("memusage","bypass",!bypass->isChecked());

    if(!bypass->isChecked())
    {
        /*QMessageBox::warning(this,"Warning: Disabling Memory Check before signal processing!",
                     "Disabling this option could lead to program crashes, especially when "
                     "you are running this program on a 32 bit machine!");
        */
        MSG_WARN("!!! MEMORY CHECK DISABLED !!!");
        MSG_STATUS("Memory check disabled!");
    }
}


/**
 * @brief MemUsageWidget::onGuiSettingsChanged This slot is
 * executed when the global GUI Settings changed.
 */
void MemUsageWidget::onGuiSettingsChanged()
{
    GuiSettings* gs = Core::instance()->guiSettings;
    bypass->setChecked(!gs->value("memusage","bypass",true).toBool());
}


/**
 * @brief MemUsageWidget::updateInfoText update
 * info text
 */
void MemUsageWidget::updateInfoText()
{
    MemUsageMonitor* m = Core::instance()->getSignalManager()->memUsageMonitor();
    QString txt;
    txt.sprintf("%2.2f/%2.2f GB",
                m->toGB(m->physicalMemoryUsedByLIISim()),
                m->toGB(m->physicalMemoryAllowed()));
    info->setText(txt);
}


