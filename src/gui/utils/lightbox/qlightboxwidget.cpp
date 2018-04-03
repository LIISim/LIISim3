#include "qlightboxwidget.h"

#include <QPixmap>
#include <QEvent>
#include <QPaintEvent>
#include <QChildEvent>
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDebug>

#include "../../core.h"

/**
 * Source: https://github.com/dimkanovikov/QLightBoxWidget
 */

QLightBoxWidget::QLightBoxWidget(QWidget* _parent, bool _folowToHeadWidget) :
	QWidget(_parent),
	m_isInUpdateSelf(false)
{
	//
	// Родительский виджет должен быть обязательно установлен
	//
	Q_ASSERT_X(_parent, "", Q_FUNC_INFO);

	//
	// Если необходимо, делаем родителем самый "старший" виджет
	//
	if (_folowToHeadWidget) {
		while (_parent->parentWidget() != 0) {
			_parent = _parent->parentWidget();
		}
		setParent(_parent);
	}


    lbBack = new QPushButton(tr("Back"));
    lbBack->setVisible(false);
    lbNext = new QPushButton(tr("Next"));

    lbClose = new QPushButton();
    lbClose->setIcon(QIcon(Core::rootDir + "resources/icons/close_white.svg"));
    lbClose->setIconSize(QSize(20,20));
    lbClose->setStyleSheet("border: 0px;");
    lbClose->setMaximumWidth(22);

    lbBack->setMaximumWidth(100);
    lbNext->setMaximumWidth(100);

    lbTitle = new QLabel(tr("QLightBoxWidget"));
    //lbTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: white");

    lbDescription = new QLabel(tr("Example how to use QLightBoxWidget\n"
                                              "in your QtWidgets applications..."));
    //lbDescription->setStyleSheet("font-size: 16px; color: white");

    QLabel* lbDummy = new QLabel("");

    QGridLayout* buttonLayout = new QGridLayout;

    buttonLayout->addWidget(lbBack, 3, 2, 1, 1, Qt::AlignRight);
    buttonLayout->addWidget(lbNext, 3, 3, 1, 1, Qt::AlignRight);

    // default alignment
    align = Qt::AlignRight;

    lbLayout = new QGridLayout;
    lbLayout->setRowStretch(0, 1);
    lbLayout->setColumnStretch(0, 1);
    lbLayout->addWidget(lbTitle, 1, 1);    
    lbLayout->setColumnStretch(3, 1);
    lbLayout->addWidget(lbDescription, 2, 1, 1, 2);
    lbLayout->addWidget(lbDummy,3,1);
    lbLayout->addLayout(buttonLayout,3, 2, 1, 1, Qt::AlignRight);
    lbLayout->setRowStretch(4, 1);


    mainLayout = new QGridLayout;
    mainLayout->setColumnStretch(0,1);
    mainLayout->setColumnStretch(1,1);
    mainLayout->setColumnStretch(2,1);
    mainLayout->setRowStretch(0, 1);
    mainLayout->setRowStretch(1, 1);
    mainLayout->setRowStretch(2, 1);

    mainLayout->addLayout(lbLayout,0,0);
    mainLayout->addWidget(lbClose, 0, 3, 1, 1, Qt::AlignRight);

    setLayout(mainLayout);

    // initialize layout
    //setTextLayoutBox();

    connect(lbBack, SIGNAL(clicked()), this, SLOT(onBack()));
    connect(lbNext, SIGNAL(clicked()), this, SLOT(onNext()));
    connect(lbClose, SIGNAL(clicked()), this, SLOT(onClose()));

    _parent->installEventFilter(this);

	setVisible(false);
}


