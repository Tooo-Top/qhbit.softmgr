#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QThread>

class TaskManager : public QThread
{
    Q_OBJECT
public:
    TaskManager(QObject * parent = 0);
protected:
    virtual void run();
};

#endif // TASKMANAGER_H
