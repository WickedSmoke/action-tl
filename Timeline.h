#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>

class QBoxLayout;

class Timeline : public QWidget
{
public:
    Timeline( QWidget* parent = NULL );
    void addSubject( const QString& name );
    //void removeSelected( const QString& name );
protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
    void contextMenuEvent(QContextMenuEvent*);
private:
    Timeline(const Timeline&);

    QBoxLayout* _lo;
    int _pixPerSec;
    int _startTime;
    int _subject;
};

class ActionTimeline : public QWidget
{
    Q_OBJECT
public:
    ActionTimeline( QWidget* parent = NULL );
    void loadSubjects(const char*);
public slots:
    void newSubject();
private:
    ActionTimeline(const Timeline&);

    Timeline* _tl;
};

#endif //TIMELINE_H
