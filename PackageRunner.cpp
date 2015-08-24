#include "PackageRunner.h"
#include "Storage.h"
#include <QtDebug>

PackageRunner::PackageRunner(QObject *parent) : QObject(parent)
{

}

mapDowningTaskObject &PackageRunner::getTasks() {
	return _TaskObjects;
}

bool PackageRunner::initTasks() {
	QString szFile = ConfOperation::Root().getSubpathFile("Conf", "installPending.conf");
	return Storage::LoadItemsFromConfArray(szFile, _TaskObjects);
}

bool PackageRunner::addTask(QVariantMap installer) {
	if (installer.contains("id") && installer.value("id").type()==QVariant::String &&
		installer.contains("catid") && installer.value("catid").type() == QVariant::String &&
		installer.contains("launchName") && installer.value("launchName").type() == QVariant::String
		) {
		if (_TaskObjects.find(installer.value("id").toString()) == _TaskObjects.end()) {
			LPDowningTaskObject taskObject = new DowningTaskObject();
			taskObject->id = installer.value("id").toString();
			taskObject->category = installer.value("catid").toString();
			taskObject->launchName = installer.value("launchName").toString();
			taskObject->status = 0;
			taskObject->hTaskHandle = NULL;

			_TaskObjects.insert(taskObject->id, taskObject);
			Storage::AddItemToConfArray(ConfOperation::Root().getSubpathFile("Conf", "installPending.conf"), installer);
            qDebug() << "add task :" << installer.value("id").toString() << ","
                     << installer.value("catid").toString() << ","
					 << installer.value("launchName").toString();
		}
        else {
            qDebug() << "repeat task :" << installer.value("id").toString() << ","
                     << installer.value("catid").toString() << ","
					 << installer.value("launchName").toString();
            return false;
        }
	}
    else {
        return false;
    }
	return true;
}
