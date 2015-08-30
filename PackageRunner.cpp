#include "PackageRunner.h"
#include "swmgrapp.h"
#include "Storage.h"
#include <QtDebug>

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

PackageRunner::PackageRunner(QObject *parent) : QObject(parent)
{
    installer = NULL;
    currentTaskObject = NULL;
}

bool PackageRunner::init() {
    if (!initMiniXL()) {
        emit initCrash();
        return false;
    }

    installer = new QProcess(this);
    QObject::connect(this,SIGNAL(installTaskStart()),SLOT(PeriodInstallTask()));
    QObject::connect(installer,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(installerFinished(int,QProcess::ExitStatus)));
    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "PendingTasks.conf");
	Storage::LoadTasks(szFile, _TaskObjects);
	return true;
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

void PackageRunner::encodeToVariantMap(LPDowningTaskObject task,QVariantMap& taskObject) {
    if (task==NULL)
        return ;

    taskObject.insert(QString("id"),QVariant::fromValue(task->id));
    taskObject.insert(QString("catid"),QVariant::fromValue(task->category));
    taskObject.insert(QString("name"),QVariant::fromValue(task->name));
    taskObject.insert(QString("largeIcon"),QVariant::fromValue(task->largeIcon));
    taskObject.insert(QString("brief"),QVariant::fromValue(task->brief));
    taskObject.insert(QString("size"),QVariant::fromValue(task->size));
    taskObject.insert(QString("percent"),QVariant::fromValue(task->percent));
    taskObject.insert(QString("speed"),QVariant::fromValue(task->speed));
    taskObject.insert(QString("status"),QVariant::fromValue(task->status));
    taskObject.insert(QString("downloadUrl"),QVariant::fromValue(task->downloadUrl));
    taskObject.insert(QString("autoInstall"),QVariant::fromValue(task->autoInstall));
    taskObject.insert(QString("versionName"),QVariant::fromValue(task->versionName));
    taskObject.insert(QString("packageName"),QVariant::fromValue(task->packageName));
}

void PackageRunner::reqAllTaskStatus() {
    QVariantList lstTaskStatus;

    for(mapDowningTaskObject::iterator it = _TaskObjects.begin();it!=_TaskObjects.end();it++) {
		if (it.value() != NULL && it.value()->status != 4 && it.value()->status != 9) {
			QVariantMap object;
			encodeToVariantMap(it.value(), object);
			if (!object.isEmpty()) { lstTaskStatus.append(object); }
		}
    }

    emit updateAllTaskStatus(lstTaskStatus);
}

void PackageRunner::reqAddTask(QVariantMap task, bool autoInstall) {
	qDebug()<< "reqAddTask" << task;
	QDir dir;
    LPDowningTaskObject taskObject = NULL;

    QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
    defaultRepository = QDir::toNativeSeparators(defaultRepository);
    defaultRepository = SwmgrApp::Instance()->getSettingParameter(QString("Repository"), defaultRepository);
    dir.mkpath(defaultRepository);

    if (task.contains("id") && task.value("id").type()==QVariant::String ) {
        mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
        if (it == _TaskObjects.end()) {
            taskObject = new DowningTaskObject();
            taskObject->id = task.value("id").toString();
            taskObject->category = task.value("category").toString();
            taskObject->name = task.value("name").toString();
            taskObject->largeIcon= task.value("largeIcon").toString();
            taskObject->brief= task.value("brief").toString();
            taskObject->size = task.value("size").toLongLong();
            taskObject->percent= 0.0f;
            taskObject->speed  = 0;
			taskObject->status = 0;
			taskObject->status = task.value("status").toInt();
            taskObject->downloadUrl = (task.value("ptdownloadUrl").isNull()||(task.value("ptdownloadUrl").type()==QVariant::String&&task.value("ptdownloadUrl").toString().size() == 0)) ? task.value("downloadUrl").toString() : task.value("ptdownloadUrl").toString();
            taskObject->versionName = (task.value("versionName").isNull()||(task.value("versionName").type()==QVariant::String&&task.value("versionName").toString().size() == 0)) ? QString("1.0.0.0") : task.value("versionName").toString();
			taskObject->packageName = task.value("packageName").toString();// (task.value("packageName").isNull() || (task.value("packageName").type() == QVariant::String&&task.value("packageName").toString().size() == 0)) ? (taskObject->name + taskObject->versionName + ".exe") : task.value("packageName").toString();
			if (taskObject->packageName.compare("0", Qt::CaseInsensitive) == 0) {
				taskObject->packageName = (taskObject->name + taskObject->versionName + ".exe");
			}
            else {
                taskObject->packageName.append(".exe");
            }

			taskObject->autoInstall = autoInstall;// task.value("autoInstall").toBool();
            taskObject->hTaskHandle = NULL;
            taskObject->launchName  = taskObject->name;
            taskObject->binaryImagePath = defaultRepository;

			_TaskObjects.insert(taskObject->id, taskObject);
			Storage::AddIntoTaskConf(ConfOperation::Root().getSubpathFile("Conf", "PendingTasks.conf"), taskObject);

            QVariantMap object;
            encodeToVariantMap(taskObject,object);
            if (!object.isEmpty()) { emit updateTaskStatus(object); }
            qDebug() << "add task :" << task.value("id").toString() << "," << task.value("catid").toString() << "," << task.value("launchName").toString();
        }
        else {
            taskObject = it.value();
            if (!taskObject) {
                _TaskObjects.erase(it);
            }
            else if (taskObject->status==2 || taskObject->status==3 || taskObject->status==4 || taskObject->status==6 ) {
                taskObject->status = 0;
            }
        }
	}
}

