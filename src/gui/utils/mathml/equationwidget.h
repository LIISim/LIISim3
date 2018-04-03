#ifndef EQUATIONWIDGET_H
#define EQUATIONWIDGET_H

/**
 * @brief The EquationWidget class
 * Qwt MathML Renderer ( see http://qwt.sourceforge.net )
 * Adapted from: https://github.com/uwerat/qwt-mml-dev
 */

#include <QWidget>
#include <QString>

class EquationWidget: public QWidget
{
    Q_OBJECT

public:
    EquationWidget( QWidget *parent = NULL );
    QString formula() const;

public Q_SLOTS:

    void setFormula( const QString & );
    void setFontSize( const qreal & );
    void setTransformation( const bool &transformation );
    void setScale( const bool &scale );
    void setRotation( const qreal & );
    void setDrawFrames( const bool &drawFrames );
    void setColors( const bool &colors );

protected:
    virtual void paintEvent( QPaintEvent * );

private:
    void renderFormula( QPainter * ) const;

    QString d_formula;
    qreal d_fontSize;
    bool d_transformation;
    bool d_scale;
    qreal d_rotation;
    bool d_drawFrames;
    bool d_colors;

};

#endif // EQUATIONWIDGET_H
