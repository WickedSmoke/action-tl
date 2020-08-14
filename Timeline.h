#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>

class QBoxLayout;
class ColorLabel;

class Timeline : public QWidget
{
public:
    Timeline( QWidget* parent = NULL );
    void addSubject( const QString& name );
    bool hasSelection() const { return _subject >= 0; }
    void select(int);
    void saveImage();
    void advance( int sec );
    int  startTime() const { return _startTime; }
    void setStartTime( int sec );
protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void mousePressEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    int  subjectAt(const QPoint& pnt) const;
private:
    ColorLabel* selectedNameLabel();
    Timeline(const Timeline&);

    QBoxLayout* _lo;
    int _pixPerSec;     // Pixels per second scale.
    int _startTime;     // Time at left side of timeline.
    int _subject;       // Selected subject index.
};

class QComboBox;
class QLineEdit;

class ActionTimeline : public QWidget
{
    Q_OBJECT
public:
    ActionTimeline( QWidget* parent = NULL );
    void loadSubjects( int count, char** names );
public slots:
    void newSubject();
    void advance();
    void timeEdited();
private:
    ActionTimeline(const Timeline&);

    Timeline* _tl;
    QComboBox* _turn;
    QLineEdit* _time;
};

#endif //TIMELINE_H
