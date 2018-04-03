#ifndef QLIGHTBOXWIDGET_H
#define QLIGHTBOXWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>


/**
 * @brief https://github.com/dimkanovikov/QLightBoxWidget
 */
class QLightBoxWidget : public QWidget
{
	Q_OBJECT

public:
	explicit QLightBoxWidget(QWidget* _parent, bool _folowToHeadWidget = false);

    inline void setHighlightArea(QRect rect) { hl_area = rect; }
    void setTitle(QString title);
    void setDescription(QString description);
    void setOrientation(Qt::Alignment flag);
    void setTextColorBlack(bool enabled);

    void setButtonBackVisible(bool state);

protected:
	/**
	 * @brief Переопределяется для отслеживания собитий родительского виджета
	 */
	bool eventFilter(QObject* _object, QEvent* _event);

	/**
	 * @brief Переопределяется для того, чтобы эмитировать эффект перекрытия
	 */
	void paintEvent(QPaintEvent* _event);

	/**
	 * @brief Переопределяется для того, чтобы перед отображением настроить внешний вид
	 */
	void showEvent(QShowEvent* _event);

    void mousePressEvent(QMouseEvent *);


private:
	void updateSelf();

    QRect hl_area;
    QGridLayout* mainLayout;
    QGridLayout* lbLayout;

    QLabel* lbTitle;
    QLabel* lbDescription;
    Qt::Alignment align;

    QPushButton* lbBack;
    QPushButton* lbNext;
    QPushButton* lbClose;

	bool m_isInUpdateSelf;

	QPixmap grabParentWidgetPixmap() const;

	QPixmap m_parentWidgetPixmap;


    void setTextLayoutBox();

private slots:

    void onBack();
    void onNext();
    void onClose();

signals:

    void onLastView();
    void onNextView();
    void onCloseView();
};

#endif // QLIGHTBOXWIDGET_H
