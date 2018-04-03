#ifndef MRUNGROUP_H
#define MRUNGROUP_H

#include "../models/dataitem.h"
#include <QList>
#include <QString>
#include <QColor>


#include "../gui/utils/colormap.h"

class MRun;
class ProcessingPluginConnector;

/**
 * @brief The MRunGroup represents a group of MRuns.
 * @ingroup Hierachical-Data-Model
 * It only stores Pointers to MRuns and is not responsible
 * for creating or deleting MRun Objects!
 */
class MRunGroup : public DataItem
{
    Q_OBJECT

public:

    explicit MRunGroup(const QString& title = "untitled");
    virtual ~MRunGroup();

    void setTitle(const QString& title);

    MRun* at(int i) const;
    QString title() const;
    static int maxID();

    inline ColorMap* colorMap(){return m_colorMap;}

    virtual bool insertChild(DataItem *child, int position = -1, bool initGlobalProcessors = true);

    MRun* findMRun(QString runname);
    QList<MRun*> mruns();
private:

    /// @brief position of title in data vector
    int pos_title;

    /// @brief title of group
    QString g_title;

    /// @brief colormap for mrun children
    ColorMap* m_colorMap;

    QList<ProcessingPluginConnector*> findInitialGroupProcessors();

signals:

public slots:

private slots:

    void assignMRunColors();


};

#endif // MRUNGROUP_H
