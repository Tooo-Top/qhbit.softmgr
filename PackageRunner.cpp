#include "PackageRunner.h"
#include "swmgrapp.h"
#include "Storage.h"
#include <QtDebug>

PackageRunner::PackageRunner(QObject *parent) : QObject(parent)
{

}

bool PackageRunner::init() {
    if (!initMiniXL()) {
        emit initCrash();
        return false;
    }

	QString szFile = ConfOperation::Root().getSubpathFile("Conf", "installPending.conf");
	return Storage::LoadItemsFromConfArray(szFile, _TaskObjects);
}

BOOL PackageRunner::initMiniXL() {
    _Wapper = LoadDll();
    if (!_Wapper) {
        return FALSE;
    }
    if (!_Wapper->Init()) {
        UnloadDll(&_Wapper);
        return FALSE;
    }
    return TRUE;
}

void PackageRunner::unInit() {
    UnloadDll(&_Wapper);
}

DownWrapper* PackageRunner::LoadDll()
{
    WCHAR szDllpath[512] = { 0 };
    QString szLoadPath = SwmgrApp::GetProgramProfilePath(QString("xbSpeed"));
    szLoadPath.append( QDir::separator() + QString("xldl.dll") );
    szLoadPath = QDir::toNativeSeparators(szLoadPath);

    StrCpyW(szDllpath, szLoadPath.toStdWString().data());
    DownWrapper* pWapper = NULL;
    try{
        pWapper = new DownWrapper(szDllpath);
    }
    catch (wchar_t e[]) {
        pWapper = NULL;
        qDebug() << "*****************:"<<QString::fromWCharArray(e);
    }
    return pWapper;
}

void PackageRunner::UnloadDll(DownWrapper** Wapper){
    if (!Wapper) {
        return;
    }
    if ((*Wapper) != NULL) {
        (*Wapper)->UnInit();
        delete (*Wapper);
        (*Wapper) = NULL;
    }
}

