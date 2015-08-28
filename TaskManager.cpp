#include "TaskManager.h"
#include <QTimer>
#include <QVariantMap>
#include "DataControl.h"
#include "PackageRunner.h"

TaskManager::TaskManager(QObject * parent) : QThread(parent)
{
}

void TaskManager::SetObjects(DataControl *dataControl,PackageRunner *runner) {
    pTaskRunner = runner;
    pDataControl= dataControl;
    pTaskRunner->moveToThread(this);
}

void TaskManager::run() {
    if (pTaskRunner!=NULL && pDataControl!=NULL) {
        // request
        QObject::connect(pDataControl,SIGNAL(sigQueryAllTaskStatus()),pTaskRunner,SLOT(reqAllTaskStatus()),Qt::QueuedConnection);

        QObject::connect(pDataControl,SIGNAL(sigAddTask(QVariantMap)),pTaskRunner,SLOT(reqAddTask(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigAddTasks(QVariantList)),pTaskRunner,SLOT(reqAddTasks(QVariantList)),Qt::QueuedConnection);

        QObject::connect(pDataControl,SIGNAL(sigPauseTask(QVariantMap)),pTaskRunner,SLOT(reqPauseTask(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigPauseAllTask()),pTaskRunner,SLOT(reqPauseAllTask()),Qt::QueuedConnection);

        QObject::connect(pDataControl,SIGNAL(sigResumeTask(QVariantMap)),pTaskRunner,SLOT(reqResumeTask(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigResumeAllTask()),pTaskRunner,SLOT(reqResumeAllTask()),Qt::QueuedConnection);

        QObject::connect(pDataControl,SIGNAL(sigRemoveTask(QVariantMap)),pTaskRunner,SLOT(reqRemoveTask(QVariantMap)),Qt::QueuedConnection);
        QObject::connect(pDataControl,SIGNAL(sigRemoveAllTask()),pTaskRunner,SLOT(reqRemoveAllTask()),Qt::QueuedConnection);

        // response
		QObject::connect(pTaskRunner, SIGNAL(updateAllTaskStatus(QVariantList)), pDataControl, SIGNAL(updateAllTaskStatus(QVariantList)), Qt::QueuedConnection);
        QObject::connect(pTaskRunner, SIGNAL(updateTaskStatus(QVariantMap)), pDataControl, SIGNAL(updateTaskStatus(QVariantMap)), Qt::QueuedConnection);
        QObject::connect(pTaskRunner, SIGNAL(updateTaskDownloadProgress(QVariantMap)),pDataControl,SIGNAL(updateTaskDownloadProgress(QVariantMap)),Qt::QueuedConnection);

        QObject::connect(pTaskRunner, SIGNAL(initCrash()),pDataControl, SIGNAL(sigCrash()),Qt::QueuedConnection);
        // load history list
        pTaskRunner->init();
        QTimer period ,installPeriod;

        QObject::connect(&period,SIGNAL(timeout()),pTaskRunner,SLOT(PeriodPollTaskStatus()));
        QObject::connect(&installPeriod,SIGNAL(timeout()),pTaskRunner,SIGNAL(installTaskStart()));

        period.start(2000);
        installPeriod.start(5000);
        QThread::exec();
        installPeriod.stop();
        period.stop();
        pTaskRunner->unInit();
    }
    else{
        QThread::exit(1);
    }
}
