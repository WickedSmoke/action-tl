#include "PixmapChooser.h"
#include <QMouseEvent>
#include <QPainter>


PixmapChooser::PixmapChooser( QWidget* parent )
    : QWidget(parent), _cols(4), _sel(-1)
{
    _dim.w = _dim.h = 16;
    setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}

void PixmapChooser::setPixmaps( const std::vector<QPixmap*>& collection )
{
    _pix = collection;
    if( ! _pix.empty() )
    {
        QPixmap* pm = _pix[0];
        _dim.w = pm->width();
        _dim.h = pm->height();
    }
}

void PixmapChooser::addPixmap( QPixmap* pm )
{
    if( _pix.empty() )
    {
        _dim.w = pm->width();
        _dim.h = pm->height();
    }
    _pix.push_back(pm);
}

QSize PixmapChooser::sizeHint() const
{
    int count = _pix.size();
    if( count )
        return QSize( 2 + (_dim.w + pad) * _cols,
                      2 + (_dim.h + pad) * ((count+_cols-1) / _cols) );
    return QSize( 16 * _cols, 16 );
}

int PixmapChooser::indexAt( int px, int py )
{
    px -= 2;
    py -= 2;
    if( py < 0 || px < 0 || px >= ((_dim.w + pad) * _cols) )
        return -1;
    return py / (_dim.h + pad) * _cols + px / (_dim.w + pad);
}

void PixmapChooser::mousePressEvent( QMouseEvent* ev )
{
    mouseMoveEvent( ev );
}

#if QT_VERSION >= 0x060000
#define POS_X(ev)   int(ev->position().x())
#define POS_Y(ev)   int(ev->position().y())
#else
#define POS_X(ev)   ev->x()
#define POS_Y(ev)   ev->y()
#endif

void PixmapChooser::mouseReleaseEvent( QMouseEvent* ev )
{
    if( ev->button() == Qt::LeftButton ||
        ev->button() == Qt::RightButton )
    {
        int ns = indexAt(POS_X(ev), POS_Y(ev));
        if( ns >= 0 )
            emit selected(ns);
    }
}

void PixmapChooser::mouseMoveEvent( QMouseEvent* ev )
{
    int ns = indexAt(POS_X(ev), POS_Y(ev));
    if( ns >= 0 && ns < int(_pix.size()) && _sel != ns )
    {
        _sel = ns;
        update();
    }
}

void PixmapChooser::paintEvent( QPaintEvent* ev )
{
    int count = _pix.size();
    if( count > 0 )
    {
        QPainter p(this);
        int i;
        int c = 0;
        int x = hpad;
        int y = hpad;
        int rbottom = ev->rect().bottom();

        QSize rdim( _dim.w + pad, _dim.h + pad );

        for( i = 0; i < count; ++i )
        {
            p.drawPixmap( x, y, *_pix[i] );
            if( ++c == _cols )
            {
                c = 0;
                x = hpad;
                y += rdim.height();
                if( y > rbottom )
                    break;
            }
            else
            {
                x += rdim.width();
            }
        }

        if( _sel >= 0 )
        {
            QPen pen( Qt::black );
            pen.setWidth( 2 );
            p.setPen( pen );
            p.drawRect( 1 + _sel % _cols * rdim.width(),
                        1 + _sel / _cols * rdim.height(),
                        _dim.w + pad, _dim.h + pad );
        }
    }
}