void PackageRunner::reqAddTasks(QVariantList tasks){
    qDebug()<<"reqAddTasks:"<<tasks;
    foreach(QVariant var,tasks) {
        if (var.type()==QVariant::Map) {
			reqAddTask(var.toMap(), true);
        }
    }
}

void PackageRunner::reqPauseTask(QVariantMap task){
    qDebug()<<"reqPauseTask:"<<task;
    mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
    if (it != _TaskObjects.end()) {
        LPDowningTaskObject taskObject = it.value();
		if (taskObject->hTaskHandle != NULL) {
			if (taskObject->status == 0) {
				taskObject->status = 2;
				QVariantMap object;
				encodeToVariantMap(taskObject,object);
				if (!object.isEmpty()) { emit updateTaskStatus(object); }
			}
			else if (taskObject->status == 1 || taskObject->status == 7) {
				_Wapper->TaskPause(taskObject->hTaskHandle);
			}
        }
    }
}

void PackageRunner::reqPauseAllTask(){
    qDebug()<<"reqPauseAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();
		if (taskObject->hTaskHandle != NULL) {
			if (taskObject->status == 0) {
	            taskObject->status = 2;
		        QVariantMap object;
		        encodeToVariantMap(taskObject,object);
		        if (!object.isEmpty()) { taskStatus.append(object); }
			}
			else if (taskObject->status == 1 || taskObject->status == 7) {
				_Wapper->TaskPause(taskObject->hTaskHandle);
			}
        }
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::reqResumeTask(QVariantMap task){
    qDebug()<<"reqResumeTask:"<<task;
    mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
    if (it != _TaskObjects.end()) {
        LPDowningTaskObject taskObject = it.value();
		if (taskObject->status == 2 || taskObject->status == 5 || taskObject->status == 6) {
            taskObject->status = 0;
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskStart(taskObject->hTaskHandle);
            }
        }
		else if (taskObject->status != 4){
	        QVariantMap object;
	        encodeToVariantMap(taskObject,object);
	        if (!object.isEmpty()) { emit updateTaskStatus(object); }
		}
    }
}

void PackageRunner::reqResumeAllTask(){
    qDebug()<<"reqResumeAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();

		if (taskObject->status == 2 || taskObject->status == 5 || taskObject->status == 6) {
            taskObject->status = 0;
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskStart(taskObject->hTaskHandle);
            }
        }
		else if (taskObject->status != 4){
			QVariantMap object;
			encodeToVariantMap(taskObject,object);
			if (!object.isEmpty()) { taskStatus.append(object); }
		}
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::reqRemoveTask(QVariantMap task){
    qDebug()<<"reqRemoveTask:"<<task;
    mapDowningTaskObject::iterator it = _TaskObjects.find(task.value("id").toString());
    if (it != _TaskObjects.end()) {
        LPDowningTaskObject taskObject = it.value();
		if (taskObject->status != 4) {
            taskObject->status = 4;
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskPause(taskObject->hTaskHandle);
                _Wapper->TaskDelete(taskObject->hTaskHandle);
                _Wapper->DelTempFile(taskObject->downTaskparam);
            }
			else {
				QVariantMap object;
				encodeToVariantMap(taskObject, object);
				if (!object.isEmpty()) { emit updateTaskStatus(object); }
			}
        }
    }
}

