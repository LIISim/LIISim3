#ifndef GRIDABLESPLITTER_H
#define GRIDABLESPLITTER_H

#include <QWidget>
#include <QSplitter>
#include <QList>

// -----------------------
// TODO: DOCUMENTATION !!!
// -----------------------

/**
 * @brief The GridableSplitter class
 * @ingroup GUI
 * @details .:. TODO .:.
 */
class GridableSplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit GridableSplitter(Qt::Orientation orientation, QWidget *parent = 0);

private:
    QList<GridableSplitter*> m_splitters;
    bool m_mute;
signals:

public slots:

    void copyMovesFromAndTo(GridableSplitter *split);
    void addWidget(QWidget *widget);
    void doMove(int pos, int index);
private slots:
    void handleOtherSplitterMoved(int pos, int index);

    void handleThisSplitterMoved(int pos, int index);
};

#endif // GRIDABLESPLITTER_H
