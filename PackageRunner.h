#ifndef PACKAGERUNNER_H
#define PACKAGERUNNER_H
#include <QObject>

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include "DownWrapper.h"

#include "ConfOperation.h"
#include "DownWrapper.h"

/*
* status follow in configure file
*     0 : just add
*     1 : started
*     2 : pause
*     3 : finish
*     4 : download err
*     5 : delete
*     6 : execute err
*/

typedef struct __DowningTaskObject {
	QString id;
	QString name;
	QString category;
	QString launchName;
    bool autoInstall;

    QString downloadUrl;
	DownTaskParam downTaskparam;
    int status; //0:added/pending,1:starting,2:pause,3:stop,4:del,5:err,6:param err,7: running, 10:finish
	float percent;
	HANDLE hTaskHandle;
}DowningTaskObject, *LPDowningTaskObject;

typedef QMap<QString, LPDowningTaskObject> mapDowningTaskObject;

class PackageRunner : public QObject
{
    Q_OBJECT
public:
    PackageRunner(QObject *parent=0);
protected:
	mapDowningTaskObject _TaskObjects;

    DownWrapper* _Wapper;  // xunlei mini
protected:
	BOOL initMiniXL();
	// XL mini
    DownWrapper* LoadDll();
    void UnloadDll(DownWrapper** Wapper);

public:
	bool init();
    void unInit();

signals:
    void initCrash();

signals:
    void updateAllTaskStatus(QVariantList);
    void updateTaskStatus(QVariantMap);
    void updateTaskDownloadProgress(QVariantMap swTaskProcess);
public slots:
    void reqAllTaskStatus();

    void reqAddTask(QVariantMap task);
    void reqAddTasks(QVariantList tasks);

    void reqPauseTask(QVariantMap task);
    void reqPauseAllTask();

    void reqResumeTask(QVariantMap task);
    void reqResumeAllTask();

    void reqRemoveTask(QVariantMap task);
    void reqRemoveAllTask();
protected slots:
    void PeriodPollTaskStatus();
};

#endif // PACKAGERUNNER_H