void PackageRunner::reqRemoveAllTask(){
    qDebug()<<"reqRemoveAllTask()";
    QVariantList taskStatus;
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin() ; it!=_TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();

        if ( taskObject->status != 4 ) {
			taskObject->status = 4;
			if (taskObject->hTaskHandle != NULL) {
				_Wapper->TaskPause(taskObject->hTaskHandle);
				_Wapper->TaskDelete(taskObject->hTaskHandle);
				_Wapper->DelTempFile(taskObject->downTaskparam);
			}
			else {
				QVariantMap object;
				encodeToVariantMap(taskObject,object);
				if (!object.isEmpty()) { taskStatus.append(object); }
			}
        }
    }
    emit updateAllTaskStatus(taskStatus);
}

void PackageRunner::PeriodPollTaskStatus() {
	int startCount = 0;
	int MaxTask = 10;
    bool needUpdateAll = false;
    bool needSerialize = false;

    DownTaskInfo info;
    QDir dir;

    QVariantMap sigTaskObject;
	QString szTmp;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
    defaultRepository = QDir::toNativeSeparators(defaultRepository);
    defaultRepository = SwmgrApp::Instance()->getSettingParameter(QString("Repository"), defaultRepository);
	dir.mkpath(defaultRepository);

    // query task process
	for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
		LPDowningTaskObject taskObject = it.value();
		if (taskObject != NULL && taskObject->hTaskHandle != NULL &&
			(taskObject->status == 1 || taskObject->status == 2 || taskObject->status == 3 || taskObject->status == 7)) {
            memset(&info, 0, sizeof(info));
            _Wapper->TaskQueryEx(taskObject->hTaskHandle, info);
			
			bool bNeedUpdate = false;
            switch (info.stat)
            {
            case NOITEM:
                // maybe finish
				qDebug() << "status:NOITEM";
                taskObject->percent=1.0f;
                szTmp = taskObject->binaryImagePath + taskObject->packageName;
                taskObject->status = QFile::exists(szTmp) ? 10 : 4;
                qDebug() << szTmp;
				_Wapper->TaskPause(taskObject->hTaskHandle);
				_Wapper->TaskDelete(taskObject->hTaskHandle);
				taskObject->hTaskHandle = NULL;

                if (taskObject->status==10) { needUpdateAll = true; }
                bNeedUpdate = true;
                needSerialize = true;
				break;
            case TSC_ERROR:
				qDebug() << "status:TSC_ERROR";
				if (taskObject->status != 5) {
					taskObject->status = 5;
					bNeedUpdate = true;
                    needSerialize = true;
				}
                break;
            case TSC_PAUSE:
				qDebug() << "status:TSC_PAUSE";
				if (taskObject->status != 2) {
					taskObject->status = 2;
					bNeedUpdate = true;
                    needSerialize = true;
				}
				break;
            case TSC_DOWNLOAD:
                // doing
				qDebug() << "status:TSC_DOWNLOAD";
				taskObject->percent = info.fPercent;
                if (taskObject->status != 7) {
                    needSerialize = true;
                }
                taskObject->status = 7;
                taskObject->speed = info.nSpeed;// + info.nSpeedP2S + info.nSpeedP2P;
                bNeedUpdate = false;
				startCount++;
                encodeToVariantMap(taskObject,sigTaskObject);
                if (!sigTaskObject.isEmpty()) {
                    emit updateTaskDownloadProgress(sigTaskObject);
                }

                break;
            case TSC_COMPLETE:
                // finish
				qDebug() << "status:TSC_COMPLETE";
				taskObject->percent = 1.0f;
                taskObject->speed = info.nSpeed;
                _Wapper->TaskPause(taskObject->hTaskHandle);
				_Wapper->TaskDelete(taskObject->hTaskHandle);
				taskObject->hTaskHandle = NULL;

				if (taskObject->status != 10) {
					taskObject->status = 10;
					QVariantMap downFinish;
					encodeToVariantMap(taskObject, downFinish);
					if (!downFinish.isEmpty()) { emit updateTaskDownloadProgress(downFinish); }
                    needUpdateAll = true;
                    bNeedUpdate = true;
                    needSerialize = true;
				}
				break;
            case TSC_STARTPENDING:
				qDebug() << "status:TSC_STARTPENDING";
				taskObject->status = 7;
                startCount++;
				bNeedUpdate = true;
				break;
            case TSC_STOPPENDING:
				qDebug() << "status:TSC_STOPPENDING";
				if (taskObject->status != 3) {
					taskObject->status = 3;
					bNeedUpdate = true;
				}
				break;
            default:
                break;
            }
			if (bNeedUpdate) {
                encodeToVariantMap(taskObject,sigTaskObject);
				if (!sigTaskObject.isEmpty()) { emit updateTaskStatus(sigTaskObject); }
			}
        }
    }

	for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
        if (startCount >= MaxTask) { break; }

        LPDowningTaskObject taskObject = it.value();
        if (taskObject != NULL && taskObject->status == 0 ) { //resume
            if (taskObject->hTaskHandle != NULL) {
                _Wapper->TaskStart(taskObject->hTaskHandle);
                taskObject->status=7;
                startCount++;
            }
            else{  // start new task
                szTmp = taskObject->packageName;
                if (taskObject->binaryImagePath.count()==0){
                    taskObject->binaryImagePath = defaultRepository;
                }

                if (QFile::exists(taskObject->binaryImagePath+szTmp)) {
                    taskObject->status = 10;
                    needUpdateAll = true;
                }
                else {
                    StrCpyW(taskObject->downTaskparam.szTaskUrl, taskObject->downloadUrl.toStdWString().data());
                    StrCpyW(taskObject->downTaskparam.szSavePath, taskObject->binaryImagePath.toStdWString().data());
                    StrCpyW(taskObject->downTaskparam.szFilename, szTmp.toStdWString().data());
                    taskObject->downTaskparam.IsOnlyOriginal = FALSE;
                    taskObject->downTaskparam.DisableAutoRename = TRUE;

                    taskObject->hTaskHandle = _Wapper->TaskCreate(taskObject->downTaskparam);
                    if (taskObject->hTaskHandle != NULL) {
                        _Wapper->TaskStart(taskObject->hTaskHandle);
                        taskObject->status = 7;
                        startCount++;
                    }
                }
                needSerialize = true;
            }
        }
    }
    if (needUpdateAll) { 
		reqAllTaskStatus(); 
	}
    if (needSerialize) {
		Storage::SaveTasks(ConfOperation::Root().getSubpathFile("Conf", "PendingTasks.conf"), _TaskObjects);
    }
}

