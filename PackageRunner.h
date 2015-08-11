#ifndef PACKAGERUNNER_H
#define PACKAGERUNNER_H

#include <QJsonObject>
#include "SoftwareList.h"
#include "ConfOperation.h"

class PackageRunner
{
public:
    PackageRunner();
protected:
	mapDowningTaskObject _TaskObjects;
public:
	mapDowningTaskObject &getTasks();
public:
	bool initTasks();
	bool addTask(QJsonObject installer);
};

#endif // PACKAGERUNNER_H
