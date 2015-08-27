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
        object.insert(QString("downloadUrl"),QVariant::fromValue(it.value()->downloadUrl));

        lstTaskStatus.append(object);
    }

    emit updateAllTaskStatus(lstTaskStatus);
}

void PackageRunner::reqAddTask(QVariantMap task) {
    if (task.contains("id") && task.value("id").type()==QVariant::String ) {
        if (_TaskObjects.find(task.value("id").toString()) == _TaskObjects.end()) {
			LPDowningTaskObject taskObject = new DowningTaskObject();
            //QString packageItems[nPackageItemCount] = {
            //	"id",
            //	"packageName",
            //	"windowsVersion",
            //	"arch",
            //	"name",
            //	"category",
            //	"description",
            //	"developer",
            //	"iconUrl",
            //	"largeIcon",

            //	"screenshotsUrl",
            //	"incomeShare",
            //	"rating",
            //	"versionName",
            //	"versionCode",
            //	"priceInfo",
            //	"tag",
            //	"downloadUrl",
            //	"hash",
            //	"size",

            //	"createTime",
            //	"updateTime",
            //	"signature",
            //	"updateInfo",
            //	"language",
            //	"brief",
            //	"isAd",
            //	"status"
            //};
            taskObject->autoInstall= task.value("autoInstall").toBool();

            taskObject->id = task.value("id").toString();
            taskObject->name = task.value("name").toString();
            taskObject->category = task.value("category").toString();
            taskObject->launchName = task.value("name").toString();//packageName
            if (task.value("ptdownloadUrl").toString().isEmpty()) {
                taskObject->downloadUrl = task.value("downloadUrl").toString();
            }
            else{
                taskObject->downloadUrl = task.value("ptdownloadUrl").toString();
            }
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
            object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
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
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskPause(taskObject->hTaskHandle);
            }

            QVariantMap object;
            object.insert(QString("id"),QVariant::fromValue(taskObject->id));
            object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
            object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
            object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
            object.insert(QString("status"),QVariant::fromValue(taskObject->status));
            object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
            object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
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
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskPause(taskObject->hTaskHandle);
            }
        }
		QVariantMap object;
		object.insert(QString("id"), QVariant::fromValue(taskObject->id));
		object.insert(QString("catid"), QVariant::fromValue(taskObject->category));
		object.insert(QString("launchName"), QVariant::fromValue(taskObject->launchName));
		object.insert(QString("autoInstall"), QVariant::fromValue(taskObject->autoInstall));
		object.insert(QString("status"), QVariant::fromValue(taskObject->status));
		object.insert(QString("percent"), QVariant::fromValue(taskObject->percent));
        object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
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
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskStart(taskObject->hTaskHandle);
            }
        }

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
        object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
        emit updateTaskStatus(object);
    }
}