void PackageRunner::PeriodInstallTask() {
    bool skip = true;
    if (!installer) { return ; }
	if (installer->state() == QProcess::NotRunning) {
        skip = false;
    }
    if (skip) {
        return ;
    }
    qDebug()<<"PackageRunner::PeriodInstallTask():"<<installer->program();

	QDir dir;
	QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
    defaultRepository = QDir::toNativeSeparators(defaultRepository);
    defaultRepository = SwmgrApp::Instance()->getSettingParameter(QString("Repository"), defaultRepository);
	dir.mkpath(defaultRepository);
    for (mapDowningTaskObject::iterator it = _TaskObjects.begin(); it != _TaskObjects.end(); it++) {
        LPDowningTaskObject taskObject = it.value();
        if (taskObject!=NULL && taskObject->status==10) {
            currentTaskObject = taskObject;
            installer->start(defaultRepository + taskObject->packageName,QStringList());
			break;
        }
    }
}

void PackageRunner::installerFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	Q_UNUSED(exitCode);
	Q_UNUSED(exitStatus);

    if (!installer) { return; }
    qDebug()<<"PackageRunner::installerFinished():"<<installer->program();

    if (currentTaskObject!=NULL) {
        qDebug()<<"id:"<<currentTaskObject->id;
		currentTaskObject->status = 9;
        QVariantMap sigTaskObject;
		encodeToVariantMap(currentTaskObject, sigTaskObject);
        if (!sigTaskObject.isEmpty()) { emit updateTaskStatus(sigTaskObject); }
		currentTaskObject = NULL;

		emit installTaskStart();
	}
}
