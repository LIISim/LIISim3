#ifndef BASEPLOTSPECTROGRAMWIDGETQWT_H
#define BASEPLOTSPECTROGRAMWIDGETQWT_H

#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>

#include <qwt_matrix_raster_data.h>

#include <QWidget>
#include <QGridLayout>



class BasePlotSpectrogramWidgetQwt : public QWidget
{
    Q_OBJECT

    public:
        explicit BasePlotSpectrogramWidgetQwt(QWidget *parent = 0);

        ~BasePlotSpectrogramWidgetQwt();

        QwtPlot* qwtPlot;

        void setAlpha(int idx, int alpha);

        int createMatrixPlot(int rows, int columns);
        void initMatrix(int idx, int rows, int columns);

        void setRange(double xi, double xa, double yi, double ya);
        void setZRange(double zi, double za);
        void setZRangeAuto(int idx);
        inline double getZMin() { return zmin; }
        inline double getZMax() { return zmax; }

        int getMatrixRows(int idx);
        int getMatrixColumns(int idx);

        void setValue(int idx, int row, int col, double value);

        void updateData(int idx);

        void setPlotAxisTitles(const QString & xTitle, const QString & yTitle, const QString & zTitle);

    protected:

        // plot


        QFont axisFont;

        QList<QwtPlotSpectrogram *> qwtPlotItems;
        QList<QwtMatrixRasterData *> rasterData;

        double xmin, xmax;
        double ymin, ymax;
        double zmin, zmax;

        // layout
        QGridLayout *layMain;

    signals:

        void rangeChangedZ(double zi, double za);
        void dataCursorSelected(double x, double y);

    protected slots:

        // tools: data cursor
        void onDataCursorSelection(const QPoint &point);

};

#endif // BASEPLOTSPECTROGRAMWIDGETQWT_H