void PackageRunner::reqResumeAllTask(){
    qDebug()<<"reqResumeAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();

        if (taskObject->status==2) {
            taskObject->status = 0;
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskStart(taskObject->hTaskHandle);
            }
        }
        else if( taskObject->status==3 || taskObject->status==5 || taskObject->status==6) {
            taskObject->status = 0;
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskStart(taskObject->hTaskHandle);
            }
        }

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
        object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
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
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskPause(taskObject->hTaskHandle);
                _Wapper->TaskDelete(taskObject->hTaskHandle);
                _Wapper->DelTempFile(taskObject->downTaskparam);
            }
        }

        QVariantMap object;
        object.insert(QString("id"),QVariant::fromValue(taskObject->id));
        object.insert(QString("catid"),QVariant::fromValue(taskObject->category));
        object.insert(QString("launchName"),QVariant::fromValue(taskObject->launchName));
        object.insert(QString("autoInstall"),QVariant::fromValue(taskObject->autoInstall));
        object.insert(QString("status"),QVariant::fromValue(taskObject->status));
        object.insert(QString("percent"),QVariant::fromValue(taskObject->percent));
        object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
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
        object.insert(QString("downloadUrl"),QVariant::fromValue(taskObject->downloadUrl));
        taskStatus.append(object);
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::PeriodPollTaskStatus() {
//    qDebug()<<"PeriodPollTaskStatus()";
	int startCount = 0;
	int MaxTask = 10;

    DownTaskInfo info;
	QString szTmp;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Data") + QDir::separator();
    defaultRepository = QDir::toNativeSeparators(defaultRepository);

    defaultRepository = SwmgrApp::Instance()->getSettingParameter(QString("Repository"), defaultRepository);

    // query task process
	for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
		LPDowningTaskObject taskObject = it.value();
        if (taskObject != NULL && taskObject->hTaskHandle != NULL &&
                (taskObject->status == 1 || taskObject->status ==7) ) {
            memset(&info, 0, sizeof(info));
            _Wapper->TaskQueryEx(taskObject->hTaskHandle, info);
			bool bNeedUpdate = false;
            switch (info.stat)
            {
            case NOITEM:
                // maybe finish
//                taskObject->percent=1.0f;
//                taskObject->status = 10;
				qDebug() << defaultRepository + QString::fromWCharArray(taskObject->downTaskparam.szFilename);
				if (QFile::exists(defaultRepository + QString::fromWCharArray(taskObject->downTaskparam.szFilename))) {
					taskObject->status = 10;
				}
				_Wapper->TaskPause(taskObject->hTaskHandle);
				_Wapper->TaskDelete(taskObject->hTaskHandle);
				taskObject->hTaskHandle = NULL;
				bNeedUpdate = true;
				break;
            case TSC_ERROR:
                if (info.fail_code) {
                    taskObject->percent=1.0f;
                    taskObject->status=5;
                }
                break;
            case TSC_PAUSE:
                taskObject->status = 2;
				bNeedUpdate = true;
				break;
            case TSC_DOWNLOAD:
                // doing
                taskObject->percent = info.fPercent;
                taskObject->status = 7;
				bNeedUpdate = true;
				startCount++;
                break;
            case TSC_COMPLETE:
                // finish
                taskObject->percent=1.0f;
                taskObject->status = 10;
				_Wapper->TaskPause(taskObject->hTaskHandle);
				_Wapper->TaskDelete(taskObject->hTaskHandle);
				taskObject->hTaskHandle = NULL;
				bNeedUpdate = true;
				break;
            case TSC_STARTPENDING:
                taskObject->status = 1;
                startCount++;
				bNeedUpdate = true;
				break;
            case TSC_STOPPENDING:
                taskObject->status = 3;
				bNeedUpdate = true;
				break;
            default:
                break;
            }
			if (bNeedUpdate) {
				QVariantMap object;
				object.insert(QString("id"), QVariant::fromValue(taskObject->id));
				object.insert(QString("catid"), QVariant::fromValue(taskObject->category));
				object.insert(QString("launchName"), QVariant::fromValue(taskObject->launchName));
				object.insert(QString("autoInstall"), QVariant::fromValue(taskObject->autoInstall));
				object.insert(QString("status"), QVariant::fromValue(taskObject->status));
				object.insert(QString("percent"), QVariant::fromValue(taskObject->percent));
				object.insert(QString("downloadUrl"), QVariant::fromValue(taskObject->downloadUrl));
                emit updateTaskDownloadProgress(object);
			}
        }
    }

    // replenish to max task objects
	for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();
        if (taskObject != NULL ) {
            if ( taskObject->status == 0 ) {
                if (startCount < MaxTask) { // may start download
                    if (taskObject->hTaskHandle != NULL) {
                        _Wapper->TaskStart(taskObject->hTaskHandle);
                        taskObject->status=1;
                        startCount++;
                    }
                    else{
                        // start new task
                        StrCpyW(taskObject->downTaskparam.szTaskUrl, taskObject->downloadUrl.toStdWString().data());
                        StrCpyW(taskObject->downTaskparam.szSavePath, defaultRepository.toStdWString().data());
						QString szTmp = taskObject->launchName + QString(".exe");
                        StrCpyW(taskObject->downTaskparam.szFilename, szTmp.toStdWString().data());
                        taskObject->downTaskparam.IsOnlyOriginal = FALSE;
						taskObject->hTaskHandle = _Wapper->TaskCreate(taskObject->downTaskparam);
						if (taskObject->hTaskHandle != NULL) {
							_Wapper->TaskStart(taskObject->hTaskHandle);
							taskObject->status = 1;
							startCount++;
						}
                    }
                }
                else {
                    break;
                }
            }
        }
    }
}
