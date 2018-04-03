#ifndef VISTOGGABLETWI_H
#define VISTOGGABLETWI_H

#include <QObject>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @brief The VisToggableTWI class
 * dummy class for empty elements like "Unprocessed signal" processing step or
 * separator in Temperature processing steps
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class VisToggableTWI : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:

    explicit VisToggableTWI(QTreeWidget* view );
    explicit VisToggableTWI(QTreeWidget* view, QString text );
    ~VisToggableTWI();

    void setupItemWidget();
    void setVisState(bool state);


private:
    void init();

    /// @brief Widget used as ItemWidget
    QWidget* m_mainItemWidget;

    /// @brief Layout for item widget
    QHBoxLayout* m_itwLayout;

    /// @brief Label which shows item name
    QLabel* m_nameLabel;

    /// @brief Button for toggling plot visibility
    QPushButton* m_visButton;


signals:

    void dataChanged(int pos, QVariant value);

public slots:

private slots:

    void onVisButtonReleased();
    void updatePlotVisibilityIcon();
};

#endif // VISTOGGABLETWI_H
