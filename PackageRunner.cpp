#include "PackageRunner.h"
#include <QtDebug>

PackageRunner::PackageRunner()
{

}

mapDowningTaskObject &PackageRunner::getTasks() {
	return _TaskObjects;
}

bool PackageRunner::initTasks() {
	QString szFile = ConfOperation::Root().getSubpathFile("Conf", "installPending.conf");
	return SoftwareList::LoadArrayFromConf(szFile, _TaskObjects);
}

bool PackageRunner::addTask(QJsonObject installer) {
	if (installer.contains("id") && installer.value("id").isString() &&
		installer.contains("catid") && installer.value("catid").isString() &&
		installer.contains("launchName") && installer.value("launchName").isString()
		) {
		if (_TaskObjects.find(installer.value("id").toString()) == _TaskObjects.end()) {
			LPDowningTaskObject taskObject = new DowningTaskObject();
			taskObject->id = installer.value("id").toString();
			taskObject->category = installer.value("catid").toString();
			taskObject->launchName = installer.value("launchName").toString();
			taskObject->status = 0;
			taskObject->hTaskHandle = NULL;

			_TaskObjects.insert(taskObject->id, taskObject);

			qDebug() << "add task :" << installer.value("id").toString() << "," << installer.value("catid").toString() << "," << installer.value("launchName").toString();
		}
		else {
			qDebug() << "repeat task :" << installer.value("id").toString() << "," << installer.value("catid").toString() << "," << installer.value("launchName").toString();
		}
	}
	return true;
}
