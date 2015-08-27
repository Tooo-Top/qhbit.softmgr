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
*     9 : all completed
*/

typedef struct __DowningTaskObject {
// id,catid,name,largeIcon,brief,size,percent,speed,status,downloadUrl,autoInstall
	QString id;
	QString category;
    QString name;
    QString largeIcon;
    QString brief;
    qint64  size;
    float   percent;
    qint64  speed;
    int status; //0:added/pending,1:starting,2:pause,3:stop,4:del,5:err,6:param err,7: running, 9 : all completed, 10:finish
    QString downloadUrl;
    bool autoInstall;

    QString launchName;
	HANDLE hTaskHandle;
    DownTaskParam downTaskparam;
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

    void encodeToVariantMap(LPDowningTaskObject,QVariantMap&);
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
