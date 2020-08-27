#ifndef PIXMAPCHOOSER_H
#define PIXMAPCHOOSER_H

#include <vector>
#include <QWidget>

class PixmapChooser : public QWidget
{
    Q_OBJECT

public:
    PixmapChooser( QWidget* parent = NULL );
    void setColumns( int count ) { _cols = count; }
    void deselect() { _sel = -1; }
    void setPixmaps( const std::vector<QPixmap*>& );
    void addPixmap( QPixmap* pm );
    QSize sizeHint() const;

signals:
    void selected(int);

protected:
    int indexAt( int px, int py );
    void mousePressEvent( QMouseEvent* ev );
    void mouseReleaseEvent( QMouseEvent* ev );
    void mouseMoveEvent( QMouseEvent* ev );
    void paintEvent( QPaintEvent* ev );

private:
    static const int pad = 4;
    static const int hpad = pad / 2 + 1;
    std::vector<QPixmap*> _pix;
    struct { int w, h; } _dim;
    int _cols;
    int _sel;
};

#endif //PIXMAPCHOOSER_H
