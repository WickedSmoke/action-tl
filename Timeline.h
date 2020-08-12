#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>

class QBoxLayout;

class Timeline : public QWidget
{
public:
    Timeline( QWidget* parent = NULL );
    void addSubject( const QString& name );
protected:
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
private:
    Timeline(const Timeline&);

    QBoxLayout* _lo;
};

class ActionTimeline : public QWidget
{
public:
    ActionTimeline( QWidget* parent = NULL );
private:
    ActionTimeline(const Timeline&);
};

#endif //TIMELINE_H
