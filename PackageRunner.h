#ifndef PACKAGERUNNER_H
#define PACKAGERUNNER_H
#include <QObject>

#include <QVariant>
#include <QVariantMap>

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
	DownTaskParam downTaskparam;
	int status; //0:added,1:start,2:pause,3:stop,4:del,5:err,6:param err
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
public:
	mapDowningTaskObject &getTasks();
public:
	bool initTasks();
    bool addTask(QVariantMap installer);
};

#endif // PACKAGERUNNER_H
