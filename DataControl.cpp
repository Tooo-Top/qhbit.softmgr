#include "DataControl.h"
#include <QJsonDocument>
#include <QJsonObject>
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

    reqAddTask(jsAddInstallObject.toVariantMap());
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

    _UserInRunner->SetObjects(this,&_user);
    _UserInRunner->start();

    _TaskRunner->SetObjects(this,&_PendingTasks);
    _TaskRunner->start();

    InitNoticeServer();
    return true;
}

void DataControl::startUserService() { _UserInRunner->start(); }
void DataControl::startTaskService() { _TaskRunner->start(); }

void DataControl::unInit() {
    _UserInRunner->exit();
    _UserInRunner->wait();
    _TaskRunner->exit();
    _TaskRunner->wait();
}

void DataControl::InstalledSoftwareChanged() {
    _mapInstalledSoftwares.clear();
    OSSystemWrapper::Instance()->GetSystemInstalledSoftware(_mapInstalledSoftwares,0);
}

void DataControl::reqLoginUser(QString username,QString password) { emit sigLoginUser(username,password); }
void DataControl::reqRegisteUser(QString username,QString password,QString email) { emit sigRegisteUser(username,password,email); }
void DataControl::reqModifyUserInfo(QVariantMap userinfo) { emit sigModifyUserInfo(userinfo); }
void DataControl::reqQueryUserStatus(){ emit sigQueryUserState(); }

void DataControl::reqQueryAllTaskStatus() { emit sigQueryAllTaskStatus(); }

void DataControl::reqAddTask(QVariantMap task) { emit sigAddTask(task); }
void DataControl::reqAddTasks(QVariantList tasks) { emit sigAddTasks(tasks); }

void DataControl::reqPauseTask(QVariantMap task) { emit sigPauseTask(task); }
void DataControl::reqPauseAllTask() { emit sigPauseAllTask(); }

void DataControl::reqResumeTask(QVariantMap task) { emit sigResumeTask(task); }
void DataControl::reqResumeAllTask() { emit sigResumeAllTask(); }

void DataControl::reqRemoveTask(QVariantMap task) { emit sigRemoveTask(task); }
void DataControl::reqRemoveAllTask() { emit sigRemoveAllTask(); }
