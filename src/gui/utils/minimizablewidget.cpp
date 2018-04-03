#include "minimizablewidget.h"

#include "QVBoxLayout"

#include "core.h"
#include "QFile"

MinimizableWidget::MinimizableWidget(QWidget *parent, bool invArrow) : QWidget(parent)
{
    // invert arrow (if collapsing upwards or downwards)
    invertArrow = invArrow;

    // set custom style sheet  (if stylesheet resource exists)
    if(QFile::exists(Core::rootDir + "resources/style/minimizablewidgetstyle.qss"))
    {
        QFile file(Core::rootDir + "resources/style/minimizablewidgetstyle.qss");
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        setStyleSheet(styleSheet);
    }
    else
    {
        MSG_ERR("Cannot find stylesheet: " + Core::rootDir + "resources/style/minimizablewidgetstyle.qss");
    }

    setLayout(new QVBoxLayout);

    toolbar = new QToolBar("", this);
    toolbar->setObjectName("toolbarMinimizableWidget");

    layout()->addWidget(toolbar);
    layout()->setMargin(0);

    if(invertArrow)
        toggleVisibility = new QAction(QIcon(Core::rootDir + "resources/icons/doubleArrowUp.png"), "Toggle Visibility", this);
    else
        toggleVisibility = new QAction(QIcon(Core::rootDir + "resources/icons/doubleArrowDown.png"), "Toggle Visibility", this);

    labelTitle = new QLabel("", this);
    labelTitle->setObjectName("labelMinimizableWidget");

    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    toolbar->addWidget(labelTitle);
    toolbar->addWidget(spacer);
    toolbar->addAction(toggleVisibility);
    toolbar->setIconSize(QSize(10,10));

    togglingWidget = new QWidget(this);
    togglingWidget->setLayout(new QVBoxLayout);
    togglingWidget->layout()->setMargin(0);

    layout()->addWidget(togglingWidget);

    connect(toggleVisibility, SIGNAL(triggered(bool)), SLOT(onActionToggleVisibility()));
}


void MinimizableWidget::setTitle(QString title)
{
    labelTitle->setText(title);
}


void MinimizableWidget::setWidget(QWidget *widget)
{
    togglingWidget->layout()->addWidget(widget);
}


void MinimizableWidget::onActionToggleVisibility()
{
    if(togglingWidget->isVisible())
    {
        togglingWidget->setVisible(false);
        if (invertArrow)
            toggleVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/doubleArrowDown.png"));
        else
            toggleVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/doubleArrowUp.png"));
    }
    else
    {
        togglingWidget->setVisible(true);
        if (invertArrow)
            toggleVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/doubleArrowUp.png"));
        else
            toggleVisibility->setIcon(QIcon(Core::rootDir + "resources/icons/doubleArrowDown.png"));
    }
}