void QLightBoxWidget::setTextLayoutBox()
{
    // 3x3 grid
    mainLayout->removeItem(lbLayout);

    if(align ==  (Qt::AlignLeft | Qt::AlignTop))
    {
            mainLayout->addLayout(lbLayout,0,0);
    }
    else if(align == Qt::AlignLeft)
    {
            mainLayout->addLayout(lbLayout,1,0);
    }
    else if(align ==  (Qt::AlignLeft | Qt::AlignBottom))
    {
            mainLayout->addLayout(lbLayout,2,0);
    }
    else if(align ==  (Qt::AlignCenter| Qt::AlignTop))
    {
            mainLayout->addLayout(lbLayout,0,1);
    }
    else if(align ==  Qt::AlignCenter)
    {
            mainLayout->addLayout(lbLayout,1,1);
    }
    else if(align ==  (Qt::AlignCenter| Qt::AlignBottom))
    {
            mainLayout->addLayout(lbLayout,2,1);
    }
    else if(align ==  (Qt::AlignRight | Qt::AlignTop))
    {
            mainLayout->addLayout(lbLayout,0,2);
    }
    else if(align == Qt::AlignRight)
    {
            mainLayout->addLayout(lbLayout,1,2);
    }
    else if(align ==  (Qt::AlignRight | Qt::AlignBottom))
    {
            mainLayout->addLayout(lbLayout,2,2);
    }
    else
    {
            mainLayout->addLayout(lbLayout,1,1);
    }
}

/**
 * SETTER
 */

void QLightBoxWidget::setTitle(QString title)
{
    lbTitle->setText(title);
}


void QLightBoxWidget::setDescription(QString description)
{
    lbDescription->setText(description);
}


void QLightBoxWidget::setOrientation(Qt::Alignment flag)
{
    align = flag;
}


void QLightBoxWidget::setTextColorBlack(bool enabled)
{
    if(enabled)
    {        
        lbTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: black");
        lbDescription->setStyleSheet("font-size: 16px; color: black");
    }
    else
    {        
        lbTitle->setStyleSheet("font-size: 28px; font-weight: bold; color: white");
        lbDescription->setStyleSheet("font-size: 16px; color: white");
    }
}

void QLightBoxWidget::setButtonBackVisible(bool state)
{
    lbBack->setVisible(state);
}

/**
 * SLOTS
 */

void QLightBoxWidget::onBack()
{
    emit onLastView();
}


void QLightBoxWidget::onNext()
{
    emit onNextView();
}


void QLightBoxWidget::mousePressEvent(QMouseEvent *)
{
     emit onNextView();
}


void QLightBoxWidget::onClose()
{
    emit onCloseView();
}


void QLightBoxWidget::paintEvent(QPaintEvent* _event)
{
    setTextLayoutBox();

    int x1 = hl_area.x();
    int w  = hl_area.width();

    int y1 = hl_area.y();
    int h  = hl_area.height();

    QPainter p;
        p.begin(this);

        p.drawPixmap(0, 0, width(), height(), m_parentWidgetPixmap);

       //... draw darkened area arround hl_rect
        p.setBrush(QBrush(QColor(0, 0, 0, 180)));
        p.setPen(Qt::NoPen);        
 //       p.drawRect(x1, y1, w, h);

        p.drawRect(0, 0, width(), y1);
        p.drawRect(0, y1+h, width(), height()-y1);

        p.drawRect(0, y1, x1, h);
        p.drawRect(x1+w, y1, width()-x1-w, h);

        p.end();

	QWidget::paintEvent(_event);
}


bool QLightBoxWidget::eventFilter(QObject* _object, QEvent* _event)
{
    //
    // The widget should always be the last child,
    // to override other widgets when displaying
    //
    if (_event->type() == QEvent::ChildAdded) {
        QChildEvent* childEvent = dynamic_cast<QChildEvent*>(_event);
        if (childEvent->child() != this) {
            QWidget* parent = parentWidget();
            setParent(0);
            setParent(parent);
        }
    }

    //
    // If the size of the parent widget has changed, you must
    // redraw yourself
    //
    if (isVisible()
        && _event->type() == QEvent::Resize) {
        updateSelf();
    }
    return QWidget::eventFilter(_object, _event);
}


void QLightBoxWidget::showEvent(QShowEvent* _event)
{
	updateSelf();
	QWidget::showEvent(_event);
}


void QLightBoxWidget::updateSelf()
{
	if (!m_isInUpdateSelf) {
		m_isInUpdateSelf = true;
		{
			//
            // update the display
			//
			hide();
			resize(parentWidget()->size());
			m_parentWidgetPixmap = grabParentWidgetPixmap();
			show();
		}

		m_isInUpdateSelf = false;
	}
}

QPixmap QLightBoxWidget::grabParentWidgetPixmap() const
{
	QPixmap parentWidgetPixmap;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	parentWidgetPixmap = parentWidget()->grab();
#else
	parentWidgetPixmap = QPixmap::grabWidget(parentWidget());
#endif

	return parentWidgetPixmap;
}