void PackageRunner::reqAllTaskStatus() {
    QVariantList lstTaskStatus;

    for(mapDowningTaskObject::iterator it = _TaskObjects.begin();it!=_TaskObjects.end();it++) {
        QVariantMap object;

        object.insert(QString("id"),QVariant::fromValue(it.value()->id));
        object.insert(QString("catid"),QVariant::fromValue(it.value()->category));
        object.insert(QString("launchName"),QVariant::fromValue(it.value()->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(it.value()->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(it.value()->status));
        object.insert(QString("percent"),QVariant::fromValue(it.value()->percent));
        lstTaskStatus.append(object);
    }

    emit updateAllTaskStatus(lstTaskStatus);
}

void PackageRunner::reqAddTask(QVariantMap task) {
    if (task.contains("id") && task.value("id").type()==QVariant::String &&
        task.contains("catid") && task.value("catid").type() == QVariant::String &&
        task.contains("launchName") && task.value("launchName").type() == QVariant::String
		) {
        if (_TaskObjects.find(task.value("id").toString()) == _TaskObjects.end()) {
			LPDowningTaskObject taskObject = new DowningTaskObject();
            taskObject->id = task.value("id").toString();
            taskObject->category = task.value("catid").toString();
            taskObject->launchName = task.value("launchName").toString();
            taskObject->autoInstall= task.value("autoInstall").toBool();
			taskObject->status = 0;
            taskObject->percent= 0.0f;

			taskObject->hTaskHandle = NULL;

			_TaskObjects.insert(taskObject->id, taskObject);
            Storage::AddItemToConfArray(ConfOperation::Root().getSubpathFile("Conf", "installPending.conf"), task);
            qDebug() << "add task :" << task.value("id").toString() << ","
                     << task.value("catid").toString() << ","
                     << task.value("launchName").toString();

            QVariantMap object;
            object.insert(QString("id"),QVariant::fromValue(taskObject->id));
            object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
            object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
            object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
            object.insert(QString("status"),QVariant::fromValue(taskObject->status));
            object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
            emit updateTaskStatus(object);
        }
        else {
			qDebug() << "repeat task :" << task.value("id").toString() << ","
                     << task.value("catid").toString() << ","
                     << task.value("launchName").toString();
            return ;
        }
	}
    else {
        return ;
    }
}

void PackageRunner::reqAddTasks(QVariantList tasks){
    qDebug()<<"reqAddTasks:"<<tasks;
    foreach(QVariant var,tasks) {
        if (var.type()==QVariant::Map) {
            reqAddTask(var.toMap());
        }
    }
}

void PackageRunner::reqPauseTask(QVariantMap task){
    qDebug()<<"reqPauseTask:"<<task;
    mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
    if (it != _TaskObjects.end()) {
        LPDowningTaskObject taskObject = it.value();
        if (taskObject->hTaskHandle!=NULL && taskObject->status==7) {
			_Wapper->TaskPause(taskObject->hTaskHandle);
            taskObject->status = 2;

            QVariantMap object;
            object.insert(QString("id"),QVariant::fromValue(taskObject->id));
            object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
            object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
            object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
            object.insert(QString("status"),QVariant::fromValue(taskObject->status));
            object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
            emit updateTaskStatus(object);
        }
    }
}

void PackageRunner::reqPauseAllTask(){
    qDebug()<<"reqPauseAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();
        if (taskObject->hTaskHandle!=NULL && taskObject->status==7) {
			_Wapper->TaskPause(taskObject->hTaskHandle);
            taskObject->status = 2;
        }
		QVariantMap object;
		object.insert(QString("id"), QVariant::fromValue(taskObject->id));
		object.insert(QString("catid"), QVariant::fromValue(taskObject->category));
		object.insert(QString("launchName"), QVariant::fromValue(taskObject->launchName));
		object.insert(QString("autoInstall"), QVariant::fromValue(taskObject->autoInstall));
		object.insert(QString("status"), QVariant::fromValue(taskObject->status));
		object.insert(QString("percent"), QVariant::fromValue(taskObject->percent));
		taskStatus.append(object);
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::reqResumeTask(QVariantMap task){
    qDebug()<<"reqResumeTask:"<<task;
    mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
    if (it != _TaskObjects.end()) {
        LPDowningTaskObject taskObject = it.value();
        if (taskObject->status!= 7) {
            taskObject->status = 0;
        }

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
        emit updateTaskStatus(object);
    }
}

void PackageRunner::reqResumeAllTask(){
    qDebug()<<"reqResumeAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();

        taskObject->status = 0;

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
		taskStatus.append(object);
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::reqRemoveTask(QVariantMap task){
    qDebug()<<"reqRemoveTask:"<<task;
    mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
    if (it != _TaskObjects.end()) {
        LPDowningTaskObject taskObject = it.value();
        if (taskObject->status!= 4) {
            taskObject->status = 4;
        }

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
        emit updateTaskStatus(object);
    }
}

void PackageRunner::reqRemoveAllTask(){
    qDebug()<<"reqRemoveAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();

        taskObject->status = 4;

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
		taskStatus.append(object);
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::PeriodPollTaskStatus() {
    qDebug()<<"PeriodPollTaskStatus()";
	int startCount = 0;
	int MaxTask = 10;

    DownTaskInfo info;

    QString szTmp;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QString("Data") + QDir::separator();
    defaultRepository = QDir::toNativeSeparators(defaultRepository);

    defaultRepository = SwmgrApp::Instance()->getSettingParameter(QString("Repository"), defaultRepository);
	for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
		LPDowningTaskObject taskObject = it.value();
            if (taskObject != NULL && taskObject->hTaskHandle != NULL) {  // polling
                memset(&info, 0, sizeof(info));
				_Wapper->TaskQueryEx(taskObject->hTaskHandle, info);
                switch (info.stat)
                {
                case NOITEM:
                    // maybe finish
                    break;
                case TSC_ERROR:
                    if (info.fail_code) {
                        ;
                    }
                    break;
                case TSC_PAUSE:
                    break;
                case TSC_DOWNLOAD:
                    // doing
                    startCount++;
                    break;
                case TSC_COMPLETE:
                    // finish
                    break;
                case TSC_STARTPENDING:
                    startCount++;
                    break;
                case TSC_STOPPENDING:
                    break;
                default:
                    break;
                }
            }
        }

        // replenish to max task objects
	for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
		LPDowningTaskObject taskObjectReplenish = it.value();
		if (taskObjectReplenish != NULL && taskObjectReplenish->hTaskHandle != NULL) {  // polling
                if (startCount < MaxTask) {
                    startCount++;
    /*				CommonItem TaskConfig;
                    if (taskObjectReplenish->status != 0) {
                        continue;
                    }

                    mapSoftwarePackages packages = _DataModel.getSoftPackages();
                    mapSoftwarePackages::iterator iter = packages.find(taskObjectReplenish->category);
                    if (iter != packages.end()) {// find category
                        lstSoftwarePackage &cateList = iter.value();
                        foreach(CommonItem item, cateList) {
                            if (item["id"].compare(taskObjectReplenish->id,Qt::CaseInsensitive)==0) {
                                TaskConfig = item;
                            }
                        }
                    }
                    if (TaskConfig.empty() || !TaskConfig.contains("downloadUrl") || TaskConfig.contains("packageName")) {
                        taskObjectReplenish->status = 6;
                        continue;
                    }
                    StrCpyW(taskObjectReplenish->downTaskparam.szTaskUrl, TaskConfig["downloadUrl"].toStdWString().data());
                    StrCpyW(taskObjectReplenish->downTaskparam.szSavePath, defaultRepository.toStdWString().data());
                    szTmp = defaultRepository + TaskConfig["packageName"];
                    StrCpyW(taskObjectReplenish->downTaskparam.szFilename, szTmp.toStdWString().data());
                    taskObjectReplenish->downTaskparam.IsOnlyOriginal = FALSE;

                    HANDLE hTask = _pWapper->TaskCreate(taskObjectReplenish->downTaskparam);
                    if (hTask) {
                        _pWapper->TaskStart(hTask);
                        taskObjectReplenish->hTaskHandle = hTask;
                    }*/
                }
            }
        }
}
