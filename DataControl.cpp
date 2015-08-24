#include "DataControl.h"
#include <QJsonDocument>
#include "Storage.h"

DataControl::DataControl(QObject *parent) : QObject(parent) {
    _UserInRunner = new UserInfoManager(this);
    _TaskRunner = new TaskManager(this);
    srvLaunchInst = new QLocalServer(this);
}

void DataControl::launchInstall() {
    QByteArray byteBuf;
    QJsonObject jsAddInstallObject;
    QJsonDocument jsDoc;
    QLocalSocket *localSock = srvLaunchInst->nextPendingConnection();

    int nCount = 5;

    do {
        if (localSock->waitForReadyRead(100)) {
            byteBuf.append(localSock->readAll());
            jsDoc = QJsonDocument::fromJson(byteBuf);
            if (jsDoc.isObject()) {
                break;
            }
            else {
                nCount--;
                continue;
            }
        }
        nCount--;
    } while (nCount > 0);

    localSock->close();
    localSock->deleteLater();
    if (jsDoc.isEmpty() || !jsDoc.isObject()) {
        return;
    }
    jsAddInstallObject = jsDoc.object();

    addInstaller(jsAddInstallObject);
    emit sigRequestShow();
}

void DataControl::InitNoticeServer() {
    if (!srvLaunchInst->isListening()) {
        srvLaunchInst->listen("launch_pipe");
    }
}

void DataControl::NoticeMain(QObject *parent, QVariantMap &jsItem) {
    QLocalSocket *sock = new QLocalSocket(parent);
    sock->connectToServer("launch_pipe");
    int nCount = 6;
    while (nCount > 0) {
        if (sock->waitForConnected(500)) {
            sock->write(QJsonDocument(QJsonObject::fromVariantMap(jsItem)).toJson(QJsonDocument::Compact));
            break;
        }
        else {
            nCount--;
        }
    }
    sock->close();
    sock->deleteLater();
}

void DataControl::LoadSettingProfile() {
    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "swgmgr.conf");
    Storage::getSettingFromFile(szFile, _setting);
}

void DataControl::SaveSettingProfile() {
    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "swgmgr.conf");
    Storage::setSettingToFile(szFile, _setting);
}

QString DataControl::getSettingParameter(QString name, QString defaultValue) {
    if (_setting.isEmpty() || !_setting.contains(name)) {
        _setting.insert(name, defaultValue);
        SaveSettingProfile();
        return defaultValue;
    }
    return _setting.value(name).toString();
}

QVariantList &DataControl::getSoftCategory() {
    return _lstSoftCategory;  //category list
}

QVariantList &DataControl::getTopPackages() {
    return  _lstSoftTopPackages;
}

QVariantList  &DataControl::getHotPackages() {
    return  _lstSoftHotPackages;
}

QMap<QString,QVariantList> &DataControl::getSoftPackages() {
    return _mapSoftPackages;  //package list by category
}

mapSoftwareList &DataControl::getInstalledSoftware() {
    return _mapInstalledSoftwares;
}

TaskManager *DataControl::getTaskManager() {
    return _TaskRunner;
}

PackageRunner &DataControl::getPackageRunner() {
    return _PendingTasks;
}

bool DataControl::initSoftCategory() {
	return Storage::LoadSoftwareCategory(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryAll.list"), _lstSoftCategory);
}

bool DataControl::initSoftPackages() {
    foreach(QVariant category, _lstSoftCategory) {
        QString szSubCategroySoftlist = ConfOperation::Root().getSubpathFile("Data", QString("SoftwareCategory") + category.toMap().value("id").toString() + ".list");
		Storage::LoadCategorySoftwareList(szSubCategroySoftlist, category.toMap().value("id").toString(), _mapSoftPackages);
	}
	return true;
}

bool DataControl::initTopPackages() {
	return Storage::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryTOP.list"), _lstSoftTopPackages);
}

bool DataControl::initHotPackages() {
	return Storage::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryHOT.list"), _lstSoftHotPackages);
}

bool DataControl::initAll() {
    LoadSettingProfile();

	initSoftCategory();
	initSoftPackages();
	initTopPackages();
	initHotPackages();

    InstalledSoftwareChanged();

    QObject::connect(srvLaunchInst, SIGNAL(newConnection()), this, SLOT(launchInstall()));
    InitNoticeServer();

    _UserInRunner->SetObjects(this,&_user);
    _UserInRunner->start();

//    _PendingTasks.initTasks();
//    _PendingTasks.moveToThread(_TaskRunner);
	InitNoticeServer();
    return true;
}

void DataControl::InstalledSoftwareChanged() {
    _mapInstalledSoftwares.clear();
    OSSystemWrapper::Instance()->GetSystemInstalledSoftware(_mapInstalledSoftwares,0);
}

void DataControl::startUserService() {
    _UserInRunner->start();
}

void DataControl::startTaskService() {
    _TaskRunner->start();
}

void DataControl::reqLoginUser(QString username,QString password) {
    emit sigLoginUser(username,password);
}

void DataControl::reqRegisteUser(QString username,QString password,QString email) {
    emit sigRegisteUser(username,password,email);
}

void DataControl::reqModifyUserInfo(QVariantMap userinfo) {
    emit sigModifyUserInfo(userinfo);
}

void DataControl::reqQueryUserState(){
    emit sigQueryUserState();
}

void DataControl::addInstaller(QJsonObject installer) {
    if (installer.isEmpty()) {
        return;
    }
    if (_PendingTasks.addTask(installer.toVariantMap())) {
        QJsonObject update;
        foreach(LPDowningTaskObject taskObject, _PendingTasks.getTasks().values()) {
            if (taskObject->id.compare(installer["id"].toString(),Qt::CaseInsensitive)==0) {
                update["id"] = taskObject->id;
                update["name"] = taskObject->name;
                update["category"] = taskObject->category;
                update["percent"] = taskObject->percent;
                update["status"] = taskObject->status;
                break;
            }
        }

        if (!update.isEmpty()) {
//            emit updateTaskInfo(update.toVariantMap());
        }
    }
}

void DataControl::checkAllTaskInfo() {
    //QJsonArray var;
    //foreach(LPDowningTaskObject taskObject, _PendingTasks.getTasks().values()) {
    //    QJsonObject it;
    //    it["id"]=taskObject->id;
    //    it["name"]=taskObject->name;
    //    it["category"]=taskObject->category;
    //    it["status"]=taskObject->status;
    //    it["percent"]=taskObject->percent;
    //    var.append(it);
    //}

    //emit updateRunningTasks(var.toVariantList());
}

void DataControl::StartInstallPackage(QString szCategoryID, QString szPackageID, bool autoInstall) {
	//QJsonObject installer;
	//QMap<QString, QJsonArray>::Iterator curItem = _softPackages.find(szCategoryID);
	//if (curItem != _softPackages.end()) {
	//	foreach(QJsonValue it, curItem.value()) {
	//		if (it.toObject().value("id").toString().compare(szPackageID, Qt::CaseInsensitive) == 0) {
	//			installer["id"] = it.toObject().value("id").toString();
	//			installer["catid"] = it.toObject().value("category").toString();
	//			installer["launchName"] = it.toObject().value("name").toString() + ".lnk";
	//			addInstaller(installer);
	//			break;
	//		}
	//	}
	//}
}
