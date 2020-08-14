#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>
#include <QPixmap>

class QBoxLayout;
class QLabel;
class ColorLabel;

class Timeline : public QWidget
{
    Q_OBJECT
public:
    Timeline( QWidget* parent = NULL );
    void addSubject( const QString& name, bool sel = true );
    void orderSubject( int dir );
    bool hasSelection() const { return _subject >= 0; }
    void select(int);
    bool appendAction(int);
    void saveImage();
    void advance( int sec );
    int  startTime() const { return _startTime; }
    void setStartTime( int sec );
    void setTurnDuration( int sec );
public slots:
    void deleteSubject(int);
    void deleteLastAction();
protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void mousePressEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    int  subjectAt(const QPoint& pnt) const;
private:
    QBoxLayout* selectedLayout();
    ColorLabel* selectedNameLabel();
    void makeTimeScale(int);
    Timeline(const Timeline&);

    QPixmap _timeScale;
    QLabel* _scale;
    QBoxLayout* _lo;
    int _pixPerSec;     // Pixels per second scale.
    int _startTime;     // Time at left side of timeline.
    int _turnDur;
    int _subject;       // Selected subject index.
};

class QComboBox;
class QLineEdit;
class QListWidgetItem;

class ActionTimeline : public QWidget
{
    Q_OBJECT
public:
    ActionTimeline( QWidget* parent = NULL );
    void loadSubjects( int count, char** names );
public slots:
    void newSubject();
    void subjectUp();
    void subjectDown();
    void turnDurationChanged(int);
    void addAction(QListWidgetItem*);
    void advance();
    void timeEdited();
private:
    ActionTimeline(const Timeline&);

    Timeline* _tl;
    QComboBox* _turn;
    QLineEdit* _time;
};

#endif //TIMELINE_H
